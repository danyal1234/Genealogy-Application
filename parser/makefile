CC=gcc
CFLAGS= -std=c11 -fPIC

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := sharedLib.so
endif

$(LIB): sharedLib.o
	$(CC) $(CFLAGS) -Iinclude -c src/LinkedListAPI.c
	$(CC) -shared -o ../$(LIB)  GEDCOMparser.o LinkedListAPI.o

sharedLib.o: src/GEDCOMparser.c
	$(CC) $(CFLAGS) -Iinclude -c src/GEDCOMparser.c

clean:
	rm $(LIB) *.o
