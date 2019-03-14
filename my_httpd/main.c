#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>      /* sockets */
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <netdb.h>          /* gethostbyaddr */
#include <unistd.h>          /* fork */  
#include <ctype.h>          /* toupper */
#include <signal.h>          /* signal */
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <time.h>

#include "communication_protocol.h"
#include <pthread.h>
#include "httpd_thread.h"
#include "queue.h"
#include "functions_httpd.h"

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    struct timeval start, current;
    gettimeofday(&start, NULL);
    int i, err;
    int servingPort, commandPort, numThreads;
    char * root_dir;

    int * descriptors = 0;

    pthread_t *tids;

    QueueInfoPtr qinfo;
    StatisticsPtr stats = STATISTICS_create();

    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof (client);
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct sockaddr *clientptr = (struct sockaddr *) &client;

    if (argc == 9) {
        if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-c") == 0 && strcmp(argv[5], "-t") == 0 && strcmp(argv[7], "-d") == 0) { // orismata
            servingPort = atoi(argv[2]);
            commandPort = atoi(argv[4]);
            numThreads = atoi(argv[6]);
            root_dir = argv[8];
        } else {
            printf("Wrong Arguments!! \n");
            exit(0);
        }
    } else {
        printf("Wrong Arguments!! \n");
        exit(0);
    }

    // ============================================================

    descriptors = malloc(sizeof (int)*2);
    for (i = 0; i < 2; i++) {
        descriptors[i] = 0;
    }

    printf("service port: %d \ncommand port: %d \nnumber of threads: %d \nroot directory: %s \n", servingPort, commandPort, numThreads, root_dir);

    // -1. FTIA3E FIFO
    qinfo = Queue_create();


    // 0. FTIA3E THREADS

    if ((tids = malloc(numThreads * sizeof (pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    for (i = 0; i < numThreads; i++) {
        ArgsPtr args = Args_create(stats, qinfo ,root_dir);
        args->queue = qinfo;
        args->root_dir = root_dir;
        args->statistics = stats;


        if ((err = pthread_create(tids + i, NULL, worker_main, (void *) args)) != 0) {
            perror("httpd : Failed to pthread_create, exiting");
            return 1;
        }
    }

    int tr = 1;
    // 1. FTIAXE SOCKET
    if ((descriptors[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror_exit("socket");
    }
    
    //www.unix.com/programming/29475-how-solve-error-bind-address-already-use.html
    if (setsockopt(descriptors[0], SOL_SOCKET, SO_REUSEADDR, &tr, sizeof (int)) == -1) {    //sets option name REUSEADDRESS to tr=true=1 value to re-bind socket if already bind
        perror("\n setsockopt \n");
    }

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(servingPort); /* The given port */

    // 2. BIND SOCKET

    if (bind(descriptors[0], serverptr, sizeof (server)) < 0) {
        perror_exit("bind");
    }
    // 3. LISTEN SOCKET

    if (listen(descriptors[0], 128) < 0) {
        perror_exit("listen");
    }

    // ---------------------------------------------------------------
    // 1. FTIAXE SOCKET 
    if ((descriptors[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror_exit("socket");
    }
    
    if (setsockopt(descriptors[1], SOL_SOCKET, SO_REUSEADDR, &tr, sizeof (int)) == -1) {
        perror("\n setsockopt \n");
    }
    
    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(commandPort); /* The given port */

    // 2. BIND SOCKET

    if (bind(descriptors[1], serverptr, sizeof (server)) < 0) {
        perror_exit("bind");
    }
    // 3. LISTEN SOCKET

    if (listen(descriptors[1], 128) < 0) {
        perror_exit("listen");
    }

    // ------------------------------------------------------------

    printf("Listening for connections to serving port %d\n", servingPort);
    printf("Listening for connections to commandPort port %d\n", commandPort);

    for (i = 0; i < 2; i++) {
        printf("File Descriptor[%d] = %d \n", i, descriptors[i]);
    }

    struct pollfd * fds = buildWaitingTableFromSockets(descriptors);

    if (fds == NULL) {
        return -1;
    }

    while (1) {
        printf("===================== httpd: waiting for connection =====================\n");
        int newsock;
        int sock = pollingForSockets(fds); //polling

        // 5. ACCEPT
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0) {
            perror_exit("accept");
        }

        printf("Accepted connection from %d and assigned to fd: %d \n", sock, newsock);

        // E3UPHRETHSE TIN ENTOLI (command vs serving)

        //        char* buffer = NULL;
        char buffer[BUFSIZE];
        memset(buffer,0,BUFSIZE);

        if (sock == descriptors[0]) { // serving
            printf("serving request \n");

            // produce to queue, do NOT READ
            QueueElem elem;
            elem.value = newsock;
            Queue_enqueue(&qinfo, elem, &err);
        }

        if (sock == descriptors[1]) { // command
            printf("command request \n");
            //            break;
            if ((i = readCommandFromSocketC(newsock, buffer, BUFSIZE)) == 0) { //read command chunk by chunk 
                printf("httpd: error in read from socket \n");
                continue;
            } else {
                printf("Bytes Read are:%d\n and String read is :%s \n", i, buffer);
                if (strcmp(buffer, "SHUTDOWN\n") == 0 || strcmp(buffer, "shutdown\n") == 0 || strcmp(buffer, "SHUTDOWN\r\n") == 0 || strcmp(buffer, "shutdown\r\n") == 0) { //Serve Shutdown Command
                    close(newsock);
                    //free(buffer);
                    break;
                } else if (strcmp(buffer, "STATS\n") == 0 || strcmp(buffer, "stats\n") == 0 || strcmp(buffer, "STATS\r\n") == 0 || strcmp(buffer, "stats\r\n") == 0) { //Serve Stats Command
                    gettimeofday(&current, NULL);
                    long int seconds = current.tv_sec - start.tv_sec;
                    long int microseconds = current.tv_usec - start.tv_usec;

                    //    time_t curtime = seconds;
                    //    struct tm *t = localtime(&curtime);
                    //    printf("%02d:%02d.%03d\n",t->tm_min, t->tm_sec, microseconds/1000);

                    pthread_mutex_lock(&stats->mtx);
                    stats->totaltime = microseconds + seconds * 1000000L;
                    STATISTICS_print(stats);
                    STATISTICS_send(stats, newsock);
                    pthread_mutex_unlock(&stats->mtx);

                } else {
                    printf("Wrong Command!! \n");
                }
            }
            close(newsock); /* parent closes command socket fd */
        }
        //free(buffer);
    }

    free(fds);

    QueueElem qe;
    qe.value = -1;

    for (i = 0; i < numThreads; i++) {
        Queue_enqueue(&qinfo, qe, &err);
        if (err == 1) {
            perror_exit("Error in enqueue!!\n");
        }
    }

    // join workers threads
    for (i = 0; i < numThreads; i++) {
        pthread_join(*(tids + i), NULL);
    }

    free(tids);

    printf("httpd: All workers finished \n");
    
    
    // delete sockets
    for (i = 0; i < 2; i++) {
        printf("Closing fd :%d\n", descriptors[i]);
        close(descriptors[i]);
    }

    free(descriptors);

    
    
    


    Queue_destroy(&qinfo);
    STATISTICS_destroy(&stats);

    return (EXIT_SUCCESS);
}
