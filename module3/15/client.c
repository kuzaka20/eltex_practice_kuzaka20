#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char filename[256];
    long filesize;
} FileInfo;

int send_file(int socket_fd, const char* filename) {
    FILE* file;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    FileInfo file_info;
    
    file = fopen(filename, "rb");
    if (!file) {
        printf("Ошибка: не удалось открыть файл %s\n", filename);
        return -1;
    }
    
    fseek(file, 0, SEEK_END);
    file_info.filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    strcpy(file_info.filename, filename);
    
    printf("Отправка файла: %s (%.2f KB)\n", 
           filename, file_info.filesize / 1024.0);
    
    if (send(socket_fd, &file_info, sizeof(FileInfo), 0) <= 0) {
        printf("Ошибка отправки информации о файле\n");
        fclose(file);
        return -1;
    }
    
    usleep(10000);
    
    long total_sent = 0;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t sent = send(socket_fd, buffer, bytes_read, 0);
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

int receive_file(int socket_fd, const char* expected_filename) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    FILE* file;
    FileInfo file_info;
    
    bytes_received = recv(socket_fd, &file_info, sizeof(FileInfo), 0);
    if (bytes_received <= 0) {
        printf("Ошибка: не удалось получить информацию о файле\n");
        return -1;
    }
    
    if (bytes_received != sizeof(FileInfo)) {
        printf("Ошибка: неполная информация о файле\n");
        return -1;
    }
    
    if (expected_filename && strlen(expected_filename) > 0 && 
        strcmp(file_info.filename, expected_filename) != 0) {
        printf("Внимание: получен файл с именем '%s', ожидался '%s'\n", 
               file_info.filename, expected_filename);
    }
    
    printf("Прием файла: %s (%.2f KB)\n", 
           file_info.filename, 
           file_info.filesize / 1024.0);
    
    file = fopen(file_info.filename, "wb");
    if (!file) {
        printf("Ошибка: не удалось создать файл %s\n", file_info.filename);
        return -1;
    }
    
    long total_received = 0;
    while (total_received < file_info.filesize) {
        long remaining = file_info.filesize - total_received;
        long to_receive = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
        
        bytes_received = recv(socket_fd, buffer, to_receive, 0);
        if (bytes_received <= 0) {
            break;
        }
        
        fwrite(buffer, 1, bytes_received, file);
        total_received += bytes_received;
        
        if (file_info.filesize > 0) {
            printf("\rПрогресс: %.2f%%", 
                   (double)total_received / file_info.filesize * 100);
            fflush(stdout);
        }
    }
    
    printf("\n");
    fclose(file);
    
    if (total_received == file_info.filesize) {
        printf("Файл успешно получен: %s\n", file_info.filename);
        return 0;
    } else {
        printf("Ошибка: получено %ld из %ld байт\n", total_received, file_info.filesize);
        remove(file_info.filename);
        return -1;
    }
}

int get_server_response(int sock, char* buffer, int buffer_size, int timeout_sec) {
    fd_set readfds;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    
    int ret = select(sock + 1, &readfds, NULL, NULL, &tv);
    
    if (ret > 0) {
        ssize_t valread = recv(sock, buffer, buffer_size - 1, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            return 1;
        } else if (valread == 0) {
            strcpy(buffer, "Сервер закрыл соединение");
            return -1;
        } else {
            perror("recv");
            strcpy(buffer, "Ошибка получения ответа");
            return -1;
        }
    } else if (ret == 0) {
        strcpy(buffer, "Таймаут ожидания ответа");
        return 0;
    } else {
        perror("select");
        strcpy(buffer, "Ошибка select");
        return -1;
    }
}

