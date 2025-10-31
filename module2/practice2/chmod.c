#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

char parsing(char* str, int len_str , char* users, char* actions, char* name_file){
    int user_count = 0 , action_count = 0, count=0 ,flag = 0;
    int check[6] = {0,0,0,0,0,0};
    char operator = '1';
    while(str[count] == 'u'  || str[count] == 'g' || str[count] == 'o'){
        if(str[count] == 'u'){
            flag = 0;
        }else if(str[count] == 'g'){
            flag = 1;
        }else{
            flag = 2;
        }
        if(check[flag] == 0){
            user_count++;
            users = realloc(users, sizeof(char) * user_count); 
            users[user_count - 1] = str[count];
            count++;
            check[flag] = 1;
        }
        else{
            operator = '0';
            break;
        }
    }
    if((str[count] == '+'  || str[count] == '-' || str[count] == '=') && operator != '0'){
        operator = str[count];
        count++;
        while(str[count] == 'r'  || str[count] == 'w' || str[count] == 'x'){
            if(str[count] == 'r'){
                flag = 3;
            }else if(str[count] == 'w'){
                flag = 4;
            }else{
                flag = 5;
            }
            if(check[flag] == 0){
                action_count++;
                actions = realloc(actions, sizeof(char) * action_count); 
                actions[action_count - 1] = str[count];
                count++;
                check[flag] = 1;
            }
        }
    }
    else{
        operator = '0';
    }
    if(str[count] == ' '){
        for(int i = count + 1; i< len_str; i++){
            name_file = realloc(name_file, sizeof(char) *  (i - count));
            name_file[i - count - 1] = str[i];
        }
    }
    return operator;
}
int  input(char* str){
    int len_str = 0;
    char simbol;
    while(scanf("%c", &simbol) != 0 && simbol != '\n'){
        len_str++;
        str = realloc(str, sizeof(char) * len_str);
        str[len_str - 1] = simbol;
    }
    return len_str;
}
void printParsStr(char* users, char* actions, char* name_file, char operator){
    printf("users - ");
    for(size_t i = 0; i < strlen(users); i++){
        printf("%c", users[i]);
    }
    printf("\n");
    printf("operator - %c", operator);
    printf("\n");
    printf("actions - ");
    for(size_t i = 0; i < strlen(actions); i++){
        printf("%c", actions[i]);
    }
    printf("\n");
    printf("file - ");
    for(size_t i = 0; i < strlen(name_file); i++){
        printf("%c", name_file[i]);
    }
    printf("\n");
}
void printMode(mode_t st_mode){
    if(S_IRUSR & st_mode){printf("r");}else{printf("-");}
    if(S_IWUSR & st_mode){printf("w");}else{printf("-");}
    if(S_IXUSR & st_mode){printf("x");}else{printf("-");}
    if(S_IRGRP & st_mode){printf("r");}else{printf("-");}
    if(S_IWGRP & st_mode){printf("w");}else{printf("-");}
    if(S_IXGRP & st_mode){printf("x");}else{printf("-");}
    if(S_IROTH & st_mode){printf("r");}else{printf("-");}
    if(S_IWOTH & st_mode){printf("w");}else{printf("-");}
    if(S_IXOTH & st_mode){printf("x");}else{printf("-");}
    printf("\n");
}
mode_t printFileMod(char* filename){
    struct _stat buf;
    int result;
    result = _stat( filename, &buf );
    if( result != 0 )
    {
        perror( "Problem getting information" );
        switch (errno)
        {
            case ENOENT:
                printf("File %s not found.\n", filename);
                break;
            case EINVAL:
                printf("Invalid parameter to _stat.\n");
                break;
            default:
                printf("Unexpected error in _stat.\n");
        }
    }
    else
    {
        printMode(buf.st_mode);
    }
    return buf.st_mode;
}
mode_t maskCalc(char* users, char* actions){
    mode_t mask_user, mask_action;
    for(size_t i = 0; i < strlen(users); i++){
        if(users[i] == 'u'){
            mask_user = mask_user | S_IRUSR | S_IWUSR | S_IXUSR;
        }
        else if(users[i] == 'g'){
            mask_user = mask_user | S_IRGRP | S_IWGRP | S_IXGRP;
        }
        else{
            mask_user = mask_user | S_IROTH | S_IWOTH | S_IXOTH;
        }
    }
    for(size_t i = 0; i < strlen(actions); i++){
        if(actions[i] == 'r'){
            mask_action = mask_action | S_IRUSR | S_IRGRP | S_IROTH;
        }
        else if(actions[i] == 'w'){
            mask_action = mask_action | S_IWUSR | S_IWGRP | S_IWOTH;
        }
        else{
            mask_action = mask_action | S_IXUSR | S_IXGRP | S_IXOTH;
        }
    }
    return mask_action & mask_user;
}
mode_t mod(mode_t mask, mode_t mask_file, char operator){
    mode_t mask_operation;
    if(operator == '+'){
        mask_operation = mask | mask_file;
    }else if(operator == '-'){
        mask_operation = mask_file & ~ mask;
    }
    else{
        mask_operation = mask; 
    }
    return mask_operation;
}
int main(){
    int len_str = 0;
    char* str = calloc(len_str + 1, sizeof(char));
    char* users = calloc(len_str + 1 , sizeof(char));
    char* actions = calloc(len_str + 1, sizeof(char));
    char* name_file = calloc(len_str + 1, sizeof(char));
    len_str = input(str);
    char operator = parsing(str, len_str, users, actions, name_file);
    if(operator == '=' || operator == '+' || operator == '-'){
        printParsStr(users, actions, name_file, operator);
        mode_t mask_file = printFileMod(name_file);
        mode_t mask = maskCalc(users, actions);
        printMode(mod(mask, mask_file, operator));
    }
    else{
        printf("incorected input str\n");
    }
    return 0;
}