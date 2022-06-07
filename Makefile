OBJS = main.o config.o connection.o queue.o list.o msg.o
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

msg.o:
	$(CC) $(FLAGS) msg.cpp



clean:
	rm -f $(OBJS) bbserv
