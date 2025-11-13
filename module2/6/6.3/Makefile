CC = gcc
CFLAGS = -Wall -Werror -Wextra -c

all: calculator.exe

calculator.exe: calculator.o
	$(CC) calculator.o -ldl -o calculator.exe

calculator.o: calculator.c
	$(CC) $(CFLAGS) calculator.c -o calculator.o

libs/libadd.so: libs/libadd.c
	$(CC) -shared -fPIC libs/libadd.c -o libs/libadd.so

libs/libsubtract.so: libs/libsubtract.c
	$(CC) -shared -fPIC libs/libsubtract.c -o libs/libsubtract.so

libs/libmultiply.so: libs/libmultiply.c
	$(CC) -shared -fPIC libs/libmultiply.c -o libs/libmultiply.so

libs/libdivide.so: libs/libdivide.c
	$(CC) -shared -fPIC libs/libdivide.c -o libs/libdivide.so

libs: libs/libadd.so libs/libsubtract.so libs/libmultiply.so libs/libdivide.so

full: libs calculator.exe

clean:
	del /Q *.o *.exe 2>nul
	del /Q libs\*.so 2>nul

cleanlibs:
	del /Q libs\*.so 2>nul

cleanexe:
	del /Q *.o *.exe 2>nul