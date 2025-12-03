#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_QUEUE_NAME   "/chat_server"
#define CLIENT_QUEUE_NAME   "/chat_client"
#define MAX_MSG_SIZE        256
#define MSG_BUFFER_SIZE     (MAX_MSG_SIZE + 10)

#define SHUTDOWN_PRIORITY   255

mqd_t server_qid, client_qid;
volatile sig_atomic_t shutdown_flag = 0;

void cleanup(int sig) {
    (void)sig;
    
    if (!shutdown_flag) {
        shutdown_flag = 1;
        printf("\nСервер завершает работу...\n");
        
        char shutdown_msg[MAX_MSG_SIZE] = "SHUTDOWN";
        if (mq_send(client_qid, shutdown_msg, strlen(shutdown_msg) + 1, SHUTDOWN_PRIORITY) == -1) {
            perror("mq_send shutdown");
        }
        
        if (server_qid != (mqd_t)-1) {
            mq_close(server_qid);
            mq_unlink(SERVER_QUEUE_NAME);
        }
        if (client_qid != (mqd_t)-1) {
            mq_close(client_qid);
        }
    }
    
    exit(0);
}

int main() {
    struct mq_attr attr;
    char buffer[MSG_BUFFER_SIZE];
    unsigned int priority;
    ssize_t bytes_read;

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    
    mq_unlink(SERVER_QUEUE_NAME);
    server_qid = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (server_qid == (mqd_t)-1) {
        perror("mq_open server");
        exit(1);
    }
    
    printf("Сервер запущен. Ожидание подключения клиента...\n");
    printf("Очередь сервера: %s\n", SERVER_QUEUE_NAME);

    printf("Ожидание создания очереди клиента...\n");
    int attempts = 0;
    while (attempts < 30) {
        client_qid = mq_open(CLIENT_QUEUE_NAME, O_WRONLY);
        if (client_qid != (mqd_t)-1) {
            break;
        }
        sleep(1);
        attempts++;
    }
    
    if (client_qid == (mqd_t)-1) {
        printf("Не удалось подключиться к клиенту\n");
        cleanup(0);
    }
    
    printf("Клиент подключен!\n");
    printf("Введите сообщение: ");
    fflush(stdout);
    
    while (!shutdown_flag) {
        printf("Сервер: ");
        fflush(stdout);
        
        if (fgets(buffer, MAX_MSG_SIZE, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;

            if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) {
                printf("Завершение чата...\n");
                break;
            }
            
            if (mq_send(client_qid, buffer, strlen(buffer) + 1, 0) == -1) {
                perror("mq_send");
                break;
            }
            
            if (strcmp(buffer, "SHUTDOWN") == 0) {
                printf("Завершение чата...\n");
                break;
            }
        } else {
            break;
        }
        
        bytes_read = mq_receive(server_qid, buffer, MSG_BUFFER_SIZE, &priority);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            
            if (priority == SHUTDOWN_PRIORITY || strcmp(buffer, "SHUTDOWN") == 0) {
                printf("Клиент завершил чат.\n");
                break;
            }
            
            printf("Клиент: %s\n", buffer);
            
            if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) {
                printf("Завершение чата...\n");
                break;
            }
        } else {
            perror("mq_receive");
            break;
        }
        
        printf("Введите сообщение: ");
        fflush(stdout);
    }
    
    cleanup(0);
    return 0;
}