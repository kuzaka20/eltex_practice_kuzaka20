#include <stdio.h>

int inputArg(double* argument1, double* argument2){
    char check;
    int flag = 1;
    printf("Enter argument 1\n");
    if(scanf("%lf", argument1) != 1){
        printf("Incorrect input\n");
    }
    if(scanf("%c", &check) != 1 || check != '\n'){
        while ((check = getchar()) != '\n' && check != EOF);
        flag = 0;
    }
    if (flag == 1){
        printf("Enter argument 2\n");
        if(scanf("%lf", argument2) != 1){
            printf("Incorrect input\n");
        }
        if(scanf("%c", &check) != 1 || check != '\n'){
            while ((check = getchar()) != '\n' && check != EOF);
            flag = 0;
        }
    }
    return flag;
}

int inputOper(char* operation){
    char check;
    int flag = 1;
    if(scanf("%c", operation) != 1){
        flag = 0;

    }
    if(scanf("%c", &check) != 1 || check != '\n'){
        while ((check = getchar()) != '\n' && check != EOF);
        flag = 0;
    }
    return flag;

}

int main(){
    while(1){
        printf("Enter arithmetic operation or 0 - exit\n");
        char operation;
        if(inputOper(&operation) == 0){
            printf("Incorrect input\n");
            continue;
        }
        double argument1, argument2;
        switch (operation)
        {
        case '*':
            if (inputArg(&argument1, &argument2) == 1){
                printf("%lf\n", argument1 * argument2);
            }
            else{
                printf("Incorrect input\n");
            }
            break;
        case '/':
            if (inputArg(&argument1, &argument2) == 1){
                printf("%lf\n", argument1 / argument2);
            }
            else{
                printf("Incorrect input\n");
            }
            break;
        
        case '+':
            if (inputArg(&argument1, &argument2) == 1){
                printf("%lf\n", argument1 + argument2);
            }
            else{
                printf("Incorrect input\n");
            }
            break;
        
        case '-':
            if (inputArg(&argument1, &argument2) == 1){
                printf("%lf\n", argument1 - argument2);
            }
            else{
                printf("Incorrect input\n");
            }
            break;
        case '0':
            return 0;
        
        default:
            printf("Incorrect input\n");
            break;
        }
    }
}