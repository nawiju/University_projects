CC	= gcc
CPPFLAGS = 
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

.PHONY: all clean

all: example_dynamic  

seq.o: seq.c seq.h
memory_tests.o: memory_tests.c memory_tests.h 
seq_example.o: seq_example.c seq.h memory_tests.h 

libseq.so: seq.o memory_tests.o
	gcc $(LDFLAGS) -o $@ $^

example_dynamic: seq_example.o libseq.so 
	gcc -L. -o $@ $< -lseq

clean:
	rm -rf example_dynamic *.s *.so *.o *.a

