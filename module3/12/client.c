#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

int client_socket;
struct sockaddr_in server_addr;
int running = 1;
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;

void cleanup(int sig) {
    (void)sig;
    running = 0;
    printf("\nЗавершение работы...\n");
    
    char quit_msg[] = "/quit";
    sendto(client_socket, quit_msg, strlen(quit_msg), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    close(client_socket);
    exit(0);
}

void display_message(const char *message) {
    pthread_mutex_lock(&display_mutex);
    printf("\r\033[K");
    printf("%s\n", message);
    printf("Введите сообщение: ");
    fflush(stdout);
    pthread_mutex_unlock(&display_mutex);
}

void* receive_messages(void* arg) {
    (void)arg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    while (running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        
        struct timeval timeout = {1, 0};
        
        int activity = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(client_socket, &read_fds)) {
            int n = recvfrom(client_socket, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr*)&from_addr, &from_len);
            
            if (n > 0) {
                buffer[n] = '\0';
                display_message(buffer);
            }
        }
    }
    return NULL;
}

int main() {
    char buffer[BUFFER_SIZE];
    pthread_t recv_thread;
    
    signal(SIGINT, cleanup);
    
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        printf("Неверный IP адрес: %s\n", SERVER_IP);
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    printf("UDP клиент запущен\n");
    printf("Подключение к серверу %s:%d\n", SERVER_IP, PORT);
    printf("Введите сообщение (для выхода введите /quit):\n");
    printf("Введите сообщение: ");
    fflush(stdout);
    
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("pthread_create failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    while (running) {
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = '\0';
            
            if (strcmp(buffer, "/quit") == 0) {
                running = 0;
                break;
            }
            
            if (strlen(buffer) > 0) {
                pthread_mutex_lock(&display_mutex);
                printf("\r\033[K");
                printf("[Вы]: %s\n", buffer);
                printf("Введите сообщение: ");
                fflush(stdout);
                pthread_mutex_unlock(&display_mutex);
                
                if (sendto(client_socket, buffer, strlen(buffer), 0,
                          (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                    perror("sendto failed");
                    printf("Проверьте, запущен ли сервер\n");
                }
            } else {
                pthread_mutex_lock(&display_mutex);
                printf("Введите сообщение: ");
                fflush(stdout);
                pthread_mutex_unlock(&display_mutex);
            }
        }
    }
    
    pthread_join(recv_thread, NULL);
    
    close(client_socket);
    printf("\nКлиент завершил работу\n");
    
    return 0;
}