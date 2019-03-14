#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "structs.h"
#include "functions_httpd.h"
#include <time.h>
#include <math.h>
#include "communication_protocol.h"
#include <sys/poll.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include "httpd_thread.h"

void get_current_server_time(char ** atime) {
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    *atime = asctime(timeinfo);

    *(strchr(*atime, '\n')) = '\0';
}

int httpd_parseCommand(char * line, char **commands, int * deadline) {
    int j = 0;
    int lineLength = strlen(line);
    char *lineCopy = (char*) malloc((lineLength + 1) * sizeof (char));
    char * p1;
    memset(lineCopy, 0, (lineLength + 1));
    strncpy(lineCopy, line, lineLength);

    p1 = strtok(lineCopy, " \t");

    if (p1 == NULL) {
        printf("error in allocation \n");
    }

    if (strcmp(p1, "/search") == 0) {
        strcpy(commands[0], p1);
        j++;

        while ((p1 = strtok(NULL, " \t\n")) != NULL && j < 13) {
            if (strcmp(p1, "-d") == 0) {
                p1 = strtok(NULL, " \t\n");

                if (p1 != NULL) {
                    *deadline = atoi(p1);
                    break;
                } else {
                    return 0;
                }
            } else {
                strcpy(commands[j], p1);
                j++;
            }
        }
    } else if (strcmp(p1, "/maxcount") == 0) {
        strcpy(commands[0], p1);
        j++;
        while ((p1 = strtok(NULL, " \t\n")) != NULL && j < 2) {
            strcpy(commands[j], p1);
            j++;
        }
    } else if (strcmp(p1, "/mincount") == 0) {
        strcpy(commands[0], p1);
        j++;
        while ((p1 = strtok(NULL, " \t\n")) != NULL && j < 2) {
            strcpy(commands[j], p1);
            j++;
        }
    } else if ((strcmp(p1, "/wc\n") == 0)) {
        j++;
        strcpy(commands[0], p1);
    } else if (strcmp(p1, "/exit\n") == 0) {
        strcpy(commands[j], p1);
        j++;
    } else {
        free(lineCopy);
        return 0;
    }

    free(lineCopy);

    return j;
}

char * getPath(char * line) {

    int lineLength = strlen(line);
    char *lineCopy = (char*) malloc((lineLength + 1) * sizeof (char));
    char * path = malloc((lineLength + 1) * sizeof (char));
    memset(lineCopy, 0, (lineLength + 1));
    strncpy(lineCopy, line, lineLength);

    char * p = strtok(lineCopy, "\n");

    if (p == NULL || lineCopy == NULL) {
        printf("ERROR IN PARSING \n");
        exit(0);
        return NULL;
    }


    int pathlength = strlen(p);
    memset(path, 0, pathlength + 1);
    strncpy(path, p, pathlength);

    free(lineCopy);
    return path;
}

char * getText(char * line, int * count) { //to keimeno ths grammis

    int lineLength = strlen(line);
    char *lineCopy = (char*) malloc((lineLength + 1) * sizeof (char));
    char *buffer = malloc((lineLength + 1) * sizeof (char));

    memset(lineCopy, 0, (lineLength + 1));
    strncpy(lineCopy, line, lineLength);

    char *remain = strtok(lineCopy, " \t");
    remain = strtok(NULL, "\n");

    char * p1 = strtok(remain, " \t");
    (*count)++;
    memset(buffer, 0, lineLength + 1);

    if (p1 == NULL) {
        printf("Missing Text \n");
        exit(0);
        return NULL;
    }

    strcat(buffer, p1);
    strcat(buffer, " ");

    while ((p1 = strtok(NULL, " \t")) != NULL) {
        strcat(buffer, p1);
        strcat(buffer, " ");
        (*count)++;
    }

    if (remain == NULL || lineCopy == NULL) {
        return NULL;
    }

    free(lineCopy);

    return buffer;
}

bool * buildServicedWorkersTable(int numWorkers) {
    int i;
    bool * servicedWorkers = malloc(sizeof (bool) * numWorkers);

    for (i = 0; i < numWorkers; i++) {
        servicedWorkers[i] = false;
    }
    return servicedWorkers;
}

