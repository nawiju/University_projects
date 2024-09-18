CC      = g++
CFLAGS  = -Wall -Wextra -O2 -std=c++20 -g
TARGETS = kierki-klient kierki-serwer

all: $(TARGETS)

kierki-klient: kierki-klient.o err.o common.o parsing.o client_library.o
kierki-serwer: kierki-serwer.o err.o common.o parsing.o server_library.o

err.o: err.cc err.h
common.o: common.cc err.h common.h 
parsing.o: parsing.cc parsing.h 

client_library.o: err.h common.h parsing.h client_library.cc client_library.h

server_library.o: err.h common.h parsing.h server_library.cc server_library.h

kierki-klient.o: kierki-klient.cc err.h common.h parsing.h client_library.h

kierki-serwer.o: kierki-serwer.cc err.h common.h parsing.h server_library.h

clean:
	rm -f $(TARGETS) *.o
