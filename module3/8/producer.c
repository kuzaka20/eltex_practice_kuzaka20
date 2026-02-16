#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>

#define MAX_NUMBERS 10
#define MAX_NUMBER_VALUE 100

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
    union semun arg;
    FILE *file;
    int i, count;

    if ((key = ftok(filename, 'P')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((semid = semget(key, 2, IPC_CREAT | 0666)) == -1) {
        perror("semget");
        exit(1);
    }

    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL producer");
        exit(1);
    }

    arg.val = 0;
    if (semctl(semid, 1, SETVAL, arg) == -1) {
        perror("semctl SETVAL consumer");
        exit(1);
    }

    srand(time(NULL) ^ getpid());

    printf("Производитель PID=%d начал работу с файлом: %s\n", getpid(), filename);

    for (i = 0; i < 5; i++) {
        sem_wait(semid, 0);

        file = fopen(filename, "a");
        if (file == NULL) {
            perror("fopen");
            sem_signal(semid, 0);
            exit(1);
        }

        count = rand() % (MAX_NUMBERS - 2) + 3;
        
        for (int j = 0; j < count; j++) {
            int num = rand() % MAX_NUMBER_VALUE + 1;
            fprintf(file, "%d", num);
            if (j < count - 1) {
                fprintf(file, " ");
            }
        }
        fprintf(file, "\n");
        
        fclose(file);
        
        printf("Производитель PID=%d записал строку %d с %d числами\n", 
               getpid(), i + 1, count);

        sem_signal(semid, 1);

        sleep(rand() % 3 + 1);
    }

    printf("Производитель PID=%d завершил работу\n", getpid());
    return 0;
}