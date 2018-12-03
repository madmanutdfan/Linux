CC=g++
CFLAGS=-Werror -Wall -g
LDFLAGS= -lrt -pthread -g
TARG_FILES = $(shell find . -name "conn_*.cpp") 
ALL_TARGS = $(TARG_FILES:./conn%.cpp=lab3%)

all: $(ALL_TARGS)

lab3_%: conn_%.o host.o goatlings.o semaphore.o
	$(CC) $< host.o goatlings.o semaphore.o -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *.o lab3*
