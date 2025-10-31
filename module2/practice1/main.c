#include <stdio.h>
#include <stdlib.h>
#include "phone_contact.h"



int main(){
    int id = 0, count = 0;
    contact* contacts[LEN_CONTACT];
    while(1){
        printf("Enter 1 - Add, 2 - Replace, 3 - Delete, 4 - Print, 0 - Exit\n");
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
        short int error;
        switch (num)
        {
        case '1':
            id++;
            count++;
            if(addContact(id, contacts, count) == 0){
                printf("Incorrect input\n");
                id--;
                count--;
            }
            break;

        case '2':
            error = replaceContactCheckID(id, contacts, count);
            if(error == 1){
                printf("Incorrect input\n");
            }
            else if(error == 0){
                printf("Id is not valid\n");
            }
            break;
        case '3':
            error = delContact(id, contacts, count);
            count--;
            if(error == 1){
                printf("Incorrect input\n");
                count++;
            }
            else if(error == 0){
                printf("Id is not valid\n");
                count++;
            }
            break;

        case '4':
            for(int i = 0; i < count; i++){
                printContact(contacts[i]);
            }
            break;

        case '0':
            
            return 0;
        
        default:
            printf("Incorrect input\n");
            continue;
        }
    }
}