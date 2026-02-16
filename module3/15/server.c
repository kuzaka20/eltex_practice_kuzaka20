#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_FILENAME 256

typedef struct {
    int socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    int buffer_len;
    int receiving_file;
    long file_size;
    long file_received;
    FILE* file_ptr;
    char filename[MAX_FILENAME];
    int waiting_file_info;
} ClientInfo;

typedef struct {
    char filename[MAX_FILENAME];
    long filesize;
} FileInfo;

ClientInfo clients[MAX_CLIENTS];
int max_clients = MAX_CLIENTS;
int server_fd;

void handle_calc(char* operation, double num1, double num2, char* result) {
    double res = 0;
    int error = 0;
    
    if (strcmp(operation, "ADD") == 0) {
        res = num1 + num2;
    } 
    else if (strcmp(operation, "SUB") == 0) {
        res = num1 - num2;
    } 
    else if (strcmp(operation, "MUL") == 0) {
        res = num1 * num2;
    } 
    else if (strcmp(operation, "DIV") == 0) {
        if (num2 != 0) {
            res = num1 / num2;
        } else {
            error = 1;
            strcpy(result, "Ошибка: деление на ноль");
        }
    } 
    else if (strcmp(operation, "POW") == 0) {
        res = pow(num1, num2);
    } 
    else {
        error = 1;
        strcpy(result, "Ошибка: неизвестная операция");
    }
    
    if (!error) {
        sprintf(result, "Результат: %.2f", res);
    }
}

int add_client(int socket, struct sockaddr_in* addr) {
    for (int i = 0; i < max_clients; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = socket;
            clients[i].address = *addr;
            clients[i].buffer_len = 0;
            clients[i].receiving_file = 0;
            clients[i].waiting_file_info = 0;
            clients[i].file_size = 0;
            clients[i].file_received = 0;
            clients[i].file_ptr = NULL;
            memset(clients[i].buffer, 0, BUFFER_SIZE);
            memset(clients[i].filename, 0, MAX_FILENAME);
            
            printf("Новый клиент подключен: %s:%d (сокет %d)\n",
                   inet_ntoa(clients[i].address.sin_addr),
                   ntohs(clients[i].address.sin_port),
                   clients[i].socket);
            return i;
        }
    }
    return -1;
}

void remove_client(int client_index) {
    if (client_index < 0 || client_index >= max_clients) return;
    
    if (clients[client_index].file_ptr) {
        fclose(clients[client_index].file_ptr);
        clients[client_index].file_ptr = NULL;
        
        if (clients[client_index].receiving_file && 
            clients[client_index].file_received < clients[client_index].file_size) {
            remove(clients[client_index].filename);
            printf("Удален неполный файл: %s\n", clients[client_index].filename);
        }
    }
    
    printf("Клиент отключен: %s:%d (сокет %d)\n",
           inet_ntoa(clients[client_index].address.sin_addr),
           ntohs(clients[client_index].address.sin_port),
           clients[client_index].socket);
    
    close(clients[client_index].socket);
    
    memset(&clients[client_index], 0, sizeof(ClientInfo));
}

int send_file(int client_socket, const char* filename) {
    FILE* file;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    FileInfo file_info;
    
    file = fopen(filename, "rb");
    if (!file) {
        perror("Ошибка открытия файла");
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    file_info.filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    strncpy(file_info.filename, filename, MAX_FILENAME - 1);
    file_info.filename[MAX_FILENAME - 1] = '\0';
    
    printf("Отправка файла: %s (%.2f KB)\n", 
           filename, file_info.filesize / 1024.0);
    
    if (send(client_socket, &file_info, sizeof(FileInfo), 0) <= 0) {
        printf("Ошибка отправки информации о файле\n");
        fclose(file);
        return -1;
    }
    
    usleep(10000);
    
    long total_sent = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t sent = send(client_socket, buffer, bytes_read, 0);
        if (sent <= 0) {
            printf("Ошибка отправки данных файла\n");
            break;
        }
        total_sent += sent;
        
        if (file_info.filesize > 0) {
            printf("\rПрогресс: %.2f%%", 
                   (double)total_sent / file_info.filesize * 100);
            fflush(stdout);
        }
    }
    
    printf("\n");
    fclose(file);
    
    if (total_sent == file_info.filesize) {
        return 0;
    } else {
        printf("Ошибка: отправлено %ld из %ld байт\n", total_sent, file_info.filesize);
        return -1;
    }
}

