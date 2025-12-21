#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char* inputStr(){
    char c = ' ';
    int n = 1;
    char* data = calloc(n, sizeof(char));
    while(scanf("%c", &c) != 1 || c!='\n'){
        data[n-1]= c;
        n++;
        data = realloc(data, n);
    }
    data[n-1]='\0';
    return data;
}

char** parseArguments(char* data, int* argCount){
    int count = 0;
    int capacity = 10;
    char** args = malloc(capacity * sizeof(char*));
    
    char* token = strtok(data, " ");
    while(token != NULL){
        if(count >= capacity){
            capacity *= 2;
            args = realloc(args, capacity * sizeof(char*));
        }
        args[count] = malloc(strlen(token) + 1);
        strcpy(args[count], token);
        count++;
        token = strtok(NULL, " ");
    }
    
    if(count >= capacity){
        args = realloc(args, (capacity + 1) * sizeof(char*));
    }
    args[count] = NULL;
    
    *argCount = count;
    return args;
}

void comandInterpretator(){
    while(1){
        printf("input command\n");
        char* data = inputStr();
        
        if(strlen(data) == 0){
            free(data);
            continue;
        }
        
        int argCount;
        char** args = parseArguments(data, &argCount);
        
        if(argCount > 0){
            pid_t pid = fork();
            
            if(pid == 0){
                execvp(args[0], args);
                perror("execvp failed");
                exit(1);
            }
            else if(pid > 0){
                wait(NULL);
            }
            else{
                perror("fork failed");
            }
        }
    
        for(int i = 0; i < argCount; i++){
            free(args[i]);
        }
        free(args);
        free(data);
    }
}

int main(){
    comandInterpretator();
    return 0;
}