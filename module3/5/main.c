#include <stdio.h> 
#include <signal.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

volatile int sigint_count = 0;
volatile int should_exit = 0;

void signalHandler(int sig) { 
    FILE* file = fopen("time.data", "a");
    if(file == NULL){
        printf("Error: cannot create or open file\n");
        return;
    }
    
    switch(sig) {
        case SIGINT:
            sigint_count++;
            fprintf(file, "Получен и обработан сигнал SIGINT (%d/3)\n", sigint_count);
            break;
        case SIGQUIT:
            fprintf(file, "Получен и обработан сигнал SIGQUIT\n");
            break;
        case SIGTERM:
            fprintf(file, "Получен и обработан сигнал SIGTERM\n");
            should_exit = 1;
            break;
        case SIGABRT:
            fprintf(file, "Получен и обработан сигнал SIGABRT\n");
            break;
        case SIGTSTP:
            fprintf(file, "Получен и обработан сигнал SIGTSTP\n");
            raise(SIGSTOP);
            break;
        case SIGCONT:
            fprintf(file, "Получен и обработан сигнал SIGCONT\n");
            break;
        default:
            fprintf(file, "Получен неизвестный сигнал: %d\n", sig);
    }
    
    fclose(file);
} 

int main(){
    int count = 1;
    
    signal(SIGINT, signalHandler); 
    signal(SIGQUIT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGCONT, signalHandler);
    
    remove("time.data");
    
    printf("Программа запущена. PID: %d\n", getpid());
    
    while(1){
        if (sigint_count >= 3 || should_exit) {
            FILE* file = fopen("time.data", "a");
            if(file != NULL){
                if (sigint_count >= 3) {
                    fprintf(file, "Программа завершена после третьего сигнала SIGINT\n");
                } else {
                    fprintf(file, "Программа завершена по флагу should_exit\n");
                }
                fclose(file);
            }
            printf("Программа завершена\n");
            break;
        }
        
        FILE* file = fopen("time.data", "a");
        if(file == NULL){
            printf("Error: cannot create or open file\n");
            return 1;
        }
        
        fprintf(file, "%d\n", count);
        fclose(file);
        
        sleep(1);
        count++;
    }
    
    return 0;
}