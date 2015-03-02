CC = g++
CFLAGS = -c --std=c++11
LDFLAGS = -lcurses -lcurl -ljsoncpp -ltag

SOURCES=cplayer.cpp 
EXECUTABLE = cplayer

all: cplayer

cplayer: cplayer.o
	$(CC) cplayer.o -o cplayer $(LDFLAGS)

cplayer.o: cplayer.cpp
	$(CC) $(CFLAGS) cplayer.cpp $(LDFLAGS)

