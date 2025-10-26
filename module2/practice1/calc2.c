#include <stdio.h>
#include <stdlib.h>

int charInInt(int x, int y, char* data){
    int num = 0;
    for(int i = x; i < y; i++){
        num *= 10;
        num += data[i] - '0';
    }
    printf("1 - %d 2 - %d 3 - %d\n", num, x, y);
    return num;
}

int sum(int x , int y){

    return (x+y);

}

int sub(int x , int y){

    return (x-y);

}

int division(int x , int y){

    return (x/y);

}

int mul(int x , int y){

    return (x*y);

}
void shift(char *operator, int* argument, int num, int result ,int count_operator){
    char* tmp_operator = calloc(count_operator - 1, sizeof(char)); 
    for(int i = 0; i < num; i++){
        tmp_operator[i] = operator[i];
    }
    for(int i = num; i < count_operator - 1; i++){
        tmp_operator[i] = operator[i + 1];
    }
    operator = realloc(operator ,(count_operator - 1) * sizeof(char));
    for(int i = 0; i < count_operator - 1; i++){
        operator[i] = tmp_operator[i];
    }
    free(tmp_operator);
    char* tmp_argument = calloc(count_operator, sizeof(char)); 
    for(int i = 0; i < num; i++){
        tmp_argument[i] = argument[i];
    }
    tmp_argument[num] = result;
    for(int i = num + 1; i < count_operator; i++){
        tmp_argument[i] = argument[i + 1];
    }
    argument = realloc(argument, count_operator * sizeof(char));
    for(int i = 0; i < count_operator; i++){
        argument[i] = tmp_argument[i];
    }
    free(tmp_argument);
}

void count_arifmetic(char* operator ,int* argument, int num, int count_operator){
    int result;
    switch (operator[num])
    {
    case '*':
        result = mul(argument[num], argument[num+1]);
        shift(operator, argument, num, result, count_operator);
        break;

    case '/':
        result = division(argument[num], argument[num+1]);
        shift(operator, argument, num, result, count_operator);
        break;

    case '-':   
        result = sub(argument[num], argument[num+1]);
        shift(operator, argument, num, result, count_operator);
        break;

    case '+':
        result = sum(argument[num], argument[num+1]);
        shift(operator, argument, num, result, count_operator);
        break;

    default:
        break;
    }
    //printf("%d ", result);
}

void priority(int* priority_operator, int count_operator, char* operator){
    for(int i = 0; i < count_operator; i++){
        if(operator[i] == '*' || operator[i] == '/'){
            priority_operator[i] = 2;
            printf("2 ");
        }
        else{
            priority_operator[i] = 1;
            printf("1 ");
        }
    }
    printf("\n");
}

void decrease_priority(int* priority_operator, int count, int num){
    char* tmp_priority = calloc(count - 1, sizeof(char)); 
    for(int i = 0; i < num; i++){
        tmp_priority[i] = priority_operator[i];
    }
    for(int i = num; i < count; i++){
        tmp_priority[i] = priority_operator[i + 1];
    }
    priority_operator = realloc(priority_operator, sizeof(char) * (count - 1));
    for(int i = 0; i < count - 1; i++){
        priority_operator[i] = tmp_priority[i];
    }
    free(tmp_priority);

}

void arifmetic(char* operator ,int* argument, int count_operator){
    int* priority_operator = calloc(count_operator, sizeof(int));
    int flag_priority_two = 1, flag = 0;;
    priority(priority_operator, count_operator, operator);
    while(count_operator != 0){
        for(int i = 0; i < count_operator; i++){
            if(flag_priority_two == 1){
                if(priority_operator[i] == 2){
                    count_arifmetic(operator, argument, i, count_operator);
                    decrease_priority(priority_operator, count_operator, i);
                    count_operator--;
                    flag = 1;
                    break;
                }
            }
            else{
                count_arifmetic(operator, argument, i, count_operator);
                decrease_priority(priority_operator, count_operator, i);
                count_operator--;
                break;
            }

        }
        if(flag == 0){
            flag_priority_two = 0;
        }else{
            flag = 0;
        }
    }
    printf("%d", argument[0]);

} 

void parsingStr(char* data, int n){
    int count_operator = 0 ,flag = 0, count_argument = 0, count_num = 0, flag_num = 0, flag_error = 0;
    char* operator = calloc(count_operator, sizeof(char));
    int* argument = calloc(count_argument, sizeof(int));
    char num[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    for(int i = 0; i < n; i++){
        if (flag == 1 && (data[i] == '*' || data[i] == '-' ||  data[i] == '+' || data[i] == '/')){
            flag = 0;
            count_operator++;
            operator = realloc(operator, sizeof(char) * count_operator);
            operator[count_operator - 1] = data[i];
        }
        else if(flag == 0){
            for(int j = 0; j < 10; j++){
                if (data[i]  == num[j]){
                    count_num++;
                    flag_num = 1;
                    break;
                }
            }
            if(count_num != 0  && flag_num == 0){
                count_argument++;
                flag = 1;
                argument = realloc(argument, sizeof(int) * count_argument);
                argument[count_argument - 1] = charInInt( i - count_num, i, data );
                i--;
                count_num = 0;
            }
            flag_num = 0;
        }
        else if(data[i] != '='){
            printf("Incorrect input");
            flag_error = 1;
            break;
        }
    }
    if (flag_error == 0){
        for(int i = 0; i < count_argument; i++){
            printf("%d ", argument[i]);
        }
        printf("\n");
        for(int i = 0; i < count_operator; i++){
            printf("%c ", operator[i]);
        }
        printf("\n");
        arifmetic(operator, argument, count_operator);
    }
}

int main(){
    int n = 0;
    char* calc_string = calloc(n, sizeof(char) );
    char simbol;
    while(scanf("%c", &simbol) == 1 && simbol != '\n'){
        calc_string[n] = simbol;
        n++;
        calc_string = realloc(calc_string, sizeof(char) * n);
    }
    if(calc_string[n -1] != '='){
        printf("there is no =\n");
        return 0;
    }
    for(int i = 0; i < n; i++){
        printf("%c", calc_string[i]);
    }
    printf("\n");
    parsingStr(calc_string, n);
}