obj-m += kernel_space.o
 
all: user.c
	gcc user.c -o user
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -f user
