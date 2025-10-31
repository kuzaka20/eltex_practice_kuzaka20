#include "phone_contact.h"
struct node;

typedef struct 
{
    contact data;
    node* next;
    node* pred;
}node;

typedef struct
{
    node* head;
}list;
