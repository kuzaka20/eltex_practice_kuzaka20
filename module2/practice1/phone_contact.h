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

void printContact(contact* data);
int addContact(int id, contact** contacts, int count);
short int delContact(int id, contact** contacts , int count);
short int replaceContactCheckID(int id, contact** contacts, int count);
