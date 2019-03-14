#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "communication_protocol.h"

//----------------------------PROTOKOLLO EPIKOINWNIAS----------------------------//

int sendNumber(int fd, int number) {
    int i;

    i = write(fd, &number, sizeof (number));

    switch (i) {
        case -1:
            if (errno != EINTR) {
                printf("demolition man \n");
                exit(0);
            }
            break;
        case 0: // end of communication
            printf("demolition man \n");
            exit(0);
            break;
        default:
            return COMMUNICATION_SUCCESSFUL;

    }

    return COMMUNICATION_FAILURE;
}

int readNumber(int fd, int * number) {
    int i;

    i = read(fd, number, sizeof (*number));

    switch (i) {
        case -1:
            if (errno != EINTR) {
                printf("demolition man \n");
                exit(0);
            }
            break;
        case 0: // end of communication
            printf("demolition man \n");
            exit(0);
            break;
        default:
            return COMMUNICATION_SUCCESSFUL;

    }

    return COMMUNICATION_FAILURE;
}

int sendString(int fd, char * buffer) {
    int i;

    i = sendNumber(fd, strlen(buffer));

    if (i == COMMUNICATION_FAILURE) {
        return COMMUNICATION_FAILURE;
    }

    i = write(fd, buffer, strlen(buffer));

    switch (i) {
        case -1:
            if (errno != EINTR) {
                printf("demolition man \n");
                exit(0);
            }
            break;
        case 0: // end of communication
            printf("demolition man \n");
            exit(0);
            break;
        default:
            return COMMUNICATION_SUCCESSFUL;

    }

    return COMMUNICATION_FAILURE;
}

int readString(int fd, char ** buffer) {
    int i;
    int number;

    i = readNumber(fd, &number);
    if (i == COMMUNICATION_FAILURE) {
        return COMMUNICATION_FAILURE;
    }
    *buffer = malloc(sizeof (char)*(number + 1));

    (*buffer)[number] = '\0';

    i = read(fd, *buffer, number);

    switch (i) {
        case -1:
            if (errno != EINTR) {
                printf("demolition man \n");
                exit(0);
            }
            break;
        case 0: // end of communication
            printf("demolition man \n");
            exit(0);
            break;
        default:
            return COMMUNICATION_SUCCESSFUL;

    }

    return COMMUNICATION_FAILURE;
}

int readCommandFromSocket(int fd, char ** buffer) {
    int i;
    int number = BUFSIZE;

    *buffer = calloc(number, sizeof (char));

    i = read(fd, *buffer, number);

    switch (i) {
        case -1:
            if (errno != EINTR) {
                printf("demolition man \n");
                exit(0);
            }
            break;
        case 0: // end of communication
            printf("EOF!\n");
            return COMMUNICATION_SUCCESSFUL;
        default:
            return i;
    }

    return COMMUNICATION_FAILURE;
}

ssize_t readCommandFromSocketBB(int fd, void * buf, size_t nbyte) { //byte-byte

    ssize_t n = 0;
    ssize_t nread = 0;


    do {
        if ((n = read(fd, &((char *) buf) [nread], CHUNKSIZE)) == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nread;
        }


        //printf("current buffer: %s \n", (char*)buf);

        nread += n;

        if (((char *) buf)[nread - 1] == '\n') {
            printf("reached end condition!! \n");
            return nread;
        }


    } while (nread < nbyte);

    return nread; //edw dn tha ftasei pote thewritika
}

ssize_t readResponseFromSocketBB(int fd, void * buf, size_t nbyte) { //byte-byte

    ssize_t n = 0;
    ssize_t nread = 0;


    do {
        if ((n = read(fd, &((char *) buf) [nread], 1)) == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nread;
        }

        //printf("Character read is %c \n", ((char *) buf)[nread]);
        //printf("current buffer: %s \n", (char*) buf);


        if (nread >= 3) {
            if (((char *) buf)[nread] == '\n' && ((char *) buf)[nread - 1] == '\r' && ((char *) buf)[nread - 2] == '\n' && ((char *) buf)[nread - 3] == '\r') {
                printf("reached end condition!! \n");
                return nread;
            }
        }

        nread += n;

    } while (nread < nbyte);

    return nread; //edw dn tha ftasei pote thewritika
}

ssize_t readRequestFromSocketC(int fd, void * buf, size_t nbyte) { //chunks of 50 bytes

    ssize_t n = 0;
    ssize_t nread = 0;

    do {
        n = read(fd, &((char *) buf) [nread], CHUNKSIZE);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nread;
        }

        printf("current buffer: %s \n", (char*) buf);

        nread += n;

        if (nread > 3) {
            if (((char *) buf)[nread - 1] == '\n' && ((char *) buf)[nread - 2] == '\r' && ((char *) buf)[nread - 3] == '\n' && ((char *) buf)[nread - 4] == '\r') {
                printf("reached end condition!! \n");
                return nread;
            }
        }
    } while (nread < nbyte);

    return nread; //edw dn tha ftasei pote thewritika
}

