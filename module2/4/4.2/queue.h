
typedef struct Node Node;

typedef struct Node
{
    int data;
    int priority;
    Node* next;
}Node;

typedef struct Queue
{
    Node* head;
    Node* tail;
}Queue;


Queue* initQueue();
Node* initNode( int value, int priority);
void pushQueue(Queue* queue, int data, int priority);
int pop(Queue* queue);
void queuePrint(Queue* queue);
void freeQueue(Queue* queue);
int popPriority(Queue* queue, int priority);
void popLowPriority(Queue* queue, int priority);