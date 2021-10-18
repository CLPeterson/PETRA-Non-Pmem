CC=g++
CFLAGS=-I. -ggdb
DEPS = 
OBJ = script.o

all: script

scriptPetra.o: script.cc $(DEPS)
	$(CC) -c script.cc -o script.o $(CFLAGS)
	
clean:
	rm -f script $(OBJ)

