OBJS	= communication_protocol.o countlist.o crawler_thread.o functions_crawler.o functions_worker.o main.o queue.o searchlist.o structs.o templist.o
SOURCE	= communication_protocol.c countlist.c crawler_thread.c functions_crawler.c functions_worker.c main.c queue.c searchlist.c structs.c templist.c
HEADER	= communication_protocol.h countlist.h crawler_thread.h functions_crawler.h functions_worker.h queue.h searchlist.h structs.h templist.h
OUT	= mycrawler
CC	 = gcc
FLAGS	 = -g -c -Wall

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) -lpthread

communication_protocol.o: communication_protocol.c
	$(CC) $(FLAGS) communication_protocol.c -lpthread

countlist.o: countlist.c
	$(CC) $(FLAGS) countlist.c -lpthread

crawler_thread.o: crawler_thread.c
	$(CC) $(FLAGS) crawler_thread.c -lpthread

functions_crawler.o: functions_crawler.c
	$(CC) $(FLAGS) functions_crawler.c -lpthread

functions_worker.o: functions_worker.c
	$(CC) $(FLAGS) functions_worker.c -lpthread

main.o: main.c
	$(CC) $(FLAGS) main.c -lpthread

queue.o: queue.c
	$(CC) $(FLAGS) queue.c -lpthread

searchlist.o: searchlist.c
	$(CC) $(FLAGS) searchlist.c -lpthread

structs.o: structs.c
	$(CC) $(FLAGS) structs.c -lpthread

templist.o: templist.c
	$(CC) $(FLAGS) templist.c -lpthread


clean:
	rm -f $(OBJS) $(OUT)

