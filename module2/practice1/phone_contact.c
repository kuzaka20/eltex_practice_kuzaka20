#include <stdio.h>
#include <stdlib.h>
#include "phone_contact.h"
 
void addContactId(contact* data, int value){
    data->id = value;
}

void addContactNum(contact* data, char** value ,int count){
    data->phone_num = value;
    data->count_num = count;
}

void addContactName(contact* data, char* value){
    data->name = value;
}

void addContactSurname(contact* data, char* value){

    data->surname = value;

}

void addContactPatronymic(contact* data, char* value){
    data->patronymic = value;
}

void addContactWork(contact* data, char* value){
    data->work = value;
}

void addContactPosition(contact* data, char* value){
    data->position = value;
}
void printString(char* value){
    int n = 0;
    while(value[n] != '*'){
        printf("%c", value[n]);
        n++;
    }
    printf("\n");
}
void printNumberPhone(char** data, int count){
    for(int i = 0; i < count; i++){
        printf("phone number %d - ", i+1);
        for(int j = 0; j < 11; j++){
            printf("%c", data[i][j]);
        }
        printf("\n");
    }
}

void printContact(contact* data){
    if(data != NULL){
        printf("Id - ");
        printf("%d\n", data->id);
        if(data->surname != NULL){
            printf("Surname - ");
            printString(data->surname);
        }
        if(data->name != NULL){
            printf("Name - ");
            printString(data->name);
        }
        if(data->patronymic != NULL){
            printf("patronymic - ");
            printString(data->patronymic);
        }
        printNumberPhone(data->phone_num, data->count_num);
        if(data->work != NULL){
            printf("Work - ");
            printString(data->work);
        }
        if(data->position != NULL){
            printf("Position - ");
            printString(data->position);
        }
    }
}

char* inputStr(){
    char c = ' ';
    int n = 1;
    char* data = calloc(n, sizeof(char));
    while(scanf("%c", &c) != 1 || c!='\n'){
        data[n-1]= c;
        n++;
        data = realloc(data, n);
    }
    data[n]='*';
    return data;
}

int addContact(int id, contact** contacts){
    contact* contic = calloc(1 ,sizeof(contact));
    addContactId(contic, id);
    printf("Enter surname\n");
    char* data = inputStr();
    if(data[0] == '*'){
        printf("This is a required field");
        free(contic);
        free(data);
        return 0;
    }
    else{
        addContactSurname(contic, data);
    }
    printf("Enter name\n");
    data = inputStr();
    if(data[0] == '*'){
        printf("This is a required field");
        free(contic);
        free(data);
        return 0;
    }
    else{
        addContactName(contic, data);
    }
    printf("Enter patronymic\n");
    data = inputStr();
    if(data[0] == '*'){
        addContactPatronymic(contic, NULL);
    }
    else{
        addContactPatronymic(contic, data);
    }
    printf("Enter number of phone numbers\n");
    int count_phone_numbers;
    if(scanf("%d", &count_phone_numbers) != 1){
        free(contic);
        free(data);
        return 0;
    }
    char check = ' ';
    if(scanf("%c", &check) != 1 || check != '\n'){
        while(check != '\n'){
            scanf("%c", &check);
        }
        free(contic);
        free(data);
        return 0;
    }
    char** phone_number = calloc(count_phone_numbers, sizeof(char*));
    for(int i = 0; i < count_phone_numbers; i++){
        phone_number[i] = calloc(11, sizeof(char));
    }
    for(int i = 0; i < count_phone_numbers; i++){
        for(int j = 0; j < 11; j++){
            if(scanf("%c", &phone_number[i][j]) != 1){
                free(contic);
                free(data);
                return 0;
            }
        }
        check = ' ';
        if(scanf("%c", &check) != 1 || check != '\n'){
            while(check != '\n'){
                scanf("%c", &check);
            }
            return 0;
            free(contic);
            free(data);
        }
    }
    addContactNum(contic, phone_number, count_phone_numbers);
    printf("Enter work\n");
    data = inputStr();
    if(data[0] == '*'){
        addContactWork(contic, NULL);
    }
    else{
        addContactWork(contic, data);
    }
    printf("Enter position\n");
    data = inputStr();
    if(data[0] == '*'){
        addContactPosition(contic, NULL);
    }
    else{
        addContactPosition(contic, data);
    }
    contacts = realloc(contacts ,sizeof(contact*) * id);
    contacts[id] = contic;
    return 1;
}

void delNumberPhone(char** data, int count){
    for(int i = 0; i < count; i++){
        free(data[i]);
    }
    free(data);
}

int delContact(int id, contact** contacts){
    printf("Enter id\n");
    int num;
    int flag = 0;
    char check = ' ';
    if(scanf("%d", &num) != 1 || (num > id && num < 0)){
        printf("%d\n", num);
        flag = 1;
    }
    else if(scanf("%c", &check) != 1 || check != '\n'){
        while(check != '\n'){
            scanf("%c", &check);
        }
        flag = 1;
    }
    else{
        for(int i = 0; i <= id; i++){
            if (contacts[i] != NULL){
                if(contacts[i]->id == num){
                    contacts[i] = NULL;
                    flag = 2;
                    break;
                }
            }
        }
    }
    return flag;

}

int replaceContact(int id, contact** contacts){
     printf("Enter id\n");
    int num;
    int flag = 0;
    char check = ' ';
    if(scanf("%d", &num) != 1 || (num > id && num < 0)){
        printf("%d\n", num);
        flag = 1;
    }
    else if(scanf("%c", &check) != 1 || check != '\n'){
        while(check != '\n'){
            scanf("%c", &check);
        }
        flag = 1;
    }
    else{
        
    }
    return flag;

}