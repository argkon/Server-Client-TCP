#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#define COMMAND_LIMIT 13
#define COMMAND_SIZE 256
#define BUFSIZE 1024
#define CHUNKSIZE 50


#define PERMS 0666

#define COMMUNICATION_SUCCESSFUL 1
#define COMMUNICATION_FAILURE 0


int sendNumber(int fd, int number);

int readNumber(int fd, int * number);

int sendString(int fd, char * buffer);

int readString(int fd, char ** buffer);

int readCommandFromSocket(int fd, char ** buffer);

ssize_t readCommandFromSocketC(int fd, void * buf,size_t nbyte);

ssize_t readRequestFromSocketBB(int fd, void * buf,size_t nbyte);

ssize_t readRequestFromSocketC(int fd, void * buf,size_t nbyte);

ssize_t sendResponseToSocketC(int fd, const void * buf,size_t nbyte);

ssize_t sendFileToSocketC(int source, int destination, void * buf, size_t buffersize, size_t filesize);

int readAll(int fd, char ** buffer,size_t nbyte);

#endif /* COMMUNICATION_PROTOCOL_H */

