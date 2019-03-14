#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "functions_worker.h"
#include "functions_httpd.h"
#include <math.h>
#include "countlist.h"
#include "searchlist.h"
#include "communication_protocol.h"
#include "templist.h"
#include <dirent.h>
#include <time.h>

// TODO: GMT time not local time
void get_current_gmt_time(char ** atime) {
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    *atime = asctime(timeinfo);

    *(strchr(*atime, '\n')) = '\0';
}

// https://stackoverflow.com/questions/29788983/split-char-string-with-multi-character-delimiter-in-c

char * multi_tok(char *input, char ** string, char *delimiter) {
    if (input != NULL)
        *string = input;

    if (*string == NULL)
        return *string;

    char *end = strstr(*string, delimiter);
    if (end == NULL) {
        char *temp = *string;
        *string = NULL;
        return temp;
    }

    char *temp = *string;

    *end = '\0';
    *string = end + strlen(delimiter);
    return temp;
}

int requestValidation(char * msg) {
    int valid = 1;
    int msgLength = strlen(msg);
    char * msgCopy = (char*) malloc((msgLength + 1) * sizeof (char));
    memset(msgCopy, 0, (msgLength + 1));
    strncpy(msgCopy, msg, msgLength);
    printf("message length:%d", msgLength);
    printf("\nUnedited message:%s\n", msgCopy);


    FILE * log_fp;
    log_fp = fopen("logtemp", "at+");
    fprintf(log_fp, "%s", msgCopy);
    fclose(log_fp);

    char * save_ptr = NULL;


    char * w = strtok_r(msgCopy, " \t\n", &save_ptr); //first word must be GET
    printf("current word:---%s--- \n", w);
    if ((strcmp(w, "GET") != 0)) {
        printf("No GET!! \n");
        valid = 0;
    }
    w = strtok_r(NULL, " \t\n", &save_ptr); //second word must exist
    printf("current word:---%s--- \n", w);
    //printf("rest:---%s--- \n", save_ptr);
    if ((w == NULL)) {
        printf("No URL!! \n");
        valid = 0;
    }
    w = multi_tok(NULL, &save_ptr, "\r\n"); //third word must be last and HTTP/1.1 TODO: DEN LEITOURGEI LOGW /r/n thelei multi_tok
    printf("current word:---%s---  \n", w);
    if ((strcmp(w, "HTTP/1.1") != 0)) {
        printf("No conclusion HTTP/1.1 !! \n");
        valid = 0;
    }
    int foundhost = 0;
    int hashost = 0;
    while ((w = strtok_r(NULL, " \t\r\n", &save_ptr)) != NULL) {
        printf("Word:---%s---\n", w);
        if (strcmp(w, "Host:") == 0) {
            foundhost = 1;
        }
        if (foundhost == 1) {
            if ((w = multi_tok(NULL, &save_ptr, " \r\n")) != NULL) {
                hashost = 1;
            }
        }
    }

    if (foundhost == 1 && hashost == 1 && valid == 1) {
        valid = 1;
    } else
        valid = 0;

    free(msgCopy);
    return valid;
}



