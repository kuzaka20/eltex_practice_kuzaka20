#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

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

typedef struct {
    char** commands;
    int command_count;
    char* input_file;
    char* output_file;
    int append_output;
} Pipeline;

Pipeline* parsePipeline(char* data, int* pipeline_count){
    int count = 0;
    int capacity = 5;
    Pipeline* pipelines = malloc(capacity * sizeof(Pipeline));
    
    for(int i = 0; i < capacity; i++){
        pipelines[i].commands = NULL;
        pipelines[i].command_count = 0;
        pipelines[i].input_file = NULL;
        pipelines[i].output_file = NULL;
        pipelines[i].append_output = 0;
    }
    
    char* token = strtok(data, "|");
    while(token != NULL){
        if(count >= capacity){
            capacity *= 2;
            pipelines = realloc(pipelines, capacity * sizeof(Pipeline));
            for(int i = count; i < capacity; i++){
                pipelines[i].commands = NULL;
                pipelines[i].command_count = 0;
                pipelines[i].input_file = NULL;
                pipelines[i].output_file = NULL;
                pipelines[i].append_output = 0;
            }
        }
        
        int arg_capacity = 10;
        pipelines[count].commands = malloc(arg_capacity * sizeof(char*));
        int arg_count = 0;
        
        char* cmd_token = strtok(token, " ");
        while(cmd_token != NULL){
            if(strcmp(cmd_token, "<") == 0){
                cmd_token = strtok(NULL, " ");
                if(cmd_token != NULL){
                    pipelines[count].input_file = strdup(cmd_token);
                }
            }
            else if(strcmp(cmd_token, ">>") == 0){
                cmd_token = strtok(NULL, " ");
                if(cmd_token != NULL){
                    pipelines[count].output_file = strdup(cmd_token);
                    pipelines[count].append_output = 1;
                }
            }
            else if(strcmp(cmd_token, ">") == 0){
                cmd_token = strtok(NULL, " ");
                if(cmd_token != NULL){
                    pipelines[count].output_file = strdup(cmd_token);
                    pipelines[count].append_output = 0;
                }
            }
            else{
                if(arg_count >= arg_capacity){
                    arg_capacity *= 2;
                    pipelines[count].commands = realloc(pipelines[count].commands, arg_capacity * sizeof(char*));
                }
                pipelines[count].commands[arg_count] = strdup(cmd_token);
                arg_count++;
            }
            cmd_token = strtok(NULL, " ");
        }
        
        if(arg_count >= arg_capacity){
            pipelines[count].commands = realloc(pipelines[count].commands, (arg_capacity + 1) * sizeof(char*));
        }
        pipelines[count].commands[arg_count] = NULL;
        pipelines[count].command_count = arg_count;
        
        count++;
        token = strtok(NULL, "|");
    }
    
    *pipeline_count = count;
    return pipelines;
}

void freePipeline(Pipeline* pipelines, int count){
    for(int i = 0; i < count; i++){
        for(int j = 0; j < pipelines[i].command_count; j++){
            free(pipelines[i].commands[j]);
        }
        free(pipelines[i].commands);
        if(pipelines[i].input_file) free(pipelines[i].input_file);
        if(pipelines[i].output_file) free(pipelines[i].output_file);
    }
    free(pipelines);
}

void executePipeline(Pipeline* pipelines, int pipeline_count){
    int pipefds[2 * (pipeline_count - 1)];
    pid_t* pids = malloc(pipeline_count * sizeof(pid_t));
    
    for(int i = 0; i < pipeline_count - 1; i++){
        if(pipe(pipefds + i * 2) < 0){
            perror("pipe failed");
            free(pids);
            return;
        }
    }
    
    for(int i = 0; i < pipeline_count; i++){
        pids[i] = fork();
        
        if(pids[i] == 0){
            if(i == 0 && pipelines[i].input_file){
                int fd = open(pipelines[i].input_file, O_RDONLY);
                if(fd < 0){
                    perror("open input file failed");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            if(i == pipeline_count - 1 && pipelines[i].output_file){
                int flags = O_WRONLY | O_CREAT;
                if(pipelines[i].append_output){
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                int fd = open(pipelines[i].output_file, flags, 0644);
                if(fd < 0){
                    perror("open output file failed");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            if(i > 0){
                dup2(pipefds[(i-1)*2], STDIN_FILENO);
            }
            
            if(i < pipeline_count - 1){
                dup2(pipefds[i*2 + 1], STDOUT_FILENO);
            }
            
            for(int j = 0; j < 2 * (pipeline_count - 1); j++){
                close(pipefds[j]);
            }
            
            execvp(pipelines[i].commands[0], pipelines[i].commands);
            perror("execvp failed");
            exit(1);
        }
        else if(pids[i] < 0){
            perror("fork failed");
        }
    }
    
    for(int i = 0; i < 2 * (pipeline_count - 1); i++){
        close(pipefds[i]);
    }
    
    for(int i = 0; i < pipeline_count; i++){
        waitpid(pids[i], NULL, 0);
    }
    
    free(pids);
}

void comandInterpretator(){
    while(1){
        printf("input command\n");
        char* data = inputStr();
        
        if(strlen(data) == 0){
            free(data);
            continue;
        }
        
        int pipeline_count;
        Pipeline* pipelines = parsePipeline(data, &pipeline_count);
        
        if(pipeline_count > 0){
            executePipeline(pipelines, pipeline_count);
        }
        
        freePipeline(pipelines, pipeline_count);
        free(data);
    }
}

int main(){
    comandInterpretator();
    return 0;
}