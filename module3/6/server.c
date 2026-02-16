#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_PRIORITY 10
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 256

struct message {
    long mtype;
    int sender;
    char mtext[MAX_MSG_SIZE];
};

int msgid;
int active_clients[MAX_CLIENTS] = {0};

void cleanup(int sig) {
    (void)sig;
    
    printf("\nСервер завершает работу...\n");
    
    struct message msg;
    msg.sender = SERVER_PRIORITY;
    strcpy(msg.mtext, "SERVER_SHUTDOWN");
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (active_clients[i]) {
            msg.mtype = (i + 1) * 10 + 10;
            msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
        }
    }
    
    sleep(1);
    
    msgctl(msgid, IPC_RMID, NULL);
    exit(0);
}

int main() {
    key_t key;
    struct message msg;
    
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    
    if ((key = ftok(".", 'A')) == -1) {
        perror("ftok");
        exit(1);
    }
    
    if ((msgid = msgget(key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    printf("Сервер запущен. ID очереди: %d\n", msgid);
    
    while (1) {
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), SERVER_PRIORITY, 0) == -1) {
            perror("msgrcv");
            continue;
        }
        
        printf("Получено сообщение от клиента %d: %s\n", (msg.sender - 10) / 10, msg.mtext);
        
        if (strcmp(msg.mtext, "shutdown") == 0) {
            printf("Клиент %d отключился\n", (msg.sender - 10) / 10);
            int client_index = (msg.sender - 10) / 10 - 1;
            if (client_index >= 0 && client_index < MAX_CLIENTS) {
                active_clients[client_index] = 0;
            }
            continue;
        }
        
        if (strcmp(msg.mtext, "SERVER_SHUTDOWN") == 0) {
            continue;
        }
        
        int client_index = (msg.sender - 10) / 10 - 1;
        if (client_index >= 0 && client_index < MAX_CLIENTS) {
            active_clients[client_index] = 1;
        }
        
        struct message forward_msg;
        forward_msg.sender = msg.sender;
        strcpy(forward_msg.mtext, msg.mtext);
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (active_clients[i]) {
                int client_priority = (i + 1) * 10 + 10;
                if (client_priority != msg.sender) {
                    forward_msg.mtype = client_priority;
                    if (msgsnd(msgid, &forward_msg, sizeof(forward_msg) - sizeof(long), 0) == -1) {
                        perror("msgsnd");
                    }
                }
            }
        }
    }
    
    return 0;
}