int worker_parseCommand(char * line, char **commands) {
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

        while ((p1 = strtok(NULL, " \t\n")) != NULL && j < 11) {
            strcpy(commands[j], p1);
            j++;
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

void parseWords(char * line, TrieNodePtr Trie, int docid, int lineid, int *wordcount, int *charcount) {
    //    printf("received line: %s \n",line);

    int lineLength = strlen(line);
    (*charcount) += strlen(line);
    char *lineCopy = (char*) malloc((lineLength + 1) * sizeof (char));
    char *p1;

    memset(lineCopy, 0, (lineLength + 1));

    strncpy(lineCopy, line, lineLength);


    p1 = strtok(lineCopy, " \t\n");
    //printf("Word: %s \n", p1);
    Trie_insert((&Trie), p1, p1, docid, lineid);
    (*wordcount)++;


    if (p1 == NULL) {
        printf("error in allocation \n");
    }


    while ((p1 = strtok(NULL, " \t\n")) != NULL) {
        //printf("Word: %s \n", p1);
        Trie_insert((&Trie), p1, p1, docid, lineid);
        (*wordcount)++;
    }


    free(lineCopy);

}

char * getLine(char * line) {

    int lineLength = strlen(line);
    char *lineCopy = (char*) malloc((lineLength + 1) * sizeof (char));
    char * trimline = malloc((lineLength + 1) * sizeof (char));
    memset(lineCopy, 0, (lineLength + 1));
    strncpy(lineCopy, line, lineLength);

    char * p = strtok(lineCopy, "\n");

    if (p == NULL || lineCopy == NULL) {
        printf("Error in allocation \n");
        exit(0);
        return NULL;
    }


    int pathlength = strlen(p);
    memset(trimline, 0, pathlength + 1);
    strncpy(trimline, p, pathlength);

    free(lineCopy);
    return trimline;
}

void df(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount) {
    if (wordcount == 1) {
        int totalwords = 0;
        Trie_traverse(map, root, &totalwords);

    } else if (wordcount == 2) {
        char * word = commands[1];
        TrieNodePtr current = Trie_find(root, word);
        if (current == NULL) {
            printf("Word not found \n");
        } else {
            printf("%s %d \n", word, current->plist->size);
        }
    }

}

void search(int descriptors[3], MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount, FILE * log_fp, int *nstrings) {
    int i;
    SListInfoPtr slinfo = SList_create();
    TListInfoPtr tlinfo = TList_create();
    for (i = 1; i < wordcount; i++) { //gia kathe query
        TrieNodePtr current = Trie_find(root, commands[i]);
        if (current != NULL && (current->plist) != NULL) {
            PListInfoPtr plist = current->plist; //ftiaxnoume komvo gia diasxisi ths posting list
            PListNodePtr pnode = plist->head;
            int error = 0;
            while (pnode != NULL && error == 0) {
                SListElem selem;
                TListElem telem;

                selem.docid = pnode->data.docid;
                selem.lineid = pnode->data.lineid;
                telem.docid = pnode->data.docid;
                SListNodePtr foundNode = NULL;
                TListNodePtr foundNode2 = NULL;
                int found = 0;
                int found2 = 0;
                SList_find(slinfo, selem, &foundNode, &found);
                TList_find(tlinfo, telem, &foundNode2, &found2);

                // ### add to another list, remove duplicate lines

                if (found == 1) { //eisagwgi se lista me vasi kai document id kai diaforetiki line id
                    //nothing
                } else {
                    //printf("inserting in search list \n");
                    SList_insert(&slinfo, selem, &error);
                    if (error == 1) {
                        printf("Error in Search List Insertion\n");
                        exit(0);
                    }
                    (*nstrings)++;
                }
                if (found2 == 1) { //eisagwgi se alli list me vasi mono document id gia katagrafi sto log kai afairesi duplicate pathname
                    //nothing
                } else {
                    TList_insert(&tlinfo, telem, &error);
                    if (error == 1) {
                        printf("Error in Temporary List Insertion\n");
                        exit(0);
                    }
                }

                PList_next(plist, &pnode, &error);
            }
        } else {
            printf("Query Word Not found in any document! \n");
        }
    }
    printf("Total found query words in different lines are : %d \n", slinfo->size);
    int j;
    int error = 0;
    SListNodePtr snode = slinfo->head;
    TListNodePtr tnode = tlinfo->head;
    printf("WORKER %d:NUMBER OF RESULTS %d\n", descriptors[0], slinfo->size);
    if (sendNumber(descriptors[2], (slinfo->size)) != COMMUNICATION_SUCCESSFUL) { //steile poses apantiseis na perimenei o exec
        printf("Worker : read/write error \n");
    }

    char * atime = NULL;
    get_current_gmt_time(&atime);
    fprintf(log_fp, "%s:search:", atime);

    for (i = 1; i < wordcount; i++) { //gia kathe query
        fprintf(log_fp, "%s", commands[i]);
        if (i < wordcount - 1) {
            fprintf(log_fp, " ");
        }
    }

    for (j = 0; j < (slinfo->size); j++) { // send to httpd
        //printf("document:%s line:%d text:%s \n#######################################################\n", map->array[snode->data.docid].filepath, snode->data.lineid, map->array[snode->data.docid].lines[snode->data.lineid]); //anti na ta ektupwnei na ta stelnei ston httpd

        char *buffer = NULL;
        int totallength = strlen(map->array[snode->data.docid].filepath) + 30 + strlen(map->array[snode->data.docid].lines[snode->data.lineid]) + 3; //desmeusi xwrou gia to path line id kai to text ths grammis
        buffer = malloc(sizeof (char)*totallength);
        memset(buffer, 0, totallength);
        sprintf(buffer, "%s %d %s\n", map->array[snode->data.docid].filepath, snode->data.lineid, map->array[snode->data.docid].lines[snode->data.lineid]);

        if (sendString(descriptors[2], buffer) != COMMUNICATION_SUCCESSFUL) { //steile to apotelesmata ston exec
            printf("Worker : read/write error \n");
        }
        SList_next(slinfo, &snode, &error);
        free(buffer);
    }
    if (tlinfo->size == 0) {
        fprintf(log_fp, ":NOT FOUND");
    }
    for (j = 0; j < (tlinfo->size); j++) { //grapse sto log
        fprintf(log_fp, ":%s", map->array[tnode->data.docid].filepath);
        TList_next(tlinfo, &tnode, &error);
    }

    fprintf(log_fp, "\n");

    if (sendNumber(descriptors[2], -1) != COMMUNICATION_SUCCESSFUL) { //steile -1 gia epishmansi telous
        printf("Worker : read/write error \n");
    }
    SList_destroy(&slinfo);
    TList_destroy(&tlinfo);
}

void maxcount(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount, int *max, int *maxid) {
    int i;
    char * word = commands[1];
    CListInfoPtr clinfo = CList_create();
    TrieNodePtr current = Trie_find(root, word);
    if (current != NULL && (current->plist) != NULL) {
        PListInfoPtr plist = current->plist;
        PListNodePtr pnode = plist->head;
        int error = 0;

        while (pnode != NULL && error == 0) {
            CListElem elem;
            elem.id = pnode->data.docid;
            elem.count = pnode->data.count;

            CListNodePtr foundNode = NULL;
            int found = 0;

            CList_find(clinfo, elem, &foundNode, &found);

            if (found == 1) {
                foundNode->data.count = foundNode->data.count + elem.count;
            } else {
                CList_insert(&clinfo, elem, &error);
                if (error == 1) {
                    printf("Problem in countlist insertion!! exiting.. \n");
                    exit(0);
                }
            }
            PList_next(plist, &pnode, &error);
        }
    } else {
        printf("Word was not found in any document!! \n");
        CList_destroy(&clinfo);
        return;
    }

    CListNodePtr cnode = clinfo->head;
    int error = 0;

    *max = cnode->data.count;
    *maxid = cnode->data.id;

    if (cnode == NULL) {
        printf("count list not created.exiting.. \n");
        exit(0);
    }

    for (i = 0; i < (clinfo->size) - 1; i++) { //nik tsekare to
        CList_next(clinfo, &cnode, &error);
        if (error == 1) {
            printf("list ended! \n");
        }
        if (cnode->data.count > *max) {
            *max = cnode->data.count;
            *maxid = cnode->data.id;
        } else if ((cnode->data.count) == *max) { //sugkrine ta filepaths poio einai megalutero
            int currpathsize = strlen(map->array[cnode->data.id].filepath);
            int prevpathsize = strlen(map->array[*maxid].filepath);
            if (currpathsize > prevpathsize) {
                *maxid = cnode->data.id;
                *max = cnode->data.count;
            }
        }
    }

    if (error == 1) {
        printf("list ended! \n");
    }


    CList_destroy(&clinfo);
}

void mincount(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount, int *min, int *minid) {
    int i;
    char * word = commands[1];
    CListInfoPtr clinfo = CList_create();
    TrieNodePtr current = Trie_find(root, word);
    if (current != NULL && (current->plist) != NULL) {
        PListInfoPtr plist = current->plist;
        PListNodePtr pnode = plist->head;
        int error = 0;

        while (pnode != NULL && error == 0) {
            CListElem elem;
            elem.id = pnode->data.docid;
            elem.count = pnode->data.count;

            CListNodePtr foundNode = NULL;
            int found = 0;

            CList_find(clinfo, elem, &foundNode, &found);

            if (found == 1) {
                foundNode->data.count = foundNode->data.count + elem.count;
            } else {
                CList_insert(&clinfo, elem, &error);
                //printf("Inserted document %s with pathsize %lu \n",map->array[elem.id].filepath,strlen(map->array[elem.id].filepath));
                if (error == 1) {
                    printf("Problem in countlist insertion!! exiting.. \n");
                    exit(0);
                }
            }
            PList_next(plist, &pnode, &error);
        }
    } else {
        printf("Word was not found in any document!! \n");
        CList_destroy(&clinfo);
        return;
    }

    CListNodePtr cnode = clinfo->head;
    //printf("current node file %s \n",map->array[cnode->data.id].filepath);
    int error = 0;

    *min = cnode->data.count;
    *minid = cnode->data.id;

    if (cnode == NULL) {
        printf("count list not created.exiting.. \n");
        exit(0);
    }

    for (i = 0; i < (clinfo->size) - 1; i++) { //nik tsekare to

        CList_next(clinfo, &cnode, &error);
        //printf("current node file %s \n",map->array[cnode->data.id].filepath);
        int currpathsize = strlen(map->array[cnode->data.id].filepath);
        int prevpathsize = strlen(map->array[*minid].filepath);
        if (error == 1) {
            printf("list ended! \n");
        }
        if (cnode->data.count < *min) {
            *min = cnode->data.count;
            *minid = cnode->data.id;
        } else if ((cnode->data.count) == *min) { //sugkrine ta filepaths poio einai megalutero
            //printf(" ISOBATHMIA \n");

            printf("Current pathsize: %d and Previous pathsize: %d \n", currpathsize, prevpathsize);
            if (currpathsize < prevpathsize) {
                *minid = cnode->data.id;
                *min = cnode->data.count;
            }
        }
    }

    if (error == 1) {
        printf("list ended! \n");
    }


    CList_destroy(&clinfo);
}

void worker_commands(int descriptors[3], MapInfoPtr map, TrieNodePtr trie, FILE * log_fp) {
    char** commands = malloc(COMMAND_LIMIT * sizeof (char*));
    for (int i = 0; i < COMMAND_LIMIT; i++) {
        commands[i] = malloc(COMMAND_SIZE * sizeof (char));
    }

    char *c = NULL;
    int nstrings = 0;

    while (1) { // read from PIPE
        //int i = 0;


        if (readString(descriptors[1], &c) != COMMUNICATION_SUCCESSFUL) {
            printf("Worker: read/write error \n");
            exit(0);
        }

        //printf("worker received: \"%s\" \n", c);

        //        sleep(5); //gia testing me to deadline

        int cmdcount = worker_parseCommand(c, commands);

        free(c); //katharizw tin mnimi pou desmeusa me to c 
        //printf("cmdcount = %d \n", cmdcount);
        //        for (i = 0; i < cmdcount; i++) { //print contents of string array commands
        //            printf("element of commands: %s \n", commands[i]);
        //        }

        if (strcmp(commands[0], "/search") == 0 && cmdcount > 1) {
            // call search
            search(descriptors, map, trie, commands, cmdcount, log_fp, &nstrings);
            // retrieve result as
            // print result
            // (or send to pipe) (LATER)


        } else if (strcmp(commands[0], "/maxcount") == 0 && cmdcount == 2) {
            int max = -1;
            int maxid = -1;
            maxcount(map, trie, commands, cmdcount, &max, &maxid);
            if (maxid >= 0) {
                printf("Worker %d: Document with maxcount of word %s is %s and count is:%d \n", descriptors[0], commands[1], map->array[maxid].filepath, max);

                char * atime = NULL;
                get_current_gmt_time(&atime);
                fprintf(log_fp, "%s:maxcount:%s:%s \n", atime, commands[1], map->array[maxid].filepath);

                if (sendNumber(descriptors[2], max) != COMMUNICATION_SUCCESSFUL) {
                    printf("Worker : read/write error \n");
                }
                if (sendString(descriptors[2], map->array[maxid].filepath) != COMMUNICATION_SUCCESSFUL) { //steile to apotelesma path kai max number
                    printf("Worker : read/write error \n");
                }
            } else {

                char * atime = NULL;
                get_current_gmt_time(&atime);
                fprintf(log_fp, "%s:maxcount:%s:NOT FOUND \n", atime, commands[1]);

                if (sendNumber(descriptors[2], max) != COMMUNICATION_SUCCESSFUL) {
                    printf("Worker : read/write error \n");
                }
            }
        } else if (strcmp(commands[0], "/mincount") == 0 && cmdcount == 2) {
            int min = -1;
            int minid = -1;
            mincount(map, trie, commands, cmdcount, &min, &minid);
            if (minid >= 0) {
                printf("Worker %d: Document with mincount of word %s is %s and count is:%d \n", descriptors[0], commands[1], map->array[minid].filepath, min);

                char * atime = NULL;
                get_current_gmt_time(&atime);
                fprintf(log_fp, "%s:mincount:%s:%s \n", atime, commands[1], map->array[minid].filepath);

                if (sendNumber(descriptors[2], min) != COMMUNICATION_SUCCESSFUL) {
                    printf("Worker : read/write error \n");
                }
                if (sendString(descriptors[2], map->array[minid].filepath) != COMMUNICATION_SUCCESSFUL) { //steile to apotelesma path kai max number
                    printf("Worker : read/write error \n");
                }
            } else {

                char * atime = NULL;
                get_current_gmt_time(&atime);
                fprintf(log_fp, "%s:mincount:%s:NOT FOUND \n", atime, commands[1]);

                if (sendNumber(descriptors[2], min) != COMMUNICATION_SUCCESSFUL) {
                    printf("Worker : read/write error \n");
                }
            }
        } else if ((strcmp(commands[0], "/wc\n") == 0) && (cmdcount == 1)) {
            int i;

            if (sendNumber(descriptors[2], map->size) != COMMUNICATION_SUCCESSFUL) {
                printf("Worker: read/write error \n");
                exit(0);
            }

            int x = 0;
            int y = 0;
            int z = 0;

            char * buffer = malloc(100);
            for (i = 0; i < map->size; i++) {
                //sprintf(buffer, "document %s: linecount %d, wordcount %d, charcount %d \n", map->array[i].filepath, map->array[i].linecount, map->array[i].wordcount, map->array[i].charcount);

                x = x + map->array[i].linecount;
                y = y + map->array[i].wordcount;
                z = z + map->array[i].charcount;

                sprintf(buffer, "%d %d %d\n", map->array[i].linecount, map->array[i].wordcount, map->array[i].charcount);
                if (sendString(descriptors[2], buffer) != COMMUNICATION_SUCCESSFUL) {
                    printf("Worker : read/write error \n");
                }
            }

            char * atime = NULL;
            get_current_gmt_time(&atime);
            fprintf(log_fp, "%s:wc:%d %d %d\n", atime, x, y, z);

            free(buffer);
        } else if (strcmp(commands[0], "/exit\n") == 0) {
            printf("Worker Terminating\n");
            //printf("Number of strings found in files: %d\n", nstrings); //instead of print send to httpd
            if (sendNumber(descriptors[2], nstrings) != COMMUNICATION_SUCCESSFUL) { //enhmerwse tin job httpd gia to plithos strings pou vrikes sta arxeia s
                printf("Worker : read/write error \n");
            }
            //free(c);
            for (int i = 0; i < COMMAND_LIMIT; i++)
                free(commands[i]);
            free(commands);
            break; //vges kai katharise tis domes
        } else {
            printf("----------Invalid command!!----------- \n Available commands are: \n\n 1./search q1 ... q10 \n 2./maxcount keyword \n 3./mincount keyword \n 4./wc \n 5./exit \n\n");
        }
    }

}

int count_files(const char ** paths, int npaths) { //metra ta arxeia mesa sta paths
    // count all files in paths
    int count = 0;
    int i;
    for (i = 0; i < npaths; i++) {
        DIR *d = NULL;
        struct dirent *dir = NULL;
        d = opendir(paths[i]);
        if (d) {
            //printf("entering directory %s \n",paths[i]);
            while ((dir = readdir(d)) != NULL) {
                if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {

                } else {
                    //printf("found file!! \n");
                    count++;
                }
            }
        }
        closedir(d);
    }

    return count;

}

//void tf(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount) {
//
//    int id = atoi(commands[1]);
//    char * word = commands[2];
//
//    TrieNodePtr current = Trie_find(root, word);
//
//    if (current == NULL) {
//        printf("Word not found \n");
//    } else {
//        PListNodePtr ptr = NULL;
//        int found = 0;
//        PListElem elem;
//        elem.id = id;
//
//        PList_find(current->plist, elem, &ptr, &found);
//
//        if (found == 0) {
//            printf("Document does not contain word \n");
//        } else {
//            printf("%d %s %d \n", id, word, ptr->data.count);
//        }
//    }
//
//}

//void debugmode(MapInfoPtr map, TrieNodePtr trie) {
//    char** commands = malloc(COMMAND_LIMIT * sizeof (char*));
//    for (int i = 0; i < COMMAND_LIMIT; i++) {
//        commands[i] = malloc(COMMAND_SIZE * sizeof (char));
//    }
//
//    char *c = malloc(COMMAND_SIZE * COMMAND_LIMIT);
//
//
//    while (1) { //read commands from terminal
//        int i = 0;
//
//        printf("(WORKER DEBUG) enter command:");
//
//        // instead of fgets, read from PIPE (LATER)
//        fgets(c, (COMMAND_SIZE) * COMMAND_LIMIT, stdin);
//
//        int x;
//        int cmdcount = httpd_parseCommand(c, commands, &x);
//
//        printf("cmdcount = %d \n", cmdcount);
//
//
//        for (i = 0; i < cmdcount; i++) { //print contents of string array commands
//            printf("element of commands: %s \n", commands[i]);
//        }
//
//        if (strcmp(commands[0], "/search") == 0 && cmdcount > 1) {
//            // call search
//            // retrieve result as
//            // print result
//            // (or send to pipe) (LATER)
//
//
//        } else if (strcmp(commands[0], "/maxcount") == 0 && cmdcount == 2) {
//            int max = -1;
//            int maxid = -1;
//            maxcount(map, trie, commands, cmdcount, &max, &maxid);
//            if (maxid >= 0) {
//                printf("Document with maxcount of word %s is %s and count is:%d \n", commands[1], map->array[maxid].filepath, max);
//            }
//        } else if (strcmp(commands[0], "/mincount") == 0 && cmdcount == 2) {
//            int min = -1;
//            int minid = -1;
//            mincount(map, trie, commands, cmdcount, &min, &minid);
//            if (minid >= 0) {
//                printf("Document with mincount of word %s is %s and count is:%d \n", commands[1], map->array[minid].filepath, min);
//            }
//        } else if ((strcmp(commands[0], "/wc\n") == 0) && (cmdcount == 1)) {
//            int i;
//            for (i = 0; i < map->size; i++) {
//                printf("document %s: linecount %d, wordcount %d, charcount %d \n", map->array[i].filepath, map->array[i].linecount, map->array[i].wordcount, map->array[i].charcount);
//            }
//        } else if (strcmp(commands[0], "/exit\n") == 0) {
//            printf("Terminating\n");
//            free(c);
//            for (int i = 0; i < COMMAND_LIMIT; i++)
//                free(commands[i]);
//            free(commands);
//            break;
//        } else {
//            printf("----------Invalid command!!----------- \n Available commands are: \n\n 1./search q1 ... q10 \n 2./maxcount keyword \n 3./mincount keyword \n 4./wc \n 5./exit \n\n");
//        }
//    }
//
//}