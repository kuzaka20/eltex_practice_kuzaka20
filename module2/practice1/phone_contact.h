typedef struct
{
    int id;
    char* surname;
    char*  name;
    char*  patronymic;
    char**  phone_num;
    int count_num;
    char*  work;
    char*  position;
}contact;

void addContactName(contact* data, char* value);
void addContactSurname(contact* data, char* value);
void addContactPatronymic(contact* data, char* value);
void addContactWork(contact* data, char* value);
void addContactPosition(contact* data, char* value);
void printContact(contact* data);
void addContactId(contact* data, int value);
void addContactNum(contact* data, char** value ,int count);
int addContact(int id, contact** contacts);
int delContact(int id, contact** contacts);
