#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void addContactId(contact* data, int value){
    data->id = value;
}

void addContactNum(contact* data, char** value ,int count){
    for(int i = 0; i < count; i++){
        for(int j = 0; j <LEN_PHONE_NUM; j++){
            data->phone_num[i][j] = value[i][j];
        }
    }
    data->count_num = count;
}

void addContactName(contact* data, char value[LEN_NAME]){
    for(int i = 0; i < LEN_NAME; i++){
        data->name[i] = value[i];
    }
    
}

void addContactSurname(contact* data, char value[LEN_NAME]){
    for(int i = 0; i < LEN_NAME; i++){
        data->surname[i] = value[i];
    }

}

void addContactPatronymic(contact* data, char value[LEN_NAME]){
    for(int i = 0; i < LEN_NAME; i++){
        data->patronymic[i] = value[i];
    }
}

void addContactWork(contact* data, char value[LEN_NAME]){
    for(int i = 0; i < LEN_NAME; i++){
        data->work[i] = value[i];
    }
}

void addContactPosition(contact* data, char value[LEN_NAME]){
    for(int i = 0; i < LEN_NAME; i++){
        data->position[i] = value[i];
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

contact* addContact(int id){
    contact* contic = calloc(1, sizeof(contact));
    addContactId(contic, id);
    printf("Enter surname\n");
    char* data = inputStr();
    if(data[0] == '*'){
        printf("This is a required field");
        free(data);
        return NULL;
    }
    else{
        addContactSurname(contic, data);
    }
    printf("Enter name\n");
    data = inputStr();
    if(data[0] == '*'){
        printf("This is a required field");
        free(data);
        return NULL;
    }
    else{
        addContactName(contic, data);
    }
    printf("Enter patronymic\n");
    data = inputStr();
    if(data[0] != '*'){
        addContactPatronymic(contic, data);
    }
    printf("Enter number of phone numbers\n");
    int count_phone_numbers;
    if(scanf("%d", &count_phone_numbers) != 1){
        free(data);
        return NULL;
    }
    char check = ' ';
    if(scanf("%c", &check) != 1 || check != '\n'){
        while ((check = getchar()) != '\n' && check != EOF);
        free(data);
        return NULL;
    }
    char** phone_number = calloc(count_phone_numbers, sizeof(char*));
    for(int i = 0; i < count_phone_numbers; i++){
        phone_number[i] = calloc(11, sizeof(char));
    }
    for(int i = 0; i < count_phone_numbers; i++){
        for(int j = 0; j < 11; j++){
            if(scanf("%c", &phone_number[i][j]) != 1){
                free(data);
                return NULL;
            }
        }
        check = ' ';
        if(scanf("%c", &check) != 1 || check != '\n'){
            while ((check = getchar()) != '\n' && check != EOF);
            free(data);
            return NULL;
        }
    }
    addContactNum(contic, phone_number, count_phone_numbers);
    printf("Enter work\n");
    data = inputStr();
    if(data[0] != '*'){
        addContactWork(contic, data);
    }
    printf("Enter position\n");
    data = inputStr();
    if(data[0] != '*'){
        addContactPosition(contic, data);
    }
    return contic;
}

void printNumberPhone(contact* data, int count){
    for(int i = 0; i < count; i++){
        printf("phone number %d - ", i+1);
        for(int j = 0; j < 11; j++){
            printf("%c", data->phone_num[i][j]);
        }
        printf("\n");
    }
}

void printString(char* value){
    int n = 0;
    while(value[n] != '*'){
        printf("%c", value[n]);
        n++;
    }
    printf("\n");
}

void printContact(contact* data){
    printf("Id - ");
    printf("%d\n", data->id);
    if(data->surname[0] != '*'){
        printf("Surname - ");
        printString(data->surname);
    }
    if(data->name[0] != '*'){
        printf("Name - ");
        printString(data->name);
    }
    if(data->patronymic[0] != '*'){
        printf("patronymic - ");
        printString(data->patronymic);
    }
    printNumberPhone(data, data->count_num);
    if(data->work[0] != '*'){
        printf("Work - ");
        printString(data->work);
    }
    if(data->position[0] != '*'){
        printf("Position - ");
        printString(data->position);
    }
}

void shiftContact(contact** contacts, int num, int count){
    for(int i = num; i < count - 1; i++){
        contacts[i] = contacts[i+1];
    }
}

void freeNode(node* data){
    free(data->next);
    free(data->pred);
}

list* delContact(int id, list* data, int* count){
    printf("Enter id\n");
    int num;
    char check = ' ';
    if(scanf("%d", &num) != 1 || (num > id && num < 0)){
        printf("Incorrect num\n");
    }
    else if(scanf("%c", &check) != 1 || check != '\n'){
        while ((check = getchar()) != '\n' && check != EOF);
        printf("Incorrect input\n");    
    }
    else{
        node* prev = data->head;
        node* item;
        if(data == NULL) return NULL;
        
        if(data->head->data->id == num){
            item = data->head;
            data->head = data->head->next;
            free(item);
            count--;
            return data;
        }

        while(prev->next != NULL){
            if(prev->next->data->id == num){
                item = prev->next;
                prev->next = prev->next->next;
                free(item);
                count--;
                return data;
            }
            else prev = prev->next;
        }
    }
    return data;
}

short int replacePhoneNumber(contact* contact){
    printf("Enter num\n");
    int num;
    int flag = 0;
    char check = ' ';
    if(scanf("%d", &num) != 1 || (num > contact->count_num && num <= 0)){
        printf("%d\n", num);
        flag = 1;
    }
    else if(scanf("%c", &check) != 1 || check != '\n'){
        while ((check = getchar()) != '\n' && check != EOF);
        flag = 1;
    }
    else{
        printf("Enter nubmer phone\n");
        for(int j = 0; j < 11; j++){
            if(scanf("%c", &contact->phone_num[num - 1][j]) != 1){
                flag = 1;
            }
        }
        check = ' ';
        if(scanf("%c", &check) != 1 || check != '\n'){
            while ((check = getchar()) != '\n' && check != EOF);
            flag = 1;
        }
    }
    return flag;
}

short int replaceContact(contact* contact){
    while(1){
        printf("Enter 1 - name, 2 - surname, 3 - patronymic, 4 - phone_num, 5 - work, 6 - position, 0 - Exit\n");
        char num, check;
        scanf("%c", &num);
        if(scanf("%c", &check) != 1 || check != '\n'){
            printf("Incorrect input\n");
            num = 'q';
            while(check != '\n'){
                scanf("%c", &check);
            }
            continue;
        }
        char* data;
        switch (num)
        {
        case '1':
            printf("Enter name\n");
            data = inputStr();
            if(data[0] != '*'){
                addContactName(contact, data);
                free(data);
            }
            else{
                printf("The value has not been changed\n");
            }
            break;

        case '2':
            printf("Enter surname\n");
            data = inputStr();
            if(data[0] != '*'){
                addContactSurname(contact, data);
                free(data);
            }
            else{
                printf("The value has not been changed\n");
            }
            break;
        case '3':
            printf("Enter patronymic\n");
            data = inputStr();
            if(data[0] != '*'){
                addContactPatronymic(contact, data);
            }
            else{
                printf("The value has not been changed\n");
            }
            break;

        case '4':
            if(replacePhoneNumber(contact) == 1){
                printf("Eroor\n");
            }
            break;

        case '5':
            printf("Enter work\n");
            data = inputStr();
            if(data[0] != '*'){
                addContactWork(contact, data);
            }
            else{
                printf("The value has not been changed\n");
            }
            break;

        case '6':
            printf("Enter position\n");
            data = inputStr();
            if(data[0] != '*'){
                addContactPosition(contact, data);
            }
            else{
                printf("The value has not been changed\n");
            }
            break;

        case '0':
            return 2;
        
        default:
            printf("Incorrect input\n");
            continue;
        }
    }
}

short int replaceContactCheckID(int id, list* data){
    printf("Enter id\n");
    int num;
    int flag = 0;
    char check = ' ';
    if(scanf("%d", &num) != 1 || (num > id && num < 0)){
        printf("%d\n", num);
        flag = 1;
    }
    else if(scanf("%c", &check) != 1 || check != '\n'){
        while ((check = getchar()) != '\n' && check != EOF);
        flag = 1;
    }
    else{
        for(node* i = data->head; i != NULL; i = i->next){
            if(i->data->id == num){
                flag = replaceContact(i->data);
                break;
            }
        }
    }
    return flag;
}


short int pushList(int id, list* data){
    short int flag = 0;
    node* newNode = initNode();
    newNode->data = addContact(id);
    if(!newNode->data){
        flag = 1;
    }
    else{
        if(data->head == NULL){
            data->head = newNode;
        }
        else{
            node* tmp = data->head;
            while(tmp->next != NULL){
                tmp = tmp->next;
            }
            tmp->next = newNode;
            newNode->pred = tmp;
        }
    }
    return flag;
}

void printList(list* data){
    for(node* tmp = data->head; tmp != NULL; tmp = tmp->next){
        printContact(tmp->data);
    }
}



list* initList(){
    list* list = calloc(1, sizeof(list));
    list->head = NULL;
    return list;
}

node* initNode(){
    node* tmp = calloc(1, sizeof(node));
    tmp->data = calloc(1, sizeof(contact));
    tmp->next = NULL;
    tmp->pred = NULL;
    return tmp;
}

short int pushListFile(list* data, contact* contic){
    short int flag = 0;
    node* newNode = initNode();
    newNode->data = contic;
    if(!newNode->data){
        flag = 1;
    }
    else{
        if(data->head == NULL){
            data->head = newNode;
        }
        else{
            node* tmp = data->head;
            while(tmp->next != NULL){
                tmp = tmp->next;
            }
            tmp->next = newNode;
            newNode->pred = tmp;
        }
    }
    return flag;
}
