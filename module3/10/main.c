#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define SHM_SIZE 1024
#define MAX_NUMBERS 20
#define MAX_NUMBER_VALUE 1000

struct shared_data {
    int numbers[MAX_NUMBERS];
    int count;
    int min;
    int max;
    int processed;
    int ready;
};

volatile sig_atomic_t shutdown_flag = 0;
int shmid;
struct shared_data *shared_mem;

void sigint_handler(int sig) {
    (void)sig;
    shutdown_flag = 1;
    printf("\nПолучен сигнал SIGINT. Завершение работы...\n");
}

void cleanup() {
    if (shared_mem != NULL) {
        shmdt(shared_mem);
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
}

int main() {
    key_t key;
    pid_t pid;
    int sets_processed = 0;
    
    signal(SIGINT, sigint_handler);
    
    if ((key = ftok(".", 'S')) == -1) {
        perror("ftok");
        exit(1);
    }
    
    if ((shmid = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0666)) == -1) {
        perror("shmget");
        exit(1);
    }
    
    if ((shared_mem = (struct shared_data*)shmat(shmid, NULL, 0)) == (void*)-1) {
        perror("shmat");
        cleanup();
        exit(1);
    }
    
    shared_mem->count = 0;
    shared_mem->min = 0;
    shared_mem->max = 0;
    shared_mem->processed = 0;
    shared_mem->ready = 1;
    
    srand(time(NULL));
    
    printf("Программа запущена. Нажмите Ctrl+C для завершения.\n");
    printf("Разделяемая память создана с ID: %d\n", shmid);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        cleanup();
        exit(1);
    }
    else if (pid == 0) {
        printf("Дочерний процесс (обработчик) PID=%d начал работу\n", getpid());
        
        while (!shutdown_flag) {
            if (shared_mem->ready == 0) {
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
                    
                    shared_mem->processed = 1;
                    shared_mem->ready = 1;
                }
            }
            usleep(10000);
        }
        
        printf("Дочерний процесс завершил работу\n");
        shmdt(shared_mem);
        exit(0);
    }
    else {
        printf("Родительский процесс (генератор) PID=%d начал работу\n", getpid());
        
        while (!shutdown_flag) {
            if (shared_mem->ready == 1) {
                shared_mem->count = rand() % (MAX_NUMBERS - 4) + 5;
                
                printf("Генерация набора %d: [", sets_processed + 1);
                for (int i = 0; i < shared_mem->count; i++) {
                    shared_mem->numbers[i] = rand() % MAX_NUMBER_VALUE + 1;
                    printf("%d", shared_mem->numbers[i]);
                    if (i < shared_mem->count - 1) {
                        printf(" ");
                    }
                }
                printf("]\n");
                
                shared_mem->processed = 0;
                shared_mem->ready = 0;
                
                while (shared_mem->ready == 0 && !shutdown_flag) {
                    usleep(10000);
                }
                
                if (shared_mem->processed == 1) {
                    printf("Результаты: min=%d, max=%d\n\n", shared_mem->min, shared_mem->max);
                    sets_processed++;
                }
                
                sleep(1);
            }
        }
        
        wait(NULL);
        
        printf("\nОбработано наборов данных: %d\n", sets_processed);
        printf("Родительский процесс завершил работу\n");
    }
    
    cleanup();
    return 0;
}