ssize_t sendResponseToSocketC(int fd, const void * buf, size_t nbyte) { //chunks of 50 bytes

    ssize_t n = 0;
    ssize_t nwritten = 0;
    


    do {
        if (nbyte-nwritten > CHUNKSIZE) {
            n = write(fd, &((const char *) buf) [nwritten], CHUNKSIZE);
        } else {
            n = write(fd, &((const char *) buf) [nwritten], nbyte-nwritten);
        }
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nwritten;
        }
             
        //printf("--------------Wrote %d bytes and current buffer: %s \n", n , (char*) buf);
        nwritten += n;
    } while (nwritten < nbyte);
    
    

    return nwritten; //edw dn tha ftasei pote thewritika
}

ssize_t sendFileToSocketC(int source, int destination, void * buf, size_t buffersize, size_t filesize) { //first read to buf then write in chunks to socket

    ssize_t n = 0;
    ssize_t nwritten = 0;
    int start = 0;

    do {
        if (filesize-nwritten > buffersize) {
            memset(buf, 0, buffersize+1);
            n = read(source, &((char *) buf) [start], buffersize);
        } else {
            memset(buf, 0, buffersize+1);
            n = read(source, &((char *) buf) [start], filesize-nwritten);
        }
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nwritten;
        }
        
        //printf("--------------Read %d bytes and current buffer: %s \n", n , (char*) buf);
        sendResponseToSocketC(destination, buf, n);
        
        nwritten += n;
    } while (nwritten < filesize);

    return nwritten; //edw dn tha ftasei pote thewritika
}


ssize_t writeChunkToFileC(int fd, const void * buf, size_t nbyte) { //chunks of 50 bytes

    ssize_t n = 0;
    ssize_t nwritten = 0;
    


    do {
        if (nbyte-nwritten > CHUNKSIZE) {
            n = write(fd, &((const char *) buf) [nwritten], CHUNKSIZE);
        } else {
            n = write(fd, &((const char *) buf) [nwritten], nbyte-nwritten);
        }
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nwritten;
        }
             
        //printf("--------------Wrote %d bytes and current buffer: %s \n", n , (char*) buf);
        nwritten += n;
    } while (nwritten < nbyte);
    
    

    return nwritten; //edw dn tha ftasei pote thewritika
}


ssize_t writeFileFromSocketC(int source, int destination, void * buf, size_t buffersize, size_t filesize) { //first read to buf then write in chunks to socket

    ssize_t n = 0;
    ssize_t nwritten = 0;
    int start = 0;

    do {
        if (filesize-nwritten > buffersize) {
            memset(buf, 0, buffersize+1);
            n = read(source, &((char *) buf) [start], buffersize);
        } else {
            memset(buf, 0, buffersize+1);
            n = read(source, &((char *) buf) [start], filesize-nwritten);
        }
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        
        if (n == 0) { //EOF
            printf("EOF!! \n");
            return nwritten;
        }
        
        //printf("--------------Read %d bytes and current buffer: %s \n", n , (char*) buf);
        writeChunkToFileC(destination, buf, n); //instead write to file
        
        nwritten += n;
    } while (nwritten < filesize);

    return nwritten; //edw dn tha ftasei pote thewritika
}

int readAll(int fd, char ** buffer, size_t nbyte) {

    int nread = 0;
    int i;
    *buffer = calloc(nbyte, sizeof (char));

    do {
        i = read(fd, buffer[nread], nbyte - nread);
        switch (i) {
            case -1:
                if (errno != EINTR) {
                    continue;
                } else {
                    printf("Demolition Man!!\n");
                    return -1;
                }
            case 0: // EOF
                printf("EOF!\n");
                return nread;
        }
        nread += i;
    } while (nread < nbyte);

    return nread;
}

//ssize_t readRequestFromSocketC(int fd, void * buf,size_t nbyte) {    //chunks of 50 bytes - lathos grammeni logw gamhmenis parenthesis :@ :@ :@
//    
//    ssize_t n = 0;
//    ssize_t nread = 0;
//    
//
//    do {
//        if ((n = read(fd, &((char *) buf) [nread], nbyte-nread) == -1)){
//            if (errno == EINTR)
//                continue;
//            else
//                return -1;    
//        }
//        if (n == 0){  //EOF
//            printf("EOF!! \n");
//            return nread;
//        }
//        
//        printf("current buffer: %s \n", (char*)buf);
//        
//        nread += n;
//        
//        if(nread > 3){
//            if(((char *)buf)[nread-1] == '\n' && ((char *)buf)[nread-2] == '\r' && ((char *)buf)[nread-3] == '\n' && ((char *)buf)[nread-4] == '\r'){
//                printf("reached end condition!! \n");
//                return nread;
//            }
//        }
//
//        
//    } while (nread < BUFSIZE);
//
//    return nread; //edw dn tha ftasei pote thewritika
//}