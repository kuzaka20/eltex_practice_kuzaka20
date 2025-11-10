CC = gcc
CFLAGS = -Wall -Werror -Wextra
DIR_CONTACT = 2/2.1
DIR_CALC = 2/2.2
DIR_CALC2 = 2/2.3
DIR_CHMOD = 3/3.1
DIR_IP = 3/3.2
DIR_LIST = 4/4.1
DIR_QUEUE = 4/4.2
DIR_TREE = 4/4.3

all: contacts calc calc2 chmod ip list queue tree 

contacts:
	$(CC) $(CFLAGS) $(DIR_CONTACT)/main.c $(DIR_CONTACT)/phone_contact.c -o contacts.exe

calc:
	$(CC) $(CFLAGS) $(DIR_CALC)/calc.c -o calc.exe

calc2:
	$(CC) $(CFLAGS) $(DIR_CALC2)/calc2.c -o calc2.exe

chmod:
	$(CC) $(CFLAGS) $(DIR_CHMOD)/chmod.c -o chmod.exe

ip:
	$(CC) $(CFLAGS) $(DIR_IP)/ip.c -o ip.exe

list:
	$(CC) $(CFLAGS) $(DIR_LIST)/list.c $(DIR_LIST)/main.c -o list.exe

queue:
	$(CC) $(CFLAGS) $(DIR_QUEUE)/queue.c $(DIR_QUEUE)/main_queue.c -o queue.exe

tree:
	$(CC) $(CFLAGS) $(DIR_TREE)/tree.c $(DIR_TREE)/main_tree.c -o tree.exe


clean:
	del *.exe
