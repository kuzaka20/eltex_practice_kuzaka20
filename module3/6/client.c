#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_PRIORITY 10
#define MAX_MSG_SIZE 256

struct message {
    long mtype;
    int sender;
    char mtext[MAX_MSG_SIZE];
};

int msgid;
int client_id;
int my_priority;
volatile sig_atomic_t shutdown_flag = 0;

void cleanup(int sig) {
    (void)sig;
    
    if (!shutdown_flag) {
        shutdown_flag = 1;
        printf("\nКлиент завершает работу...\n");
        
        if (getpid() != 0) {
            struct message msg;
            msg.mtype = SERVER_PRIORITY;
            msg.sender = my_priority;
            strcpy(msg.mtext, "shutdown");
            msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
        }
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    key_t key;
    struct message msg;
    
    if (argc != 2) {
        printf("Использование: %s <номер_клиента>\n", argv[0]);
        printf("Номер клиента должен быть от 1 до 9\n");
        exit(1);
    }
    
    client_id = atoi(argv[1]);
    if (client_id < 1 || client_id > 9) {
        printf("Номер клиента должен быть от 1 до 9\n");
        exit(1);
    }
    
    signal(SIGINT, cleanup);
    
    if ((key = ftok(".", 'A')) == -1) {
        perror("ftok");
        exit(1);
    }
    
    if ((msgid = msgget(key, 0666)) == -1) {
        perror("msgget");
        printf("Убедитесь, что сервер запущен\n");
        exit(1);
    }
    
    my_priority = client_id * 10 + 10;
    
    printf("Клиент %d подключен к чату (приоритет: %d)\n", client_id, my_priority);
    printf("Введите сообщение: ");
    fflush(stdout);
    
    pid_t pid = fork();
    
    if (pid == 0) {
        signal(SIGINT, SIG_IGN);
        
        while (1) {
            if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), my_priority, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }
            
            if (strcmp(msg.mtext, "SERVER_SHUTDOWN") == 0) {
                printf("\nСервер завершил работу. Клиент отключается.\n");
                exit(0);
            }
            
            if (msg.sender != my_priority) {
                printf("\n[Клиент %d]: %s\n", (msg.sender - 10) / 10, msg.mtext);
                printf("Введите сообщение: ");
                fflush(stdout);
            }
        }
    } else {
        while (1) {
            char input[MAX_MSG_SIZE];
            if (fgets(input, MAX_MSG_SIZE, stdin) != NULL) {
                input[strcspn(input, "\n")] = 0;
                
                if (shutdown_flag) {
                    exit(0);
                }
                
                int status;
                if (waitpid(pid, &status, WNOHANG) == pid) {
                    printf("Сервер отключился. Клиент завершает работу.\n");
                    exit(0);
                }
                
                msg.mtype = SERVER_PRIORITY;
                msg.sender = my_priority;
                strncpy(msg.mtext, input, MAX_MSG_SIZE - 1);
                msg.mtext[MAX_MSG_SIZE - 1] = '\0';
                
                if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                    perror("msgsnd");
                }
                
                printf("[Вы]: %s\n", input);
                printf("Введите сообщение: ");
                fflush(stdout);
            }
        }
    }
    
    return 0;
}