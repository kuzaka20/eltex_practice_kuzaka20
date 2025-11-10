#include <stdio.h>
#include <stdlib.h>
#include "tree.h"



int main(){
    int id = 0;
    tree* contacts = initTree();
    while(1){
        printf("Enter 1 - Add, 2 - Replace, 3 - Delete, 4 - Print, 5 - Balance Tree, 6 - Print root, 7 - Print Tree 0 - Exit\n");
        char num, check;
        int id_num;
        scanf("%c", &num);
        if(scanf("%c", &check) != 1 || check != '\n'){
            printf("Incorrect input\n");
            num = 'q';
            while(check != '\n'){
                scanf("%c", &check);
            }
            continue;
        }
        int error;
        switch (num)
        {
        case '1':
            id++;
            if(pushTree(id, &(contacts->root)) == 1){
                printf("Incorrect input\n");
                id--;
            }
            break;

        case '2':
            printf("Enter id\n");
            check = ' ';
            if(scanf("%d", &id_num) != 1 || (id_num > id && id_num < 0)){
                printf("%d\n", id_num);
            }
            else if(scanf("%c", &check) != 1 || check != '\n'){
                while ((check = getchar()) != '\n' && check != EOF);
            }
            error = replaceContactCheckID(id, &(contacts->root));
            if(error == 1){
                printf("Incorrect input\n");
            }
            else if(error == 0){
                printf("Id is not valid\n");
            }
            break;
        case '3':
            printf("Enter id\n");

            check = ' ';
            if(scanf("%d", &id_num) != 1 || (id_num > id && id_num < 0)){
                printf("Incorrect num\n");
            }
            else if(scanf("%c", &check) != 1 || check != '\n'){
                while ((check = getchar()) != '\n' && check != EOF);
                printf("Incorrect input\n");    
            }
            if(delContact(id_num, &(contacts->root)) == 0){
                printf("Id is not valid\n");
            }
            break;

        case '4':
            printTree(contacts->root);
            break;

        case '5':
            balanceTree(contacts);
            printf("Tree balanced successfully!\n");
            break;

        case'6':
            printRoot(contacts);
            break;
            
        case'7':
            displayTree(contacts);
            break;

        case '0':
            
            return 0;
        
        default:
            printf("Incorrect input\n");
            continue;
        }
    }
}