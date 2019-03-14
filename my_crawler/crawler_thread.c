#include "crawler_thread.h"
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>      /* sockets */
#include <sys/types.h>      /* sockets */
#include <sys/socket.h>      /* sockets */
#include <netinet/in.h>      /* internet sockets */
#include <netdb.h>          /* gethostbyaddr */
#include <unistd.h>          /* fork */  
#include <ctype.h>          /* toupper */
#include <signal.h>          /* signal */
#include <string.h>

#include "queue.h"
#include "structs.h"
#include "functions_worker.h"
#include "communication_protocol.h"

void perror_exit_crawler(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void * thread_main(void * args) {
    ArgsPtr arguments = (ArgsPtr) args;
    QueueInfoPtr q = arguments->queue;
    StatisticsPtr stats = arguments->statistics;
    pthread_mutex_t *filemtx = arguments->filemtx;
    char *save_dir = arguments->save_dir;
    char * host = arguments->host;
    int port = arguments->port;
    TListInfoPtr templist = arguments->templist;
    int sock;
    int error;

    char cwd[1024];
    getcwd(cwd, sizeof (cwd));
    printf("Thread (%lu) working directory : %s \n", (unsigned long int) pthread_self(), cwd);



    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct hostent *rem;


    while (1) {


        //Dequeue url if finito break and terminate thread
        QueueElem qe;

        Queue_dequeue(&q, &qe, &error);

        //        if (qe.value >= 0) {
        //            // service 
        //            printf("Worker thread (%lu) dequeued url:%s\n", (unsigned long int) pthread_self(), qe.url);
        //        }
        //        if (qe.value < 0) {
        //            break;
        //        }

        if (strcmp(qe.url, "finito") == 0) {
            break;
        } else {
            printf("Worker thread (%lu) dequeued url:%s\n", (unsigned long int) pthread_self(), qe.url);
        }



        //Analyse-Tokenize url (host,page,port)
        char * starting_url = qe.url;
        char * trim_url = tokenize_url(starting_url); //returns trimed url like /sitex/pagex_y.html

        //printf("trimed url is : %s \n",trim_url);



        //Create and Build Request

        RequestPtr request = Request_build(trim_url, host, port);



        //Create socket 

        // ---------------------- CRAWLER SERVER CONNECTION SOCKET INITIALIZATION ------------------------//

        // 2. FTIAXE SOCKET
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror_exit_crawler("socket");
        }


        // 3. GET HOST

        if ((rem = gethostbyname(host)) == NULL) {
            herror("gethostbyname \n");
            exit(0);
        }

        server.sin_family = AF_INET; /* Internet domain */
        memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
        server.sin_port = htons(port); /* The given port */


        // 4. CONNECT TO SOCKET
        printf("Attempting to connect to server %s port:%d \n", host, port);
        if (connect(sock, serverptr, sizeof (server)) < 0) {
            perror("Connection failed!\n");
            free(trim_url);
            Request_destroy(&request);
            close(sock);
            continue;
        }


        //Send request

        Request_send(request, sock);

        Request_destroy(&request);


        //Receive Response header
        char buf[BUFSIZE];
        memset(buf, 0, BUFSIZE);
        int n = 0;
        if ((n = readResponseFromSocketBB(sock, buf, BUFSIZE)) < 0) { // read header byte byte till u find \r\n
            printf("error in communication!\n");
        } else {
            //printf("received %d bytes and response header: %s \n",n,buf);
        }



        //Parse Response to create folders from site# and files *careful needs mutex when creating folder
        int content_length = 0;
        int code = parseResponseHeader(buf, &content_length);

        printf("received code: %d and content_length %d \n", code, content_length);

        if (code == 404) {
            printf("File not found!\n");
        } else if (code == 403) {
            printf("Forbidden!\n");
        } else if (code == 400) {
            printf("Request was wrong read the manual!\n"); //DEN PREPEI NA SUMVEI POTE AN O CRAWLER STELNEI SWSTA REQUEST
        } else if (code == 200) {

            //construct fullpath savedir + site

            char * urlCopy = (char *) malloc(strlen(trim_url) + 1);
            memset(urlCopy, 0, strlen(trim_url) + 1);
            strncpy(urlCopy, trim_url, strlen(trim_url)); //    e.g   /site1/page1_1234.html

            char *save_ptr = NULL;
            char * sitefolder = multi_tok(urlCopy, &save_ptr, "page"); //e.g     /site1/

            char * fullpath = (char *) malloc(strlen(save_dir) + strlen(sitefolder) + 2);
            memset(fullpath, 0, (strlen(save_dir) + strlen(sitefolder) + 2));

            strcpy(fullpath, "");
            strcat(fullpath, save_dir);
            strcat(fullpath, sitefolder); // merge of /site1/ and /home/user/.../save_dir

            char * fullpathCopy = (char *) malloc(strlen(fullpath) + 1);
            memset(fullpathCopy, 0, strlen(fullpath) + 1);
            strncpy(fullpathCopy, fullpath, strlen(fullpath));

            //            printf("fullpath is: %s \n",fullpath);
            //            printf("trim_url is: %s \n",trim_url);
            //            printf("trim_url copy is: %s \n",urlCopy);

            char * filename = NULL;
            multi_tok(trim_url, &filename, urlCopy);

            //            printf("file name is: %s \n",filename);

            char * filepath = (char *) malloc(strlen(fullpath) + strlen(filename) + 2);
            memset(filepath, 0, (strlen(fullpath) + strlen(filename) + 2));

            strcpy(filepath, fullpath);
            strcat(filepath, filename);

            printf("Absolute filepath is: %s \n", filepath);

            pthread_mutex_lock(filemtx); //thelei mutex

            struct stat st = {0};

            if (stat(fullpath, &st) == -1) {          //check if folder exists
                printf("Creating folder %s ... \n", sitefolder);
                mkdir(fullpath, 0777);
            } else {
                printf("Folder already exists!Entering \n");
            }

            pthread_mutex_unlock(filemtx);

            //Read from response body write to new file
            int newfd = open(filepath, O_WRONLY | O_APPEND | O_CREAT, 0644);


            char buf[BUFSIZE] = {0};
            writeFileFromSocketC(sock, newfd, buf, BUFSIZE, content_length);


            close(newfd);


            pthread_mutex_lock(&stats->mtx);
            stats->numpages++;
            stats->totalbytes += (content_length);
            pthread_mutex_unlock(&stats->mtx);


            FILE * fp = fopen(filepath, "rb");
            // printf("\n\nFILENAME : %s\n\n",filename);
            char * line = NULL; //kathe grammi thewreitai ena keimeno
            ssize_t read;
            size_t length = 0;

            if (fp == NULL) {
                printf("ERROR when opening file \n");
            }

            while (((read = getline(&line, &length, fp)) != -1)) { //vres ta link mesa sto arxeio 

                if (strcmp(line, "\n") == 0) {
                    continue;
                }

                int lineLength = strlen(line);
                char *lineCopy = (char*) malloc((lineLength + 1) * sizeof (char));
                memset(lineCopy, 0, (lineLength + 1));
                strncpy(lineCopy, line, lineLength);


                char * save_ptr = NULL;
                char * trimline = strtok_r(lineCopy, "\n", &save_ptr);


                char * linkptr = multi_tok(trimline, &save_ptr, "href=\"");

                if (strcmp(linkptr, "<p></p><p><a ") == 0) {
                    //printf("link found and it is: ---%s---\n",save_ptr);
                    linkptr = multi_tok(NULL, &save_ptr, "\">");
                    //printf("link found and it is: ---%s---\n",linkptr);


                    int err = 0;
                    QueueElem sqe;
                    memset(sqe.url, 0, MAX_URL_LEN);
                    sqe.value = strlen(linkptr); //edw tha mpei to link pou vrisketai afou elegxthei oti dn uphr3e stin oura
                    strncpy(sqe.url, linkptr, strlen(linkptr));
                    Queue_enqueue(&q, sqe, &err, &templist);


                }

                free(lineCopy);
            }

            free(line);
            fclose(fp);

            free(filepath);
            free(urlCopy);
            free(fullpathCopy);
            free(fullpath);
        }



        //Search file for links and enqueue links to queue - checking if they existed in queue




        //Close socket

        free(trim_url);
        close(sock);

    }

    printf("Worker %d (%lu) exited \n", getpid(), (unsigned long int) pthread_self());



    free(args);

    return NULL;
}