#include <stdio.h>
#include <stdlib.h>
#include "list.h"



int main(){
    int id = 0;
    list* contacts = initList();
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
            if(pushList(id, contacts) == 1){
                printf("Incorrect input\n");
                id--;
            }
            break;

        case '2':
            error = replaceContactCheckID(id, contacts);
            if(error == 1){
                printf("Incorrect input\n");
            }
            else if(error == 0){
                printf("Id is not valid\n");
            }
            break;
        case '3':
            contacts = delContact(id, contacts);
            break;

        case '4':
            printList(contacts);
            break;

        case '0':
            
            return 0;
        
        default:
            printf("Incorrect input\n");
            continue;
        }
    }
}