void print_menu() {
    printf("\n=== Меню ===\n");
    printf("1. Калькулятор\n");
    printf("2. Отправить файл серверу\n");
    printf("3. Получить файл с сервера\n");
    printf("4. Справка\n");
    printf("5. Выход\n");
    printf("Выберите действие: ");
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    printf("=== TCP Клиент ===\n");
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nОшибка создания сокета\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nНеверный адрес / Адрес не поддерживается\n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nОшибка подключения\n");
        return -1;
    }
    
    printf("Подключено к серверу\n");
    
    int choice;
    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        
        switch (choice) {
            case 1: {
                char operation[10];
                double num1, num2;
                
                printf("Выберите операцию (ADD, SUB, MUL, DIV, POW): ");
                if (scanf("%s", operation) != 1) {
                    printf("Ошибка ввода операции\n");
                    break;
                }
                printf("Введите первое число: ");
                if (scanf("%lf", &num1) != 1) {
                    printf("Ошибка ввода первого числа\n");
                    break;
                }
                printf("Введите второе число: ");
                if (scanf("%lf", &num2) != 1) {
                    printf("Ошибка ввода второго числа\n");
                    break;
                }
                while (getchar() != '\n');
                
                snprintf(buffer, BUFFER_SIZE, "CALC %s %.2lf %.2lf", operation, num1, num2);
                
                if (send(sock, buffer, strlen(buffer), 0) <= 0) {
                    printf("Ошибка отправки команды\n");
                    break;
                }
                printf("Отправлено: %s\n", buffer);
                
                if (get_server_response(sock, buffer, BUFFER_SIZE, 5) > 0) {
                    printf("Ответ сервера: %s\n", buffer);
                } else {
                    printf("Ответ сервера: %s\n", buffer);
                }
                break;
            }
            
            case 2: {
                char filename[256];
                printf("Введите имя файла для отправки: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                
                FILE* test_file = fopen(filename, "rb");
                if (!test_file) {
                    printf("Ошибка: файл '%s' не существует или недоступен\n", filename);
                    break;
                }
                fclose(test_file);
                
                strcpy(buffer, "SEND");
                if (send(sock, buffer, strlen(buffer), 0) <= 0) {
                    printf("Ошибка отправки команды SEND\n");
                    break;
                }
                printf("Отправлено: %s\n", buffer);
                
                if (send_file(sock, filename) == 0) {
                    if (get_server_response(sock, buffer, BUFFER_SIZE, 10) > 0) {
                        printf("Ответ сервера: %s\n", buffer);
                    } else {
                        printf("Ответ сервера: %s\n", buffer);
                    }
                } else {
                    printf("Ошибка при отправке файла\n");
                }
                break;
            }
            
            case 3: {
                char filename[256];
                printf("Введите имя файла для получения: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                
                snprintf(buffer, BUFFER_SIZE, "GET %s", filename);
                
                if (send(sock, buffer, strlen(buffer), 0) <= 0) {
                    printf("Ошибка отправки команды GET\n");
                    break;
                }
                printf("Отправлено: %s\n", buffer);
                
                if (receive_file(sock, filename) == 0) {
                    if (get_server_response(sock, buffer, BUFFER_SIZE, 5) > 0) {
                        printf("Ответ сервера: %s\n", buffer);
                    } else {
                        printf("Ответ сервера: %s\n", buffer);
                    }
                } else {
                    printf("Ошибка при получении файла\n");
                }
                break;
            }
            
            case 4: {
                strcpy(buffer, "HELP");
                if (send(sock, buffer, strlen(buffer), 0) <= 0) {
                    printf("Ошибка отправки команды HELP\n");
                    break;
                }
                printf("Отправлено: %s\n", buffer);
                
                if (get_server_response(sock, buffer, BUFFER_SIZE, 5) > 0) {
                    printf("Ответ сервера:\n%s\n", buffer);
                } else {
                    printf("Ответ сервера: %s\n", buffer);
                }
                break;
            }
            
            case 5: {
                strcpy(buffer, "EXIT");
                if (send(sock, buffer, strlen(buffer), 0) <= 0) {
                    printf("Ошибка отправки команды EXIT\n");
                } else {
                    printf("Отправлено: %s\n", buffer);
                    
                    if (get_server_response(sock, buffer, BUFFER_SIZE, 5) > 0) {
                        printf("Ответ сервера: %s\n", buffer);
                    }
                }
                
                close(sock);
                printf("Соединение закрыто\n");
                return 0;
            }
            
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
                continue;
        }
    }
    
    close(sock);
    return 0;
}