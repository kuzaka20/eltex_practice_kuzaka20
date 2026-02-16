#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#define MAX_LINE_LENGTH 256
#define MAX_NUMBERS 10


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void sem_wait(int semid, int sem_num) {
    struct sembuf sb = {sem_num, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid, int sem_num) {
    struct sembuf sb = {sem_num, 1, 0};
    semop(semid, &sb, 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    key_t key;
    int semid;
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int numbers[MAX_NUMBERS];
    int count, min, max;
    char *token;

    if ((key = ftok(filename, 'P')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((semid = semget(key, 2, 0666)) == -1) {
        perror("semget");
        printf("Семафор для файла %s не найден. Запустите сначала производителя.\n", filename);
        exit(1);
    }

    printf("Потребитель PID=%d начал работу с файлом: %s\n", getpid(), filename);

    while (1) {
        sem_wait(semid, 1);

        file = fopen(filename, "r");
        if (file == NULL) {
            perror("fopen");
            sem_signal(semid, 1);
            exit(1);
        }

        char last_line[MAX_LINE_LENGTH] = "";
        while (fgets(line, sizeof(line), file) != NULL) {
            if (strlen(line) > 1) { 
                strcpy(last_line, line);
            }
        }
        fclose(file);

        if (strlen(last_line) > 0) {

            count = 0;
            token = strtok(last_line, " \n");
            while (token != NULL && count < MAX_NUMBERS) {
                numbers[count] = atoi(token);
                token = strtok(NULL, " \n");
                count++;
            }

            if (count > 0) {
                min = max = numbers[0];
                for (int i = 1; i < count; i++) {
                    if (numbers[i] < min) min = numbers[i];
                    if (numbers[i] > max) max = numbers[i];
                }

                printf("Потребитель PID=%d: строка [", getpid());
                for (int i = 0; i < count; i++) {
                    printf("%d", numbers[i]);
                    if (i < count - 1) printf(" ");
                }
                printf("] -> min=%d, max=%d\n", min, max);
            }

            file = fopen(filename, "w");
            if (file != NULL) {
                fclose(file);
            }
        }

        sem_signal(semid, 0);

        sleep(1); 
    }

    return 0;
}