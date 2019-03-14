#include <malloc.h>
#include "httpd_thread.h"
#include "communication_protocol.h"
#include "functions_worker.h"
#include "functions_httpd.h"
#include "queue.h"
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>


void * worker_main(void * args) { //WORKER THREAD
    ArgsPtr arguments = (ArgsPtr) args;
    QueueInfoPtr q = arguments->queue;
    StatisticsPtr stats = arguments->statistics;
    char *root_dir = arguments->root_dir;
    int error;
    
    
    printf("Worker %d (%lu) started \n", (int) getpid(), (unsigned long int) pthread_self());

    while (1) {
        //        break;

        QueueElem qe;

        Queue_dequeue(&q, &qe, &error);

        if (qe.value >= 0) {
            // service 
            printf("Worker thread (%lu) dequeued %d\n", (unsigned long int) pthread_self(), qe.value);
        }
        if (qe.value < 0) {
            break;
        }

        int newsock = qe.value;

        int headersize;
        char headerbuffer[BUFSIZE];
        memset(headerbuffer,0,BUFSIZE);
        if ((headersize = readRequestFromSocketC(newsock, headerbuffer, BUFSIZE)) < 0) {
            printf("Error in read from socket!\n");
        } else { //vres to html ston disko kai meta kane write alliws ektupwse mhnuma lathous
            
            if (headersize == 0) {
                close(newsock);
                continue;
            }
            printf("Read %d bytes\n", headersize);
            printf("Message read is: %s\n", headerbuffer);

            // ------- VALIDATION IS REQUIRED -------- //

            int val = requestValidation(headerbuffer);
            //            printf("---------- VALID REQUEST:%d ----------- \n", val);
            if (val == 0) {
                printf("Invalid HTTP request!! Next Please! \n");
                //free(buffer);

                ResponsePtr response = Response_create();
                response->code = 400;

                Response_send(response, newsock);
                
                Response_destroy(&response);
                printf("Closing fd :%d\n", newsock);
                close(newsock);
                continue;
            }



            // get the html relative path from header
            int bufferLength = strlen(headerbuffer);
            char * bufferCopy = (char*) malloc((bufferLength + 1) * sizeof (char));
            memset(bufferCopy, 0, (bufferLength + 1));
            strncpy(bufferCopy, headerbuffer, bufferLength);

            char * rest = NULL;
            char *p = strtok_r(headerbuffer, "\n", &rest);
            p = strtok_r(headerbuffer, " \t", &rest);
            p = strtok_r(NULL, " \t", &rest);
            printf("Requested html is %s .................\n", p);


            //create filepath by merging relative with root
            char * filepath = (char *) malloc(strlen(p) + strlen(root_dir) + 2);
            memset(filepath, 0, (strlen(p) + strlen(root_dir) + 2));

            strcpy(filepath, root_dir);
            strcat(filepath, p);

            printf("Will find file %s \n", filepath);


            //stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
            //stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c


            ResponsePtr response = Response_build(filepath);

            Response_send(response, newsock);

            if (response->code == 200) {    //send contents to newsocket
//                char *buf = malloc(sizeof(char)* (BUFSIZE));
//                memset(buf,'\0',(BUFSIZE));
                char buf[BUFSIZE];
                memset(buf,0,(BUFSIZE));

                pthread_mutex_lock(&stats->mtx);
                stats->numpages++;
                stats->totalbytes+=(response->content_length);
                pthread_mutex_unlock(&stats->mtx);
                
                int source = open(filepath, O_RDONLY);
                if (source > 0) {
                    sendFileToSocketC(source, newsock, buf, BUFSIZE, response->content_length);
                    close(source);
                }
//                free(buf);
            }

            Response_destroy(&response);

            free(bufferCopy);
            free(filepath);
        }


        // free(buffer);

        printf("Closing fd :%d\n", newsock);
        close(newsock);
    }



    printf("Worker %d (%lu) exited \n", getpid(), (unsigned long int) pthread_self());


    free(args);

    return NULL;
}