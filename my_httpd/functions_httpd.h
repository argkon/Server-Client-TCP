#ifndef httpd_FUNCTIONS_H
#define httpd_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include <math.h>


void get_current_server_time(char ** atime);

//--------PARSING FUNCTIONS--------//
int httpd_parseCommand(char * line, char **commands, int * deadline);
char * getText(char * line,int * count);
char * getPath(char * line);

//--------POLLING FUNCTIONS--------//
bool * buildServicedWorkersTable(int numWorkers);
bool * buildReadyWorkersTable(int numWorkers);
struct pollfd * buildWaitingTable(bool * servicedWorkers, int ** descriptors, int numWorkers, int * sum_waiting);
int findWhichWorker(int ** descriptors, int w, int fd);
int findWhichWorkerByPID(int ** descriptors, int w, int pid);
bool * polling(int ** descriptors, struct pollfd * fds, bool * servicedWorkers, int numWorkers, int sum_waiting);


struct pollfd * buildWaitingTableFromSockets(int * descriptors);
int pollingForSockets(struct pollfd * fds);

//--------SERVICE FUNCTIONS-------//
void service_search(int ** descriptors, int numWorkers, int deadline);
void service_wc(int ** descriptors, int numWorkers);
void service_max(int ** descriptors, int numWorkers);
void service_min(int ** descriptors, int numWorkers);

void restartWorkers(int **descriptors,int numWorkers,int doclines,char* docfile);





#endif /* FUNCTIONS_H */
