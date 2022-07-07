#Makefile
#Created by Dianyong Chen, 2022-05-24
#CS590 Master Project(BBServer) @ Bishop's University


OBJS = bootup.o config.o event.o connection.o queue.o list.o msg.o semaphore.o sync.o
CC = g++
FLAGS = -g -Wall -Wno-literal-suffix -c

all: bbserv

bbserv: $(OBJS)
	$(CC) -g $(OBJS) -o bbserv

bootup.o: bootup.cpp
	$(CC) $(FLAGS) bootup.cpp

config.o: config.cpp
	$(CC) $(FLAGS) config.cpp

event.o: event.cpp
	$(CC) $(FLAGS) event.cpp

msg.o: msg.cpp
	$(CC) $(FLAGS) msg.cpp

sync.o: sync.cpp
	$(CC) $(FLAGS) sync.cpp

connection.o: connection.cpp
	$(CC) $(FLAGS) connection.cpp

queue.o: queue.cpp
	$(CC) $(FLAGS) queue.cpp

list.o: list.cpp
	$(CC) $(FLAGS) list.cpp

semaphore.o: semaphore.cpp
	$(CC) $(FLAGS) semaphore.cpp


clean:
	rm -f $(OBJS) bbserv bbserv.log bbserv.pid
