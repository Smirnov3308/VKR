CC=g++

CFLAGS=-O3 -m64 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -Wall -c
OFLAGS=-pthread -O3 -m64 -DNDEBUG -DINTEL64 -D_REENTRANT -lrt -o

all: concurrent_rand

concurrent_rand: concurrent_rand.obj hopscotch.obj
	$(CC) concurrent_rand.o hopscotch.o $(OFLAGS) concurrent_rand

concurrent_rand.obj: concurrent_rand.cpp hopscotch.hpp
	$(CC) $(CFLAGS) concurrent_rand.cpp hopscotch.hpp

hopscotch.obj: hopscotch.cpp hopscotch.hpp
	$(CC) $(CFLAGS) hopscotch.cpp hopscotch.hpp

clean:
	rm -rf *.o *.gch concurrent_rand
