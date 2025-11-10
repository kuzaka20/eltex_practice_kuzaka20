#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <time.h>


int main(){
    srand(time(NULL));
    Queue* queue = initQueue();
    int priority_pop[10];
    for(int i = 0; i < 10; i++){
        int data = rand();
        int priority = rand() % 256;
        priority_pop[i] = priority;
        pushQueue(queue, data, priority);
    }
    queuePrint(queue);
    printf("%d\n", pop(queue));
    queuePrint(queue);
    int priority_del = rand()%10;
    printf("-%d-\n",priority_pop[priority_del]);
    int error = popPriority(queue, priority_pop[priority_del]);
    if(error == 1){
        printf("Queue is empty\n");
    }
    else if(error == 2){
        printf("No data with priority %d\n", priority_del);
    }
    else{
        printf("Ok\n");
    }
    queuePrint(queue);
    priority_del = rand()%10;
    printf("-%d-\n", priority_pop[priority_del]);
    popLowPriority(queue, priority_pop[priority_del]);
    queuePrint(queue);
    freeQueue(queue);
    free(queue);

    return 0;
}