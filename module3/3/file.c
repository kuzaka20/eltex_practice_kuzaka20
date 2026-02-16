#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "list.h"

int readFile(list* data, int* count, int* id){
    int fd = open("list.data", O_RDONLY);
    if(fd == -1){
        // Файл не существует, создаем новый
        fd = open("list.data", O_WRONLY | O_CREAT, 0644);
        if(fd == -1){
            printf("Error: cannot create or open file\n");
            return 1;
        }
        printf("File created successfully\n");
        close(fd);
        return 0;
    }
    
    while(1){
        contact* contic = malloc(sizeof(contact));
        ssize_t bytes_read = read(fd, contic, sizeof(contact));
        if(bytes_read == sizeof(contact)){
            if(pushListFile(data, contic) == 1){
                printf("Incorrect read file\n");
                free(contic);
                close(fd);
                return 1;
            }
            else{
                (*count)++;
                if(data->head != NULL && data->head->data != NULL) {
                    node* current = data->head;
                    *id = current->data->id;
                    while(current != NULL) {
                        if(current->data->id > *id) {
                            *id = current->data->id;
                        }
                        current = current->next;
                    }
                }
            }
        }
        else{
            free(contic);
            break;
        }
    }
    close(fd);
    return 0;
}

int writeFile(list* data, int count){
    int fd = open("list.data", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        printf("Error not open file\n");
        return 1;
    }
    
    node* current = data->head;
    int written = 0;
    
    while(current != NULL && written < count){
        ssize_t bytes_written = write(fd, current->data, sizeof(contact));
        if(bytes_written != sizeof(contact)){
            printf("Error not write in file\n");
            close(fd);
            return 1;
        }
        current = current->next;
        written++;
    }
    
    close(fd);
    return 0;
}