#include <stdio.h>
#include <stdlib.h>
#include "phone_contact.h"



int main(){
    int id = 0;
    contact** contacts = calloc(id ,sizeof(contact*));
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
        switch (num)
        {
        case '1':
            id++;
            if(addContact(id, contacts) == 0){
                printf("Incorrect input\n");
                id--;
            }
            break;

        case '2':
            break;
        case '3':
            short int error = delContact(id, contacts);
            if(error == 1){
                printf("Incorrect input\n");
            }
            else if(error == 0){
                printf("Id is not valid\n");
            }
            break;

        case '4':
            for(int i = 1; i < id + 1; i++){
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