bool * buildReadyWorkersTable(int numWorkers) {
    int i;
    bool * readyWorkers = malloc(sizeof (bool) * numWorkers);

    for (i = 0; i < numWorkers; i++) {
        readyWorkers[i] = false;
    }
    return readyWorkers;
}

struct pollfd * buildWaitingTableFromSockets(int * descriptors) {
    struct pollfd * fds = malloc(sizeof (struct pollfd) * 2);
    int i;

    for (i = 0; i < 2; i++) {
        fds[i].fd = descriptors[i];
        fds[i].events = POLLIN | POLLHUP;
        fds[i].revents = 0;
    }

    return fds;
}

struct pollfd * buildWaitingTable(bool * servicedWorkers, int ** descriptors, int numWorkers, int * sum_waiting) {
    int i = 0, j = 0;
    *sum_waiting = 0;

    for (i = 0; i < numWorkers; i++) {
        if (servicedWorkers[i] == false) {
            (*sum_waiting)++;
        }
    }

    if (*sum_waiting == 0) {
        return NULL;
    }


    struct pollfd * fds = malloc(sizeof (struct pollfd) * (*sum_waiting));

    for (i = 0; i < numWorkers; i++) {
        if (servicedWorkers[i] == false) {
            fds[j].fd = descriptors[i][2];
            fds[j].events = POLLIN | POLLHUP;
            fds[j].revents = 0;
            j++;
        }
    }

    return fds;
}

int findWhichWorker(int ** descriptors, int w, int fd) {
    int i;

    for (i = 0; i < w; i++) {
        if (descriptors[i][2] == fd) {
            return i;
        }
    }

    printf("  findWhichWorker() failed \n");
    exit(0);

    return -1;
}

int findWhichWorkerByPID(int ** descriptors, int w, int pid) {
    int i;

    for (i = 0; i < w; i++) {
        if (descriptors[i][0] == pid) {
            return i;
        }
    }

    printf("  findWhichWorker() failed \n");
    exit(0);

    return -1;
}

int pollingForSockets(struct pollfd * fds) {
    int rc;
    int i;
    int nfds = 2;

    rc = poll(fds, nfds, -1);

    if (rc < 0) {
        perror("  poll() failed");
        exit(0);
    }

    if (rc == 0) {
        printf("  poll() timed out.  End program.\n");
        exit(0);
    }

    for (i = 0; i < nfds; i++) {
        if (fds[i].revents == 0) {
            continue;
        }

        if ((fds[i].revents & POLLIN) != 0) {
            int whichsocket = fds[i].fd;
            return whichsocket;
        } else if ((fds[i].revents & POLLHUP) != 0) {
            exit(0);
            return -1;
        }
    }
    
    return -1;
}

bool * polling(int ** descriptors, struct pollfd * fds, bool * servicedWorkers, int numWorkers, int sum_waiting) {
    bool * readyWorkers = buildReadyWorkersTable(numWorkers);
    int rc;
    int i;
    int nfds = sum_waiting;

    rc = poll(fds, nfds, -1);

    if (rc < 0) {
        perror("  poll() failed");
        exit(0);
    }

    if (rc == 0) {
        printf("  poll() timed out.  End program.\n");
        exit(0);
    }

    for (i = 0; i < nfds; i++) {
        if (fds[i].revents == 0) {
            continue;
        }

        if ((fds[i].revents & POLLIN) != 0) {
            int whichworker = findWhichWorker(descriptors, numWorkers, fds[i].fd);

            if (servicedWorkers[whichworker] == false) {
                readyWorkers[whichworker] = true;
            }
        } else if ((fds[i].revents & POLLHUP) != 0) {
            int whichworker = findWhichWorker(descriptors, numWorkers, fds[i].fd);

            if (servicedWorkers[whichworker] == false) {
                servicedWorkers[whichworker] = true;
            }
        }
    }

    return readyWorkers;
}

