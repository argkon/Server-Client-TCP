OBJS	= communication_protocol.o countlist.o functions_httpd.o functions_worker.o httpd_thread.o main.o queue.o searchlist.o structs.o templist.o
SOURCE	= communication_protocol.c countlist.c functions_httpd.c functions_worker.c httpd_thread.c main.c queue.c searchlist.c structs.c templist.c
HEADER	= communication_protocol.h countlist.h functions_httpd.h functions_worker.h httpd_thread.h queue.h searchlist.h structs.h templist.h
OUT	= myhttpd
CC	 = gcc
FLAGS	 = -g -c -Wall

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) -lpthread

communication_protocol.o: communication_protocol.c
	$(CC) $(FLAGS) communication_protocol.c -lpthread

countlist.o: countlist.c
	$(CC) $(FLAGS) countlist.c -lpthread

functions_httpd.o: functions_httpd.c
	$(CC) $(FLAGS) functions_httpd.c -lpthread

functions_worker.o: functions_worker.c
	$(CC) $(FLAGS) functions_worker.c -lpthread

httpd_thread.o: httpd_thread.c
	$(CC) $(FLAGS) httpd_thread.c -lpthread

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

