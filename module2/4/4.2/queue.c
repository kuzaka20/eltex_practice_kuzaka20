#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

Queue* initQueue(){
    Queue* queue = malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

Node* initNode( int value, int priority){
    Node* node = malloc(sizeof(Node));
    node->data = value;
    node->priority = priority;
    node->next = NULL;
    return node;
}

void pushQueue(Queue* queue, int data, int priority){
    Node* node = initNode(data, priority);
    if(queue->head == NULL){
        queue->head = node;
        queue->tail = node;
        return;
    }
    if(queue->head->priority > priority){
        Node* tmp = queue->head;
        node->next = tmp;
        queue->head = node;
        return;
    }
    Node* tmp = NULL;
    for(tmp = queue->head; tmp->next != NULL; tmp = tmp->next){
        if(tmp->next->priority > priority){
            node->next = tmp->next;
            tmp->next = node;
            if(node->next == NULL){
                queue->tail = node;
            }
            return;
        }
    }
    tmp->next = node;
    queue->tail = node;
    return;
}

int pop(Queue* queue){
    if(queue->head == NULL){
        return 1;
    }
    int data = queue->head->data;
    Node* tmp = queue->head;
    if(tmp->next == NULL){
        queue->head = NULL;
        queue->tail = NULL;
    }
    else{
        queue->head = tmp->next;
    }
    free(tmp);
    return data;
}

void queuePrint(Queue* queue){

    Node* tmp = queue->head;
    if(tmp == NULL) {
        printf("Queue is empty\n");
    }
    else {
        printf("value | priority\n");
        while (tmp != NULL) {

            printf("%d | %d\n", tmp->data, tmp->priority);
            tmp = tmp->next;
        
        }
    }

}

void freeQueue(Queue* queue){
    Node* node = queue->head;
    Node* tmp = NULL;
    while(node != NULL){
        tmp = node->next;
        free(node);
        node = tmp;
    }
    free(tmp);
}

int popPriority(Queue* queue, int priority,int* result){
    if(queue->head == NULL){
        return 0;
    }
    
    Node* tmp = queue->head;
    
    if(tmp->priority == priority){
        *result = tmp->data;
        queue->head = tmp->next;
        if(queue->head == NULL){
            queue->tail = NULL;
        }
        free(tmp);
        return 1;
    }
    
    while(tmp->next != NULL){
        if(tmp->next->priority == priority){
            *result = tmp->next->data;
            Node* toDelete = tmp->next;
            tmp->next = tmp->next->next;
            if(tmp->next == NULL){
                queue->tail = tmp;
            }
            free(toDelete);
            return 1;
        }
        tmp = tmp->next;
    }
    
    return 0;
}

int popLowPriority(Queue* queue, int priority, int* result){
    if(queue->head == NULL){
        return 0;
    }
    
    Node* tmp = queue->head;
    
    if(tmp->priority > priority){
        *result = tmp->data;
        queue->head = tmp->next;
        if(queue->head == NULL){
            queue->tail = NULL;
        }
        free(tmp);
        return 1;
    }
    
    while(tmp->next != NULL){
        if(tmp->next->priority > priority){
            *result = tmp->next->data;
            Node* toDelete = tmp->next;
            tmp->next = tmp->next->next;
            if(tmp->next == NULL){
                queue->tail = tmp;
            }
            free(toDelete);
            return 1;
        }
        tmp = tmp->next;
    }
    
    return 0;
}
