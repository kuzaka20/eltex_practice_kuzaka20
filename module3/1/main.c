#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void strToNum(char* str);
void parsing(int begin, int end, char* str[]);
void forkexample(int count, char* str[]);

int main(int argc, char* argv[]){
    if(argc == 1){ 
        printf("error\n");
        return 1;
    }
    forkexample(argc, argv);
}

void strToNum(char* str){
    int result = atoi(str);
    if (result != 0){
        printf("%d, %d\n", result, result * 2);
    }
    else{
        for(size_t i = 0; i <strlen(str); i++){
            printf("%c", str[i]);
        }
        printf("\n");
    }
}

void parsing(int begin, int end, char* str[]){
    for(int i = begin; i < end; i++){
        strToNum(str[i]);
    }
}

void forkexample(int count, char* str[]){
    pid_t p = fork();
    if(p<0)
    {
      perror("fork fail");
      exit(1);
    }
    else if(p == 0){
        printf("%d\n", p);
        parsing(1 , count/2, str);
    }
    else{
        printf("%d\n", p);
        parsing(count/2 ,count, str);
    }
}