void service_search(int ** descriptors, int numWorkers, int deadline) {
    int i, k, p;
    int sum_waiting = numWorkers;
    bool * servicedWorkers = buildServicedWorkersTable(numWorkers);

    time_t t = time(0);

    while (sum_waiting > 0) {
        struct pollfd * fds = buildWaitingTable(servicedWorkers, descriptors, numWorkers, &sum_waiting);

        if (fds == NULL || sum_waiting == 0) {
            break;
        }

        bool * readyWorkers = polling(descriptors, fds, servicedWorkers, numWorkers, sum_waiting);

        time_t poll_t = time(0);

        int delay = (int) difftime(poll_t, t);

        for (i = 0; i < numWorkers; i++) {
            if (readyWorkers[i] == true) {

                if (readNumber(descriptors[i][2], &k) != COMMUNICATION_SUCCESSFUL) { //diavase posa apotelesmata na perimeneis apo ton i worker
                    printf("Job httpd: read/write error \n");
                    exit(0);
                }
                printf("httpd:NUMBER OF RESULTS: %d \n", k);

                int j;
                char* c = NULL;
                for (j = 0; j < k; j++) {
                    if (readString(descriptors[i][2], &c) != COMMUNICATION_SUCCESSFUL) { //diavase ta apotelesmata
                        printf("Job httpd: read/write error \n");
                        exit(0);
                    }

                    if (delay < deadline || deadline == 0) {
                        printf("%s\n", c);
                    }
                    free(c);
                }

                //                free(c);

                if (readNumber(descriptors[i][2], &p) != COMMUNICATION_SUCCESSFUL) { //diavase posa apotelesmata na perimeneis apo ton i worker
                    printf("Job httpd: read/write error \n");
                    exit(0);
                }

                if (p != -1) { //eleg3e gia termatismo epikoinwnias tou i worker
                    printf("Pipe communication error!\n");
                    exit(0);
                }


                servicedWorkers[i] = true;

            }
        }
        free(readyWorkers);
        free(fds);
    }

    free(servicedWorkers);
}

void service_max(int ** descriptors, int numWorkers) {
    int i;
    int sum_waiting = numWorkers;
    bool * servicedWorkers = buildServicedWorkersTable(numWorkers);
    int totalmax = -1;
    char * totalmaxpath = NULL;
    while (sum_waiting > 0) {
        struct pollfd * fds = buildWaitingTable(servicedWorkers, descriptors, numWorkers, &sum_waiting);

        // printf("=====================> httpd: remaining workers: %d \n", sum_waiting);

        if (fds == NULL || sum_waiting == 0) {
            break;
        }

        bool * readyWorkers = polling(descriptors, fds, servicedWorkers, numWorkers, sum_waiting);
        //int ** maxtable = malloc(sizeof (int*)*numWorkers);

        for (i = 0; i < numWorkers; i++) {
            if (readyWorkers[i] == true) {

                char * c = NULL;
                int max = -1;

                if (readNumber(descriptors[i][2], &max) != COMMUNICATION_SUCCESSFUL) {
                    printf("Job httpd: read/write error \n");
                    exit(0);
                }
                if (max > 0) {
                    if (readString(descriptors[i][2], &c) != COMMUNICATION_SUCCESSFUL) {
                        printf("Job httpd: read/write error \n");
                        exit(0);
                    }
                    //printf("path: %s \n",c);
                }

                if (max > totalmax) {
                    totalmax = max;
                    totalmaxpath = malloc(sizeof (char)*(strlen(c) + 1));
                    //memset(totalmaxpath,0,strlen(c));
                    strcpy(totalmaxpath, c);
                } else if (max == totalmax && max != -1) {
                    int currsize = strlen(c);
                    int prevsize = strlen(totalmaxpath);
                    if (currsize < prevsize) {
                        totalmax = max;
                        totalmaxpath = malloc(sizeof (char)*(strlen(c) + 1));
                        strcpy(totalmaxpath, c);
                    } else {
                        //nothing
                    }
                }

                servicedWorkers[i] = true;
                if (c != NULL) {
                    free(c);
                }
            }
        }

        free(readyWorkers);
        free(fds);
    }

    if (totalmaxpath == NULL) {
        printf("WORD NOT FOUND!!! \n");
    } else {
        printf("httpd: File with max number is %s and contains the word %d times.\n", totalmaxpath, totalmax);
        free(totalmaxpath);
    }


    free(servicedWorkers);
}

