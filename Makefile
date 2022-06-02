OBJS = main.o config.o connection.o queue.o list.o
CC = g++
FLAGS = -g -Wall -Wno-literal-suffix -c

all: bbserv

bbserv: $(OBJS)
	$(CC) -g $(OBJS) -o bbserv

main.o: main.cpp common.h config.h connection.h queue.h list.h
	$(CC) $(FLAGS) main.cpp

config.o: config.cpp common.h config.h
	$(CC) $(FLAGS) config.cpp

connection.o: connection.cpp common.h config.h connection.h queue.h list.h
	$(CC) $(FLAGS) connection.cpp

queue.o: queue.cpp queue.h
	$(CC) $(FLAGS) queue.cpp

list.o: list.cpp list.h
	$(CC) $(FLAGS) list.cpp


clean:
	rm -f $(OBJS) bbserv
