#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>

#define SHM_NAME "/shared_memory"
#define SEM_PRODUCER "/producer_sem"
#define SEM_CONSUMER "/consumer_sem"
#define MAX_NUMBERS 20
#define MAX_NUMBER_VALUE 1000

struct shared_data {
    int numbers[MAX_NUMBERS];
    int count;
    int min;
    int max;
    int processed_count;
};

volatile sig_atomic_t shutdown_flag = 0;
int shm_fd;
struct shared_data *shared_mem;
sem_t *sem_producer, *sem_consumer;

void sigint_handler(int sig) {
    (void)sig;
    shutdown_flag = 1;
    printf("\nПолучен сигнал SIGINT. Завершение работы...\n");
}

void cleanup() {
    if (shared_mem != NULL) {
        munmap(shared_mem, sizeof(struct shared_data));
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
    if (sem_producer != SEM_FAILED) {
        sem_close(sem_producer);
        sem_unlink(SEM_PRODUCER);
    }
    if (sem_consumer != SEM_FAILED) {
        sem_close(sem_consumer);
        sem_unlink(SEM_CONSUMER);
    }
}

int main() {
    pid_t pid;
    
    signal(SIGINT, sigint_handler);
    
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_PRODUCER);
    sem_unlink(SEM_CONSUMER);
    
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) {
        perror("ftruncate");
        cleanup();
        exit(1);
    }
    
    shared_mem = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        cleanup();
        exit(1);
    }
    
    sem_producer = sem_open(SEM_PRODUCER, O_CREAT | O_EXCL, 0666, 1);
    if (sem_producer == SEM_FAILED) {
        perror("sem_open producer");
        cleanup();
        exit(1);
    }
    
    sem_consumer = sem_open(SEM_CONSUMER, O_CREAT | O_EXCL, 0666, 0);
    if (sem_consumer == SEM_FAILED) {
        perror("sem_open consumer");
        cleanup();
        exit(1);
    }
    
    shared_mem->count = 0;
    shared_mem->min = 0;
    shared_mem->max = 0;
    shared_mem->processed_count = 0;
    
    srand(time(NULL));
    
    printf("Программа запущена. Нажмите Ctrl+C для завершения.\n");
    printf("Разделяемая память POSIX создана: %s\n", SHM_NAME);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        cleanup();
        exit(1);
    }
    else if (pid == 0) {
        printf("Дочерний процесс (обработчик) PID=%d начал работу\n", getpid());
        
        while (!shutdown_flag) {
            if (sem_wait(sem_consumer) == -1) {
                if (shutdown_flag) break;
                perror("sem_wait consumer");
                break;
            }
            
            if (shutdown_flag) break;
            
            if (shared_mem->count > 0) {
                shared_mem->min = shared_mem->numbers[0];
                shared_mem->max = shared_mem->numbers[0];
                
                for (int i = 1; i < shared_mem->count; i++) {
                    if (shared_mem->numbers[i] < shared_mem->min) {
                        shared_mem->min = shared_mem->numbers[i];
                    }
                    if (shared_mem->numbers[i] > shared_mem->max) {
                        shared_mem->max = shared_mem->numbers[i];
                    }
                }
                
                shared_mem->processed_count++;
                
                printf("Обработчик: найден min=%d, max=%d\n", shared_mem->min, shared_mem->max);
            }
            
            if (sem_post(sem_producer) == -1) {
                perror("sem_post producer");
                break;
            }
        }
        
        printf("Дочерний процесс завершил работу\n");
        munmap(shared_mem, sizeof(struct shared_data));
        close(shm_fd);
        sem_close(sem_producer);
        sem_close(sem_consumer);
        exit(0);
    }
    else {
        printf("Родительский процесс (генератор) PID=%d начал работу\n", getpid());
        
        while (!shutdown_flag) {
            if (sem_wait(sem_producer) == -1) {
                if (shutdown_flag) break;
                perror("sem_wait producer");
                break;
            }
            
            if (shutdown_flag) break;
            
            shared_mem->count = rand() % (MAX_NUMBERS - 4) + 5;
            
            printf("Генерация набора %d: [", shared_mem->processed_count + 1);
            for (int i = 0; i < shared_mem->count; i++) {
                shared_mem->numbers[i] = rand() % MAX_NUMBER_VALUE + 1;
                printf("%d", shared_mem->numbers[i]);
                if (i < shared_mem->count - 1) {
                    printf(" ");
                }
            }
            printf("]\n");
            
            if (sem_post(sem_consumer) == -1) {
                perror("sem_post consumer");
                break;
            }
            
            if (sem_wait(sem_producer) == -1) {
                if (shutdown_flag) break;
                perror("sem_wait producer");
                break;
            }
            
            if (shutdown_flag) break;
            
            printf("Результаты: min=%d, max=%d\n\n", shared_mem->min, shared_mem->max);
            
            if (sem_post(sem_consumer) == -1) {
                perror("sem_post consumer");
                break;
            }
            
            sleep(1);
        }
        
        sem_post(sem_consumer);
        
        wait(NULL);
        
        printf("\nОбработано наборов данных: %d\n", shared_mem->processed_count);
        printf("Родительский процесс завершил работу\n");
    }
    
    cleanup();
    return 0;
}