CC     = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu17 -g -lsocket
TARGETS = ppcbc ppcbs

all: $(TARGETS)

ppcbc: ppcbc.o err.o common.o clients.o

ppcbs: ppcbs.o err.o common.o servers.o

err.o: err.c err.h
common.o: common.c err.h common.h 
ppcbc.o: ppcbc.c err.h common.h protconst.h
ppcbs.o: ppcbs.c err.h common.h protconst.h
servers.o: servers.c err.h common.h protconst.h
clients.o: clients.c err.h common.h 

clean:
	rm -f $(TARGETS) *.o
