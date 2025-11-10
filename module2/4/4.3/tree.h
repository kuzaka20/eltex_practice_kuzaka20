#define LEN_NAME 30
#define LEN_PHONE_NUM 11
#define LEN_COUNT_PHONE 10
#define LEN_CONTACT 10


typedef struct
{
    int id;
    char surname[LEN_NAME];
    char  name[LEN_NAME];
    char patronymic[LEN_NAME];
    char phone_num[LEN_COUNT_PHONE][LEN_PHONE_NUM];
    int count_num;
    char work[LEN_NAME];
    char position[LEN_NAME];
}contact;

typedef struct node node;

typedef struct node
{
    contact* data;
    node* left;
    node* right;

}node;

typedef struct
{
    node* root;

}tree;



short int pushTree(int id, node** data);
void printTree(node* data);
tree* initTree();
node* initNode();
int replaceContactCheckID(int id, node** data);
int delContact(int num, node** data);
void balanceTree(tree* contacts);
void printRoot(tree* contact);