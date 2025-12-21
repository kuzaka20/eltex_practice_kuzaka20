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
        printf("\nКлиент завершает работу...\n");
        
        char shutdown_msg[MAX_MSG_SIZE] = "SHUTDOWN";
        if (mq_send(server_qid, shutdown_msg, strlen(shutdown_msg) + 1, SHUTDOWN_PRIORITY) == -1) {
            perror("mq_send shutdown");
        }
        
        if (client_qid != (mqd_t)-1) {
            mq_close(client_qid);
            mq_unlink(CLIENT_QUEUE_NAME);
        }
        if (server_qid != (mqd_t)-1) {
            mq_close(server_qid);
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
    
    mq_unlink(CLIENT_QUEUE_NAME);
    client_qid = mq_open(CLIENT_QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (client_qid == (mqd_t)-1) {
        perror("mq_open client");
        exit(1);
    }
    
    printf("Клиент запущен. Подключение к серверу...\n");
    printf("Очередь клиента: %s\n", CLIENT_QUEUE_NAME);
    
    printf("Подключение к серверу...\n");
    int attempts = 0;
    while (attempts < 30) {
        server_qid = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
        if (server_qid != (mqd_t)-1) {
            break;
        }
        sleep(1);
        attempts++;
    }
    
    if (server_qid == (mqd_t)-1) {
        printf("Не удалось подключиться к серверу\n");
        cleanup(0);
    }
    
    printf("Подключение к серверу установлено!\n");
    printf("Ожидание сообщения от сервера...\n");
    
    while (!shutdown_flag) {

        bytes_read = mq_receive(client_qid, buffer, MSG_BUFFER_SIZE, &priority);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            
            if (priority == SHUTDOWN_PRIORITY || strcmp(buffer, "SHUTDOWN") == 0) {
                printf("Сервер завершил чат.\n");
                break;
            }
            
            printf("Сервер: %s\n", buffer);
            
            if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) {
                printf("Завершение чата...\n");
                break;
            }
        } else {
            perror("mq_receive");
            break;
        }
        
        printf("Клиент: ");
        fflush(stdout);
        
        if (fgets(buffer, MAX_MSG_SIZE, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;
            
            if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) {
                printf("Завершение чата...\n");
                break;
            }
            
            if (mq_send(server_qid, buffer, strlen(buffer) + 1, 0) == -1) {
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
        
        printf("Ожидание ответа от сервера...\n");
    }
    
    cleanup(0);
    return 0;
}