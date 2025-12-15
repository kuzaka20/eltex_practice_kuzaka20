#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <time.h>

#define FILENAME "data.txt"
#define SEM_PRODUCER "/producer_sem"
#define SEM_CONSUMER "/consumer_sem"
#define MAX_NUMBERS 10
#define MAX_NUMBER_VALUE 100
#define MAX_LINE_LENGTH 256

void producer_process(sem_t *sem_producer, sem_t *sem_consumer) {
    FILE *file;
    int i, count;
    
    srand(time(NULL) ^ getpid());
    printf("Родительский процесс (производитель) PID=%d начал работу\n", getpid());

    for (i = 0; i < 5; i++) {
        sem_wait(sem_producer);

        file = fopen(FILENAME, "w");
        if (file == NULL) {
            perror("fopen");
            exit(1);
        }

        count = rand() % (MAX_NUMBERS - 2) + 3;
        
        printf("Производитель: генерирую строку %d: [", i + 1);
        for (int j = 0; j < count; j++) {
            int num = rand() % MAX_NUMBER_VALUE + 1;
            fprintf(file, "%d", num);
            printf("%d", num);
            if (j < count - 1) {
                fprintf(file, " ");
                printf(" ");
            }
        }
        fprintf(file, "\n");
        printf("]\n");
        
        fclose(file);
        
        printf("Производитель записал строку %d с %d числами\n", i + 1, count);

        sem_post(sem_consumer);

        sleep(rand() % 2 + 1);
    }

    sem_wait(sem_producer);
    file = fopen(FILENAME, "w");
    if (file != NULL) {
        fprintf(file, "END\n");
        fclose(file);
    }
    sem_post(sem_consumer);

    printf("Родительский процесс завершил работу\n");
}

void consumer_process(sem_t *sem_producer, sem_t *sem_consumer) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int numbers[MAX_NUMBERS];
    int count, min, max;
    char *token;
    
    printf("Дочерний процесс (потребитель) PID=%d начал работу\n", getpid());

    while (1) {
        sem_wait(sem_consumer);

        file = fopen(FILENAME, "r");
        if (file == NULL) {
            perror("fopen");
            exit(1);
        }

        if (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = 0;
            
            if (strcmp(line, "END") == 0) {
                fclose(file);
                printf("Потребитель получил сигнал о завершении\n");
                break;
            }

            count = 0;
            token = strtok(line, " ");
            while (token != NULL && count < MAX_NUMBERS) {
                numbers[count] = atoi(token);
                token = strtok(NULL, " ");
                count++;
            }

            if (count > 0) {
                min = max = numbers[0];
                for (int i = 1; i < count; i++) {
                    if (numbers[i] < min) min = numbers[i];
                    if (numbers[i] > max) max = numbers[i];
                }

                printf("Потребитель: строка [");
                for (int i = 0; i < count; i++) {
                    printf("%d", numbers[i]);
                    if (i < count - 1) printf(" ");
                }
                printf("] -> min=%d, max=%d\n", min, max);
            }
        }
        
        fclose(file);

        sem_post(sem_producer);
    }

    printf("Дочерний процесс завершил работу\n");
}

int main() {
    sem_t *sem_producer, *sem_consumer;
    pid_t pid;

    sem_unlink(SEM_PRODUCER);
    sem_unlink(SEM_CONSUMER);

    sem_producer = sem_open(SEM_PRODUCER, O_CREAT | O_EXCL, 0666, 1);
    if (sem_producer == SEM_FAILED) {
        perror("sem_open producer");
        exit(1);
    }

    sem_consumer = sem_open(SEM_CONSUMER, O_CREAT | O_EXCL, 0666, 0);
    if (sem_consumer == SEM_FAILED) {
        perror("sem_open consumer");
        sem_close(sem_producer);
        sem_unlink(SEM_PRODUCER);
        exit(1);
    }

    remove(FILENAME);

    printf("Запуск модели производитель-потребитель с семафорами POSIX\n");

    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        consumer_process(sem_producer, sem_consumer);
        
        sem_close(sem_producer);
        sem_close(sem_consumer);
        exit(0);
    }
    else {
        producer_process(sem_producer, sem_consumer);
        
        wait(NULL);
        
        sem_close(sem_producer);
        sem_close(sem_consumer);
        sem_unlink(SEM_PRODUCER);
        sem_unlink(SEM_CONSUMER);
        
        remove(FILENAME);
        
        printf("Оба процесса завершили работу\n");
    }

    return 0;
}