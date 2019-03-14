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
#include <pthread.h>
#include <sys/stat.h>

#include "crawler_thread.h"
#include "queue.h"
#include "structs.h"
#include "communication_protocol.h"
#include "functions_worker.h"


void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    
    struct timeval start, current;
    gettimeofday(&start, NULL);
    
    int connectPort;
    int commandPort;
    char * host;
    int numThreads;
    char * save_dir;
    char * starting_url;
    int * descriptors = 0;
    int i;
    int err=0;
    
    pthread_t *tids;
    
    QueueInfoPtr qinfo;
    StatisticsPtr stats = STATISTICS_create();
    
    
    pthread_mutex_t * filemtx = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(filemtx,0);
    
    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof (client);
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct sockaddr *clientptr = (struct sockaddr *) &client;
    
    
    if (argc == 12) {
        if (strcmp(argv[1], "-h") == 0 && strcmp(argv[3], "-p") == 0 && 
                strcmp(argv[5], "-c") == 0 && strcmp(argv[7], "-t") == 0 && 
                strcmp(argv[9], "-d") == 0 && argv[11] != NULL) { // orismata
            host = argv[2];
            connectPort = atoi(argv[4]);
            commandPort = atoi(argv[6]);
            numThreads = atoi(argv[8]);
            save_dir = argv[10];
            starting_url = argv[11];
        } else {
            printf("Wrong Arguments!! \n");
            exit(0);
        }
    } else {
        printf("Wrong Arguments!! \n");
        exit(0);
    }

    struct stat st = {0};

    if (stat(save_dir, &st) == -1) {
        printf("Creating folder %s ... \n", save_dir);
        int make = mkdir(save_dir, 0777);
        if(make != 0){
            printf("failed to create folder save_dir! Please enter the folder path correctly!\n");
            exit(0);
        }
        //chdir(save_dir);
    } else {
        printf("Save directory already exists!\n");
        //chdir(save_dir);
    }
    
    descriptors = malloc(sizeof (int)*2);
    for (i = 0; i < 2; i++) {
        descriptors[i] = 0;
    }
    
    
    printf("host: %s \nconnect port: %d \ncommand port: %d \nnumber of threads: %d \nsave directory: %s \nstarting URL:%s \n", host,connectPort, commandPort, numThreads, save_dir ,starting_url);
    
    // 0. FTIA3E FIFO KAI VALE TO STARTING URL
    
    TListInfoPtr templist = TList_create();
    
    
    qinfo = Queue_create();
    QueueElem sqe;
    

    char * trim_url = tokenize_url(starting_url);
    
    memset(sqe.url,0,MAX_URL_LEN);
    sqe.value = strlen(trim_url);
    strncpy(sqe.url,trim_url,strlen(trim_url));
    

    printf("starting URL is :---%s---\n",sqe.url);
    Queue_enqueue(&qinfo, sqe, &err,&templist);
    
 
    // 1. FTIA3E THREADS
    
    if ((tids = malloc(numThreads * sizeof (pthread_t))) == NULL) {
        perror("malloc");
        exit(1);
    }

    for (i = 0; i < numThreads; i++) {
        ArgsPtr args = Args_create();
        args->queue = qinfo;
        args->save_dir = save_dir;
        args->statistics = stats;
        args->port = connectPort;
        args->host = host;
        args->templist = templist;
        args->filemtx = filemtx;

        if ((err = pthread_create(tids + i, NULL, thread_main, (void *) args)) != 0) {
            perror("httpd : Failed to pthread_create, exiting");
            return 1;
        }
    }
    
    
    // ---------------------- CRAWLER COMMAND SOCKET INITIALIZATION - descriptors[0] ------------------------//
    int tr = 1;
    // 2. FTIAXE SOCKET
    if ((descriptors[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror_exit("socket");
    }
    
    //www.unix.com/programming/29475-how-solve-error-bind-address-already-use.html
    if (setsockopt(descriptors[0], SOL_SOCKET, SO_REUSEADDR, &tr, sizeof (int)) == -1) {    //sets option name REUSEADDRESS to tr=true=1 value to re-bind socket if already bind
        perror("\n setsockopt \n");
    }

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(commandPort); /* The given port */

    // 3. BIND SOCKET

    if (bind(descriptors[0], serverptr, sizeof (server)) < 0) {
        perror_exit("bind");
    }
    
    // 4. LISTEN SOCKET

    if (listen(descriptors[0], 128) < 0) {
        perror_exit("listen");
    }
    

    
    
    
    //---------------------------- MAIN BODY --------------------------------//
    
    
    while (1) {
        printf("===================== crawler : waiting for command =====================\n");
        int newsock;
        int sock = descriptors[0]; 

        // 5. ACCEPT
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0) {
            perror_exit("accept");
        }

        printf("Accepted connection from %d and assigned to fd: %d \n", sock, newsock);

        // E3UPHRETHSE TIN ENTOLI (command)

        //        char* buffer = NULL;
        char buffer[1024] = {0};


        // command
        printf("command request \n");
        //            break;
        if ((i = readCommandFromSocketBB(newsock, buffer, 1024)) == 0) { //actually its chunk by chunk todo: change name of function
            printf("httpd: error in read from socket \n");
            continue;
        } else {
            printf("Bytes Read are:%d\n and String read is :%s \n", i, buffer);
            if (strcmp(buffer, "SHUTDOWN\n") == 0 || strcmp(buffer, "shutdown\n") == 0 || strcmp(buffer, "SHUTDOWN\r\n") == 0 || strcmp(buffer, "shutdown\r\n") == 0) { //Serve Shutdown Command
                close(newsock);
                //free(buffer);
                break;
            } else if (strcmp(buffer, "STATS\n") == 0 || strcmp(buffer, "stats\n") == 0 || strcmp(buffer, "STATS\r\n") == 0 || strcmp(buffer, "stats\r\n") == 0) { //Serve Stats Command
                
                printf("GOT STATS COMMAND! \n");
                
                gettimeofday(&current, NULL);
                long int seconds = current.tv_sec - start.tv_sec;
                long int microseconds = current.tv_usec - start.tv_usec;
//
//                //                    time_t curtime = seconds;
//                //                    struct tm *t = localtime(&curtime);
//                //                    printf("%02d:%02d.%03d\n",t->tm_min, t->tm_sec, microseconds/1000);
//
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
    
    
    
    QueueElem qe;
    //qe.value = -1;
    memset(qe.url,0,MAX_URL_LEN);
    strcpy(qe.url,"finito");

    for (i = 0; i < numThreads; i++) {
        Queue_enqueue(&qinfo, qe, &err,&templist);
        if (err == 1) {
            perror_exit("Error in enqueue!!\n");
        }
    }

    
    // Join Workers Threads
    for (i = 0; i < numThreads; i++) {
        pthread_join(*(tids + i), NULL);
    }

    free(tids);

    printf("httpd: All workers finished \n");
    
    
     // delete sockets
    for (i = 0; i < 1; i++) {
        printf("Closing fd :%d\n", descriptors[i]);
        close(descriptors[i]);
    }
    
    free(descriptors);
    free(trim_url);
    
    TList_destroy(&templist);
    Queue_destroy(&qinfo);
    STATISTICS_destroy(&stats);
    pthread_mutex_destroy(filemtx);
    free(filemtx);
    
    
    
    return (EXIT_SUCCESS);
}