void process_command(int client_index) {
    ClientInfo* client = &clients[client_index];
    char response[BUFFER_SIZE];
    
    if (client->waiting_file_info) {
        FileInfo file_info;
        ssize_t bytes_received = recv(client->socket, &file_info, sizeof(FileInfo), 0);
        
        if (bytes_received <= 0) {
            remove_client(client_index);
            return;
        }
        
        if (bytes_received != sizeof(FileInfo)) {
            printf("Ошибка: неполная информация о файле\n");
            strcpy(response, "Ошибка при получении информации о файле");
            send(client->socket, response, strlen(response), 0);
            client->waiting_file_info = 0;
            return;
        }
        
        printf("Начало приема файла: %s (%.2f KB)\n", 
               file_info.filename, file_info.filesize / 1024.0);
        
        client->file_ptr = fopen(file_info.filename, "wb");
        if (!client->file_ptr) {
            perror("Ошибка открытия файла");
            strcpy(response, "Ошибка при получении файла");
            send(client->socket, response, strlen(response), 0);
            client->waiting_file_info = 0;
            return;
        }
        
        client->receiving_file = 1;
        client->waiting_file_info = 0;
        client->file_size = file_info.filesize;
        client->file_received = 0;
        strncpy(client->filename, file_info.filename, MAX_FILENAME - 1);
        client->filename[MAX_FILENAME - 1] = '\0';
        
        return;
    }
    
    if (client->receiving_file) {
        char buffer[BUFFER_SIZE];
        long remaining = client->file_size - client->file_received;
        long to_receive = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
        
        ssize_t bytes_received = recv(client->socket, buffer, to_receive, 0);
        if (bytes_received <= 0) {
            remove_client(client_index);
            return;
        }
        
        fwrite(buffer, 1, bytes_received, client->file_ptr);
        client->file_received += bytes_received;
        
        if (client->file_size > 0) {
            printf("\rПрием файла %s: %.2f%%", 
                   client->filename, 
                   (double)client->file_received / client->file_size * 100);
            fflush(stdout);
        }
        
        if (client->file_received >= client->file_size) {
            printf("\nФайл успешно получен: %s\n", client->filename);
            fclose(client->file_ptr);
            client->file_ptr = NULL;
            client->receiving_file = 0;
            
            strcpy(response, "Файл успешно получен");
            send(client->socket, response, strlen(response), 0);
        }
        return;
    }
    
    ssize_t bytes_received = recv(client->socket, client->buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        remove_client(client_index);
        return;
    }
    
    client->buffer[bytes_received] = '\0';
    printf("Клиент %d: %s\n", client->socket, client->buffer);
    
    if (strncmp(client->buffer, "CALC", 4) == 0) {
        char operation[10];
        double num1, num2;
        
        if (sscanf(client->buffer, "CALC %s %lf %lf", operation, &num1, &num2) == 3) {
            handle_calc(operation, num1, num2, response);
        } else {
            strcpy(response, "Ошибка: неправильный формат команды CALC");
        }
        
        send(client->socket, response, strlen(response), 0);
        
    } else if (strncmp(client->buffer, "SEND", 4) == 0) {
        client->waiting_file_info = 1;
        printf("Ожидание информации о файле от клиента %d...\n", client->socket);
        
    } else if (strncmp(client->buffer, "GET", 3) == 0) {
        char filename[MAX_FILENAME];
        
        if (sscanf(client->buffer, "GET %s", filename) == 1) {
            FILE* test_file = fopen(filename, "rb");
            if (!test_file) {
                strcpy(response, "Ошибка: файл не найден");
                send(client->socket, response, strlen(response), 0);
            } else {
                fclose(test_file);
                
                if (send_file(client->socket, filename) == 0) {
                    strcpy(response, "Файл успешно отправлен");
                } else {
                    strcpy(response, "Ошибка при отправке файла");
                }
                send(client->socket, response, strlen(response), 0);
            }
        } else {
            strcpy(response, "Ошибка: укажите имя файла");
            send(client->socket, response, strlen(response), 0);
        }
        
    } else if (strcmp(client->buffer, "EXIT") == 0) {
        strcpy(response, "До свидания!");
        send(client->socket, response, strlen(response), 0);
        remove_client(client_index);
        
    } else if (strcmp(client->buffer, "HELP") == 0) {
        strcpy(response, "Доступные команды:\n"
                         "CALC [ADD|SUB|MUL|DIV|POW] число1 число2 - математические операции\n"
                         "SEND - отправить файл серверу\n"
                         "GET filename - получить файл с сервера\n"
                         "HELP - справка\n"
                         "EXIT - выход");
        send(client->socket, response, strlen(response), 0);
        
    } else {
        strcpy(response, "Неизвестная команда. Введите HELP для справки");
        send(client->socket, response, strlen(response), 0);
    }
    
    memset(client->buffer, 0, BUFFER_SIZE);
}

int main() {
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    fd_set readfds;
    int max_sd, activity;
    
    printf("=== TCP Сервер (мультиплексирование) ===\n");
    
    for (int i = 0; i < max_clients; i++) {
        memset(&clients[i], 0, sizeof(ClientInfo));
    }
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Ошибка setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Ошибка привязки");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Ошибка прослушивания");
        exit(EXIT_FAILURE);
    }
    
    printf("Сервер запущен на порту %d\n", PORT);
    printf("Ожидание подключений...\n");
    
    while (1) {
        FD_ZERO(&readfds);
        
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;
        
        for (int i = 0; i < max_clients; i++) {
            int sd = clients[i].socket;
            
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            
            if (sd > max_sd) {
                max_sd = sd;
            }
        }
        
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        
        if ((activity < 0) && (errno != EINTR)) {
            perror("Ошибка select");
        }
        
        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket;
            
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
                perror("Ошибка accept");
                exit(EXIT_FAILURE);
            }
            
            int client_index = add_client(new_socket, &address);
            if (client_index == -1) {
                printf("Достигнуто максимальное количество клиентов\n");
                char* msg = "Сервер перегружен. Попробуйте позже.\n";
                send(new_socket, msg, strlen(msg), 0);
                close(new_socket);
            }
        }
        
        for (int i = 0; i < max_clients; i++) {
            int sd = clients[i].socket;
            
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                process_command(i);
            }
        }
    }
    
    close(server_fd);
    return 0;
}