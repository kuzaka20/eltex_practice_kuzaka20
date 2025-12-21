#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>

#define PORT 8888
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define MAX_NAME_LEN 31
#define MAX_MESSAGE_LEN 500

typedef struct {
    struct sockaddr_in addr;
    int active;
    char name[MAX_NAME_LEN + 1];
} client_t;

client_t clients[MAX_CLIENTS];
int server_socket;

void cleanup(int sig) {
    (void)sig;
    printf("\nСервер завершает работу...\n");
    close(server_socket);
    exit(0);
}

int find_client_index(struct sockaddr_in *addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && 
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

int add_client(struct sockaddr_in *addr, const char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].addr = *addr;
            clients[i].active = 1;
            strncpy(clients[i].name, name, MAX_NAME_LEN);
            clients[i].name[MAX_NAME_LEN] = '\0';
            return i;
        }
    }
    return -1;
}

void broadcast_message(const char *message, int exclude_index) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != exclude_index) {
            sendto(server_socket, message, strlen(message), 0,
                   (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
        }
    }
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    fd_set read_fds;
    
    signal(SIGINT, cleanup);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
    }
    
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    printf("UDP чат-сервер запущен на порту %d\n", PORT);
    printf("Ожидание подключения 2 клиентов...\n");
    
    int connected_clients = 0;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        
        struct timeval timeout = {1, 0};
        
        int activity = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            perror("select error");
            continue;
        }
        
        if (activity == 0) {
            continue;
        }
        
        if (FD_ISSET(server_socket, &read_fds)) {
            int n = recvfrom(server_socket, buffer, MAX_MESSAGE_LEN, 0,
                            (struct sockaddr*)&client_addr, &client_len);
            
            if (n > 0) {
                buffer[n] = '\0';
                
                int client_index = find_client_index(&client_addr);
                
                if (client_index == -1) {
                    if (connected_clients < MAX_CLIENTS) {
                        char client_name[MAX_NAME_LEN + 1];
                        snprintf(client_name, sizeof(client_name), "Клиент%d", connected_clients + 1);
                        
                        client_index = add_client(&client_addr, client_name);
                        if (client_index != -1) {
                            connected_clients++;
                            printf("%s подключился (%s:%d)\n", 
                                   clients[client_index].name,
                                   inet_ntoa(client_addr.sin_addr),
                                   ntohs(client_addr.sin_port));
                            
                            char welcome_msg[BUFFER_SIZE];
                            snprintf(welcome_msg, sizeof(welcome_msg), 
                                    "СЕРВЕР: %s присоединился к чату", clients[client_index].name);
                            broadcast_message(welcome_msg, client_index);
                            
                            char greeting[BUFFER_SIZE];
                            snprintf(greeting, sizeof(greeting),
                                    "СЕРВЕР: Добро пожаловать в чат! Вы %s. Всего участников: %d",
                                    clients[client_index].name, connected_clients);
                            sendto(server_socket, greeting, strlen(greeting), 0,
                                  (struct sockaddr*)&client_addr, client_len);
                        }
                    } else {
                        char reject_msg[] = "СЕРВЕР: Чат заполнен. Максимум 2 участника.";
                        sendto(server_socket, reject_msg, strlen(reject_msg), 0,
                              (struct sockaddr*)&client_addr, client_len);
                        continue;
                    }
                }
                
                if (client_index != -1) {
                    printf("%s: %s\n", clients[client_index].name, buffer);
                    
                    if (strcmp(buffer, "/quit") == 0) {
                        char leave_msg[BUFFER_SIZE];
                        snprintf(leave_msg, sizeof(leave_msg),
                                "СЕРВЕР: %s покинул чат", clients[client_index].name);
                        
                        printf("%s отключился\n", clients[client_index].name);
                        clients[client_index].active = 0;
                        connected_clients--;
                        
                        broadcast_message(leave_msg, -1);
                        continue;
                    }
                    
                    char formatted_msg[BUFFER_SIZE];
                    int max_msg_len = BUFFER_SIZE - strlen(clients[client_index].name) - 3;
                    if (max_msg_len > 0) {
                        snprintf(formatted_msg, sizeof(formatted_msg),
                                "%.*s: %.*s", 
                                MAX_NAME_LEN, clients[client_index].name,
                                max_msg_len, buffer);
                    } else {
                        snprintf(formatted_msg, sizeof(formatted_msg),
                                "%.*s: сообщение", MAX_NAME_LEN, clients[client_index].name);
                    }
                    
                    broadcast_message(formatted_msg, client_index);
                }
            }
        }
    }
    
    close(server_socket);
    return 0;
}