void service_min(int ** descriptors, int numWorkers) {
    int i;
    int sum_waiting = numWorkers;
    bool * servicedWorkers = buildServicedWorkersTable(numWorkers);
    int totalmin = 100000000;
    char * totalminpath = NULL;
    while (sum_waiting > 0) {
        struct pollfd * fds = buildWaitingTable(servicedWorkers, descriptors, numWorkers, &sum_waiting);

        //printf("=====================> httpd: remaining workers: %d \n", sum_waiting);

        if (fds == NULL || sum_waiting == 0) {
            break;
        }

        bool * readyWorkers = polling(descriptors, fds, servicedWorkers, numWorkers, sum_waiting);

        for (i = 0; i < numWorkers; i++) {
            if (readyWorkers[i] == true) {

                char * c = NULL;
                int min = -1;

                if (readNumber(descriptors[i][2], &min) != COMMUNICATION_SUCCESSFUL) {
                    printf("Job httpd: read/write error \n");
                    exit(0);
                }
                if (min > 0) {
                    if (readString(descriptors[i][2], &c) != COMMUNICATION_SUCCESSFUL) {
                        printf("Job httpd: read/write error \n");
                        exit(0);
                    }
                    //printf("path: %s \n",c);
                }

                if (min < totalmin && min > 0) {
                    totalmin = min;
                    totalminpath = malloc(sizeof (char)*(strlen(c) + 1));
                    strcpy(totalminpath, c);
                    // printf("totalminpath: %s \n",totalminpath);
                }

                servicedWorkers[i] = true;

                if (c != NULL) {
                    free(c);
                }
            }
        }

        free(readyWorkers);
        free(fds);
    }

    if (totalminpath == NULL) {
        printf("WORD NOT FOUND!!! \n");
    } else {
        printf("httpd: File with min number is %s and contains the word %d times.\n", totalminpath, totalmin);
        free(totalminpath);
    }


    free(servicedWorkers);

}

void service_wc(int ** descriptors, int numWorkers) {
    int i, j;
    int sum_waiting = numWorkers;
    bool * servicedWorkers = buildServicedWorkersTable(numWorkers);

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    int linecount = 0;
    int wordcount = 0;
    int charcount = 0;
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    while (sum_waiting > 0) {
        struct pollfd * fds = buildWaitingTable(servicedWorkers, descriptors, numWorkers, &sum_waiting);

        printf("=====================> httpd: remaining workers: %d \n", sum_waiting);

        if (fds == NULL || sum_waiting == 0) {
            break;
        }

        bool * readyWorkers = polling(descriptors, fds, servicedWorkers, numWorkers, sum_waiting);

        for (i = 0; i < numWorkers; i++) {
            if (readyWorkers[i] == true) {
                // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                int mapsize;
                char * c = NULL;
                readNumber(descriptors[i][2], &mapsize);

                for (j = 0; j < mapsize; j++) {
                    if (readString(descriptors[i][2], &c) != COMMUNICATION_SUCCESSFUL) {
                        printf("Job httpd: read/write error \n");
                        exit(0);
                    }
                    //calculate total word count
                    //printf("%s \n", c);
                    char *counter = strtok(c, " \n");
                    if (counter == NULL) {
                        printf("strtok error ! \n");
                    }
                    //                    printf("Linecount: %s \n", counter);
                    linecount = linecount + atoi(counter);
                    //                    printf("Linecount: %d \n",linecount);
                    counter = strtok(NULL, " \n");
                    if (counter == NULL) {
                        printf("strtok error ! \n");
                    }
                    //                    printf("Wordcount: %s \n", counter);
                    wordcount = wordcount + atoi(counter);
                    //                    printf("Wordcount: %d \n",wordcount);
                    counter = strtok(NULL, " \n");
                    if (counter == NULL) {
                        printf("strtok error ! \n");
                    }
                    //                    printf("Charcount: %s \n", counter);
                    charcount = charcount + atoi(counter);
                    //                    printf("Charcount: %d \n",charcount);


                    free(c);
                }
                // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                servicedWorkers[i] = true;
            }
        }

        free(readyWorkers);
        free(fds);
    }

    free(servicedWorkers);

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    printf("linecount: %d wordcount: %d charcount: %d \n", linecount, wordcount, charcount);
}

