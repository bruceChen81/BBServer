OBJS = main.o config.o connection.o queue.o list.o msg.o semaphore.o sync.o
CC = g++
FLAGS = -g -Wall -Wno-literal-suffix -c

all: bbserv

bbserv: $(OBJS)
	$(CC) -g $(OBJS) -o bbserv

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp

config.o: config.cpp
	$(CC) $(FLAGS) config.cpp

connection.o: connection.cpp
	$(CC) $(FLAGS) connection.cpp

queue.o: queue.cpp
	$(CC) $(FLAGS) queue.cpp

list.o: list.cpp
	$(CC) $(FLAGS) list.cpp

msg.o: msg.cpp
	$(CC) $(FLAGS) msg.cpp

semaphore.o: semaphore.cpp
	$(CC) $(FLAGS) semaphore.cpp

sync.o: sync.cpp
	$(CC) $(FLAGS) sync.cpp



clean:
	rm -f $(OBJS) bbserv core
