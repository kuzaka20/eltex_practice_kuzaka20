#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
    char filename[256];
    long filesize;
} FileInfo;

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

int receive_file(int client_socket, FileInfo* file_info) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    FILE* file;
    
    bytes_received = recv(client_socket, file_info, sizeof(FileInfo), 0);
    if (bytes_received <= 0) {
        printf("Ошибка: не удалось получить информацию о файле\n");
        return -1;
    }
    
    printf("Прием файла: %s (%.2f KB)\n", 
           file_info->filename, 
           file_info->filesize / 1024.0);
    
    file = fopen(file_info->filename, "wb");
    if (!file) {
        perror("Ошибка открытия файла");
        return -1;
    }
    
    long total_received = 0;
    while (total_received < file_info->filesize) {
        long remaining = file_info->filesize - total_received;
        long to_receive = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
        
        bytes_received = recv(client_socket, buffer, to_receive, 0);
        if (bytes_received <= 0) {
            break;
        }
        
        fwrite(buffer, 1, bytes_received, file);
        total_received += bytes_received;
        
        printf("\rПрогресс: %.2f%%", 
               (double)total_received / file_info->filesize * 100);
        fflush(stdout);
    }
    
    printf("\n");
    fclose(file);
    
    if (total_received == file_info->filesize) {
        printf("Файл успешно получен\n");
        return 0;
    } else {
        printf("Ошибка: получено %ld из %ld байт\n", total_received, file_info->filesize);
        remove(file_info->filename);
        return -1;
    }
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
    strcpy(file_info.filename, filename);
    
    if (send(client_socket, &file_info, sizeof(FileInfo), 0) <= 0) {
        printf("Ошибка отправки информации о файле\n");
        fclose(file);
        return -1;
    }
    
    printf("Отправка файла: %s (%.2f KB)\n", 
           filename, file_info.filesize / 1024.0);
    
    long total_sent = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) <= 0) {
            printf("Ошибка отправки данных файла\n");
            break;
        }
        total_sent += bytes_read;
        
        printf("\rПрогресс: %.2f%%", 
               (double)total_sent / file_info.filesize * 100);
        fflush(stdout);
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

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    ssize_t bytes_received;
    
    free(arg);
    
    printf("Клиент подключен\n");
    
    while (1) {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Клиент отключился\n");
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("Получено: %s\n", buffer);
        
        if (strncmp(buffer, "CALC", 4) == 0) {
            char operation[10];
            double num1, num2;
            
            if (sscanf(buffer, "CALC %s %lf %lf", operation, &num1, &num2) == 3) {
                handle_calc(operation, num1, num2, response);
            } else {
                strcpy(response, "Ошибка: неправильный формат команды CALC");
            }
            
            send(client_socket, response, strlen(response), 0);
            
        } else if (strncmp(buffer, "SEND", 4) == 0) {
            FileInfo file_info;
            
            if (receive_file(client_socket, &file_info) == 0) {
                strcpy(response, "Файл успешно получен");
            } else {
                strcpy(response, "Ошибка при получении файла");
            }
            
            send(client_socket, response, strlen(response), 0);
            
        } else if (strncmp(buffer, "GET", 3) == 0) {
            char filename[256];
            
            if (sscanf(buffer, "GET %s", filename) == 1) {
                FILE* test_file = fopen(filename, "rb");
                if (!test_file) {
                    strcpy(response, "Ошибка: файл не найден");
                    send(client_socket, response, strlen(response), 0);
                } else {
                    fclose(test_file);
                    
                    if (send_file(client_socket, filename) == 0) {
                        strcpy(response, "Файл успешно отправлен");
                    } else {
                        strcpy(response, "Ошибка при отправке файла");
                    }
                    send(client_socket, response, strlen(response), 0);
                }
            } else {
                strcpy(response, "Ошибка: укажите имя файла");
                send(client_socket, response, strlen(response), 0);
            }
            
        } else if (strcmp(buffer, "EXIT") == 0) {
            strcpy(response, "До свидания!");
            send(client_socket, response, strlen(response), 0);
            break;
            
        } else if (strcmp(buffer, "HELP") == 0) {
            strcpy(response, "Доступные команды:\n"
                             "CALC [ADD|SUB|MUL|DIV|POW] число1 число2 - математические операции\n"
                             "SEND - отправить файл серверу\n"
                             "GET filename - получить файл с сервера\n"
                             "HELP - справка\n"
                             "EXIT - выход");
            send(client_socket, response, strlen(response), 0);
            
        } else {
            strcpy(response, "Неизвестная команда. Введите HELP для справки");
            send(client_socket, response, strlen(response), 0);
        }
    }
    
    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    
    printf("=== TCP Сервер ===\n");
    
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
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Ошибка accept");
            continue;
        }
        
        printf("Новое подключение: %s:%d\n", 
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        pthread_t thread_id;
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_fd;
        
        if (pthread_create(&thread_id, NULL, handle_client, (void*)new_sock) != 0) {
            perror("Ошибка создания потока");
            close(client_fd);
            free(new_sock);
        }

        pthread_detach(thread_id);
    }
    
    close(server_fd);
    return 0;
}