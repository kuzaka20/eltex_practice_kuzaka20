#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int readFile(list* data, int* count, int* id){
    FILE *file = fopen("list.data", "r");
    if(file == NULL){
        printf("Error not open file\n");
        return 1;
    }
    while(1){
        contact* contic = malloc(sizeof(contact));
        size_t read_count = fread(contic,sizeof(contact), 1, file);
        if(read_count == 1){
            if(pushListFile(data, contic) == 1){
                printf("Incorrect read file\n");
                return 1;
            }
            else{
                (*count)++;
                *id = data->head->data->id;
            }
        }
        else{
            break;
        }
    }
    fclose(file);
    return 0;
}

int writeFile(list* data, int count){
    FILE *file = fopen("list.txt", "w");
    if(file == NULL){
        printf("Error not open file\n");
        return 1;
    }
    node* current = data->head;
    int written = 0;
    
    while(current != NULL && written < count){
        size_t write_count = fwrite(current->data, sizeof(contact), 1, file);
        if(write_count != 1){
            printf("Error not write in file\n");
            fclose(file);
            return 1;
        }
        current = current->next;
        written++;
    }
    
    fclose(file);
    return 0;
}