//void restartWorkers(int **descriptors, int numWorkers, int doclines, char* docfile) {
//        
//        pid_t exitedWorker;
//        char temporaryBuffer[1000];
//        int i,j;
//        char * line = NULL;
//        ssize_t read;
//        size_t length = 0;
//        
//        while ((exitedWorker = waitpid(-1, NULL, WNOHANG)) > 0) {       //restart workers in case of kill | pid -1 for any process, stat_loc NULL, WNOHANG to not block the main process
//                printf("worker exited with pid: %d \n", exitedWorker);
//    
//                i = findWhichWorkerByPID(descriptors, numWorkers, exitedWorker);
//    
//                printf("worker has number: %d \n", i);
//    
//                sprintf(temporaryBuffer, "workerpipe%d.pipe", i);
//    
//                unlink(temporaryBuffer);
//    
//                if ((mkfifo(temporaryBuffer, PERMS) < 0) && (errno != EEXIST)) {
//                    perror("can't create fifo");
//                }
//    
//                sprintf(temporaryBuffer, "jobpipe%d.pipe", i);
//    
//                unlink(temporaryBuffer);
//    
//                if ((mkfifo(temporaryBuffer, PERMS) < 0) && (errno != EEXIST)) {
//                    perror("can't create fifo");
//                }
//    
//                pid_t childpid;
//    
//                childpid = fork();
//                if (childpid == -1) {
//                    perror("httpd : Failed to fork, exiting");
//                    return;
//                }
//    
//                if (childpid == 0) {
//                    printf("I am the child process with ID : %lu \n", (unsigned long) getpid());
//                    worker_main(i);
//                    exit(0);
//                } else {
//                    //            printf("I am the parent process with ID : % lu \n", (unsigned long) getpid());
//    
//                    close(descriptors[i][1]);
//                    close(descriptors[i][2]);
//    
//                    descriptors[i][0] = (int) childpid;
//                    descriptors[i][1] = -1;
//                    descriptors[i][2] = -1;
//                }
//    
//                // 3 open pipes
//                sprintf(temporaryBuffer, "workerpipe%d.pipe", i);
//    
//                if (((descriptors[i][1] = open(temporaryBuffer, O_WRONLY)) < 0)) {
//                    perror("can't open fifo");
//                }
//    
//                sprintf(temporaryBuffer, "jobpipe%d.pipe", i);
//    
//                if (((descriptors[i][2] = open(temporaryBuffer, O_RDONLY)) < 0)) {
//                    perror("can't open fifo");
//                }
//    
//                // to posa path tha steilei ston worker
//                int pathNumber = 0;
//    
//                if (i < doclines % numWorkers) {
//                    pathNumber = doclines / numWorkers + 1;
//                } else {
//                    pathNumber = doclines / numWorkers;
//                }
//    
//                if (sendNumber(descriptors[i][1], pathNumber) != COMMUNICATION_SUCCESSFUL) {
//                    printf("Job httpd: read/write error \n");
//                    exit(0);
//                }
//    
//                FILE * F2 = fopen(docfile, "r");
//    
//                int count = 0;
//                
//                while (((read = getline(&line, &length, F2)) != -1)) { //stelne monopatia stous workers
//                    // send sto pipe me id count mod numWorkers
//    
//                    char *path = getPath(line); //trim the \n at the end of path names
//    
//                    if (count == i) {
//                        printf("path is : %s , assigned to worker %d \n", path, count);
//    
//                        if (sendString(descriptors[count][1], path) != COMMUNICATION_SUCCESSFUL) {
//                            printf("Job httpd: read/write error  \n");
//                            exit(0);
//                        }
//                    }
//    
//                    count++;
//    
//                    if (count == numWorkers) {
//                        count = 0;
//                    }
//    
//                    free(path);
//                }
//    
//                if (sendNumber(descriptors[i][1], -1) != COMMUNICATION_SUCCESSFUL) {
//                    printf("Job httpd: read/write error \n");
//                    exit(0);
//                }
//                printf("count is: %d \n", count);
//                fclose(F2);
//    
//                if (readNumber(descriptors[i][2], &j) != COMMUNICATION_SUCCESSFUL) {
//                    printf("Job httpd: read/write error \n");
//                    exit(0);
//                }
//    
//                printf("All new workers have finished their setup. \n");
//            }
//}
