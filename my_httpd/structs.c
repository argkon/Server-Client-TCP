#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include "structs.h"
#include <string.h>
#include "functions_worker.h"
#include "communication_protocol.h"
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

/*-----------------------------Posting List-----------------------------*/

PListInfoPtr PList_create() {
    PListInfoPtr linfo;
    linfo = malloc(sizeof (PListInfo));
    linfo->size = 0;
    linfo->head = NULL;
    linfo->word = NULL;
    return linfo;
}

void PList_destroy(PListInfoPtr * linfo) {
    PListNodePtr todel, todel2;
    todel = (*linfo)->head;
    while (todel != NULL) {
        todel2 = todel;
        todel = (todel)-> next;
        free(todel2);
    }
    (*linfo)->head = NULL;
    free((*linfo)->word);
    free(*linfo);
    (*linfo) = NULL;
}

void PList_setValue(PListElem* data, PListElem temp) {
    data->docid = temp.docid;
    data->lineid = temp.lineid;
    data->count = temp.count;
}

void PList_getValue(PListInfoPtr const linfo, const PListNodePtr p, PListElem *val, int * const error) { //epistrefei sto val to stoixeio/periexomeno tou ekastote komvou
    *error = 0;
    if (p != NULL)
        PList_setValue(val, p->data);
    else
        *error = 1;
}

void PList_insert(PListInfoPtr * const linfo, PListElem elem, int *error) {
    PListNodePtr temp;
    temp = malloc(sizeof (PListNode));
    if (temp == NULL) {
        *error = 1;
        return;
    }
    PList_setValue(&temp->data, elem);
    temp->next = (*linfo)->head; //vale to neo stoixeio na deixnei sto head tou voithitikou
    (*linfo)->head = temp; //vale ton vohthitiko na deixnei sto neo stoixeio 
    (*linfo)->size++; //au3hse to megethos ths listas

}

int PList_isEqual(PListElem t1, PListElem t2) {
    return ((t1.lineid == t2.lineid) && (t1.docid == t2.docid)); //anazhtame me vasi to path kai tin grammi
}

void PList_find(PListInfoPtr const linfo, PListElem stoixeio, PListNodePtr *prepointer, int *found) {

    PListElem temp;
    PListNodePtr current;
    int error = 0;
    current = linfo->head; //3ekina apo ton voithitiko
    *prepointer = NULL;
    *found = 0;
    while ((!(*found)) && (current != NULL)) {
        PList_getValue(linfo, current, &temp, &error);
        if (PList_isEqual(temp, stoixeio)) { //an to vreis return kai found=1
            *found = 1;
            *prepointer = current;
            return;
        } else { //alliws proxora ston epomeno komvo
            *prepointer = current;
            PList_next(linfo, &current, &error);
            if (error)
                current = NULL;
        }
    }
}

void PList_next(const PListInfoPtr linfo, PListNodePtr * const p, int * const error) {

    *error = 0;
    if ((*p) != NULL) {
        if ((*p)->next != NULL)
            *p = (*p)->next;
        else
            *error = 1;
    } else
        *error = 2;
}

void PList_previous(const PListInfoPtr linfo, PListNodePtr * const p, int * const error) {


    *error = 0;
    if ((*p) != NULL) /*  lista oxi adeia  */ {
        if ((*p) == linfo->head) /* an deixnei ston proto kombo tis listas */
            *error = 1;
        else {
            PListNodePtr temp = linfo->head; //alliws psaxnei apo tin arxi ton proigoumeno
            while (temp->next != *p)
                temp = temp->next;
            *p = temp;
        }
    } else
        *error = 2;
}


/*----------------------------- Arguments -----------------------------*/

ArgsPtr Args_create(StatisticsPtr statistics, QueueInfoPtr queue, char * root_dir) {
    ArgsPtr ainfo;
    ainfo = malloc(sizeof (Args));
    ainfo->statistics = statistics;
    ainfo->queue = queue;
    ainfo->root_dir = root_dir;
    return ainfo;
}

void Args_destroy(ArgsPtr * p) {
    free(*p);
    *p = NULL;
}

/*----------------------------- Response -----------------------------*/

ResponsePtr Response_create() {
    ResponsePtr rinfo;
    rinfo = malloc(sizeof (Response));
    rinfo->code = 0;
    rinfo->content_length = 0;
    rinfo->currtime = NULL;
    return rinfo;
}

ResponsePtr Response_build(const char * filepath) {
    ResponsePtr response = Response_create();

    FILE *f = fopen(filepath, "rb"); //anoi3e to arxeio html kai eleg3e gia lathi

    if (f == NULL) {
        if (errno == ENOENT) {
            printf("file does not exist!!\n");
            response->code = 404;
            strcpy(response->msg, "Couldn't find file!");
        } else if (errno == EACCES) {
            printf("File has no read permissions!!\n");
            response->code = 403;
            strcpy(response->msg, "Couldn't access file!");
        } else {
            printf("Something else went wrong!!Could not open file\n");
            response->code = 498;
            strcpy(response->msg, "yolo hoi bla!");
        }
    } else {
        fseek(f, 0, SEEK_END);
        int fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        printf("read %d bytes from %s \n", fsize, filepath);

        response->code = 200;
        response->content_length = fsize;
        strcpy(response->msg, "");

        fclose(f);
    }

    return response;
}

void Response_send(ResponsePtr response, int newsock) {
    char header[1024];
    const char * description;

    char * atime = NULL;
    get_current_gmt_time(&atime);
    response->currtime = atime;

//    char buffer[1024] = {0};
//    time_t curtime;
//    time(&curtime);
//    struct tm *t = gmtime(&curtime);
//    sprintf(buffer,"Date: %d-%d-%d %02d:%02d:%02d %s\n",t->tm_mday,t->tm_mon,t->tm_year,t->tm_hour,t->tm_min,t->tm_sec,t->tm_zone);
//    response->currtime = buffer;
    
    if (response->code == 200) {
        description = "OK";
    }

    if (response->code == 400) {
        description = "Bad request";
        strcpy(response->msg, "Please read the manual!");
    }

    if (response->code == 403) {
        description = "Forbidden";
    }

    if (response->code == 404) {
        description = "Not found";
    }

    if (response->code == 400 || response->code == 403 || response->code == 404) {
        response->content_length = strlen(response->msg);

        sprintf(header, "HTTP/1.1 %d %s\r\nDate: %s\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: %ld\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n%s", response->code, description, response->currtime, response->content_length, response->msg);
    } else {
        sprintf(header, "HTTP/1.1 %d %s\r\nDate: %s\r\nServer: myhttpd/1.0.0 (Ubuntu64)\r\nContent-Length: %ld\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n", response->code, description, response->currtime, response->content_length);
    }

    sendResponseToSocketC(newsock, header, strlen(header));
}

void Response_destroy(ResponsePtr * p) {
    free(*p);
    *p = NULL;
}

/*----------------------------- Stats -----------------------------*/

StatisticsPtr STATISTICS_create() {
    StatisticsPtr sinfo;
    sinfo = malloc(sizeof (Statistics));
    sinfo->totaltime = 0;
    sinfo->numpages = 0;
    sinfo->totalbytes = 0;

    pthread_mutex_init(&sinfo->mtx, 0);
    return sinfo;
}

void STATISTICS_destroy(StatisticsPtr * p) {
    pthread_mutex_destroy(&((*p)->mtx));

    free(*p);
    *p = NULL;
}

void STATISTICS_print(StatisticsPtr stats) {
    int min = 0;
    int sec = 0;
    int milli = (stats->totaltime) / 1000;
    min = milli / 60000;
    milli = milli - min * 60000;
    sec = milli / 1000;
    milli = milli - sec * 1000;
    printf(" Statistics: \n");
    printf("    Server up for : %02d:%02d.%d \n", min, sec, milli); //anagnwseis
    printf("    Pages Served  : %d \n", stats->numpages); //eggrafes
    printf("    Bytes    : %d \n", stats->totalbytes); //plaisia mnimis
}

void STATISTICS_send(StatisticsPtr stats, int fd) {
    int min = 0;
    int sec = 0;
    int milli = (stats->totaltime) / 1000;
    min = milli / 60000;
    milli = milli - min * 60000;
    sec = milli / 1000;
    milli = milli - sec * 1000;
    
    
    char buffer[3000];
    sprintf(buffer, "Statistics: \nServer up for : %02d:%02d.%d \n  Pages Served  : %d \n  Bytes    : %d \n", min, sec, milli,  stats->numpages,  stats->totalbytes); //anagnwseis
    
    sendResponseToSocketC(fd, buffer, strlen(buffer));
}

/*----------------------------- Map -----------------------------*/


MapInfoPtr Map_create(int size) {
    MapInfoPtr minfo = malloc(sizeof (MapInfo));
    minfo->size = size;
    minfo->array = malloc(sizeof (Map) * size);
    for (int i = 0; i < size; i++) {
        minfo->array[i].charcount = 0;
        minfo->array[i].linecount = 0;
        minfo->array[i].wordcount = 0;
        minfo->array[i].lines = NULL;
        minfo->array[i].filepath = NULL;
    }
    return minfo;
}

int Map_insert_document(MapInfoPtr minfo, int index, const char * filepath) { //pairnei ena path kai eisagei ola ta arxeia pou uparxoun sto path sto map
    int j = index;
    char * line = NULL;
    ssize_t read;
    size_t length = 0;
    int filelines = 0;


    DIR *d = NULL;
    struct dirent *dir = NULL;
    d = opendir(filepath);

    // store to variable 
    char cwd[PATH_MAX];
    char *err = getcwd(cwd, sizeof (cwd));

    if (err != NULL) {
        printf("Current working directory is %s \n", cwd);
    } else {
        printf("failed to find current working directory name! \n");
        return 0;
    }

    if (d) { //if directory exists
        int ch = chdir(filepath); //change directory to open files
        if (!ch) {
            printf("changed directory to: %s \n", filepath);
        }
        while ((dir = readdir(d)) != NULL) {
            filelines = 0;
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
                //nothing
            } else {

                minfo->array[j].filepath = (char *) malloc(strlen(filepath) + strlen(dir->d_name) + 2);
                memset(minfo->array[j].filepath, 0, (strlen(filepath) + strlen(dir->d_name) + 2));

                strcpy(minfo->array[j].filepath, filepath);
                strcat(minfo->array[j].filepath, "/");
                strcat(minfo->array[j].filepath, dir->d_name);

                printf("Entering ... folder %s \n", minfo->array[j].filepath);
                printf("indexing ... file %s \n", dir->d_name);


                FILE * F1 = fopen(dir->d_name, "r");

                if (F1 == NULL) {
                    printf("error in file \n");
                    return -1;
                }
                while (((read = getline(&line, &length, F1)) != -1)) { //count the lines of the file
                    filelines++;
                }
                printf("filelines are: %d \n", filelines);
                printf("map index is: %d \n", j);
                minfo->array[j].linecount = filelines;

                fclose(F1);

                FILE * F2 = fopen(dir->d_name, "r");

                minfo->array[j].lines = malloc(sizeof (char*)*(minfo->array[j].linecount));
                int i = 0;
                while (((read = getline(&line, &length, F2)) != -1)) { //assign the text of the line to the map
                    if (strlen(line) == 1) {
                        printf("spotted a blank line\n");
                        minfo->array[j].lines[i] = malloc(sizeof (char) + 1);
                        strcpy(minfo->array[j].lines[i], "\0");
                        i++;
                    } else {
                        minfo->array[j].lines[i] = getLine(line);
                        i++;
                    }
                }

                fclose(F2);
                j++;
            }
        }
        free(line);
        closedir(d);
    } else {
        printf("didnt find folder: \"%s\" \n", filepath);
    }

    // restore directory to ... variable ...
    chdir(cwd); //return to project folder

    return j;

}

void Map_destroy(MapInfoPtr minfo, int size) {


    for (int i = 0; i < size; i++) { // for each doc
        //printf("cleaning map document %d \n", i);
        if (minfo->array[i].lines != NULL) {
            for (int j = 0; j < minfo->array[i].linecount; j++) { // for each line of the doc
                if (minfo->array[i].lines[j] != NULL) {
                    free(minfo->array[i].lines[j]);
                    //printf("cleaning map document line %d \n", j);
                }
            }

            free(minfo->array[i].lines);
            free(minfo->array[i].filepath);
        }

    }
    free(minfo->array);
    free(minfo);
}

/*----------------------------- Trie -----------------------------*/

void Trie_create(TrieNodePtr * root) {
    *root = Trie_node_create(' ');
}

TrieNodePtr Trie_node_create(char key) {

    TrieNodePtr node = NULL;

    node = (TrieNodePtr) malloc(sizeof (TrieNode));

    if (node == NULL) {
        printf("Error in trie node allocation\n");
        exit(0);
        return 0;
    }

    node->children = NULL;
    node->next = NULL;
    node->key = key;
    node->plist = NULL;

    return node;
}

void Trie_insert(TrieNodePtr *proot, char* word, char* original, int docid, int lineid) {
    int wordLength = strlen(original);

    if ((*proot) == NULL) {
        printf("Tree not created!! \n");
        exit(0);
        return;
    }

    TrieNodePtr traversal = NULL;

    traversal = (*proot)->children;

    if (traversal == NULL) { // 1st letter, 1st word
        for (traversal = *proot; *word != '\0'; traversal = traversal->children) {
            traversal->children = Trie_node_create(*word);
            //            traversal->children->parent = traversal;
            word++;
        }

        PListInfoPtr pl = PList_create();
        pl->word = malloc((wordLength + 1) * sizeof (char));
        memset(pl->word, 0, (wordLength + 1));
        strncpy(pl->word, original, wordLength);
        //        printf("INSERTING WORD %s \n",pl->word);
        PListElem temp;
        temp.lineid = lineid;
        temp.docid = docid;
        temp.count = 1;
        int error = 0;
        PList_insert((&pl), temp, &error);

        if (error == 1) {
            printf("error: insert in pl list failed \n");
            exit(0);
        }
        traversal->plist = pl;
        return;
    }

    TrieNodePtr tmptrav = Trie_find(*proot, word);

    if (tmptrav != NULL) { // word already exists in the trie
        int found = 0;
        PListNodePtr preptr = NULL;
        PListElem elem;
        elem.docid = docid;
        elem.lineid = lineid;
        PList_find((tmptrav->plist), elem, &preptr, &found);

        if (found == 1) { // an vreis komvo ths posting list me idio path kai grammi au3ise to count alliws prosthese komvo gia to neo document
            preptr->data.count++;
            //            printf("count of word %d \n", preptr->data.count);
        } else {
            PListElem temp;
            temp.docid = docid;
            temp.lineid = lineid;
            temp.count = 1;
            int error = 0;
            PList_insert(&(tmptrav->plist), temp, &error);

            if (error == 1) {
                printf("error: insert in pl list failed \n");
                exit(0);
            }
        }

        return;
    }

    while (*word != '\0') { // while word letters are equal continue else break in the different letter
        if (*word == traversal->key) {
            word++;

            if ((*word) != '\0') {
                if ((traversal->children) != NULL) {
                    // move downwards
                    traversal = traversal->children;
                } else {
                    // extend a word
                    Trie_insert(&(traversal), word, original, docid, lineid);
                    return;
                }
            }
            // add a shorter word
            if ((*word) == '\0') {
                break;
            }
        } else
            break;
    }
    if ((*word) != '\0') {
        // search horizontally, stop at last node if letter not found
        while (traversal->next) {
            if (*word == traversal->next->key) {
                word++;
                if ((*word) != '\0') { //if word has more letters recurse
                    Trie_insert(&(traversal->next), word, original, docid, lineid);
                } else { //else if word has no more letters create posting list in next node
                    traversal = traversal -> next;
                    PListInfoPtr pl = PList_create();
                    pl->word = malloc((wordLength + 1) * sizeof (char));
                    memset(pl->word, 0, (wordLength + 1));
                    strncpy(pl->word, original, wordLength);
                    //                    printf("INSERTING WORD %s \n", pl->word);
                    PListElem temp;
                    temp.lineid = lineid;
                    temp.docid = docid;
                    temp.count = 1;
                    int error = 0;
                    PList_insert((&pl), temp, &error);

                    if (error == 1) {
                        printf("error: insert in pl list failed \n");
                        exit(0);
                    }

                    traversal->plist = pl;
                }
                return;
            }
            traversal = traversal->next;
        }
        // we reached rightmost node, we need to create next node and insert word downwards
        traversal->next = Trie_node_create(*word);
        //            traversal->next->parent = traversal->parent;
        //            traversal->next->prev = traversal;
        word++;
        for (traversal = traversal->next; *word != '\0'; traversal = traversal->children) {
            traversal->children = Trie_node_create(*word);
            //                traversal->children->parent = traversal;
            word++;
        }
    }

    PListInfoPtr pl = PList_create();
    pl->word = malloc((wordLength + 1) * sizeof (char));
    memset(pl->word, 0, (wordLength + 1));
    strncpy(pl->word, original, wordLength);
    //    printf("INSERTING WORD %s \n", pl->word);
    PListElem temp;
    temp.lineid = lineid;
    temp.docid = docid;
    temp.count = 1;
    int error = 0;
    PList_insert((&pl), temp, &error);

    if (error == 1) {
        printf("error: insert in pl list failed \n");
        exit(0);
    }

    traversal->plist = pl;
}

TrieNodePtr Trie_find(TrieNodePtr root, char* word) {

    TrieNodePtr level = root->children;
    while (1) {
        TrieNodePtr current;
        // search for letter horizontally
        for (current = level; current != NULL; current = current->next) {
            if ((current -> key) == *word) {
                break;
            }
        }
        if (current == NULL) {
            return NULL;
        }
        word++;
        if ((*word) == '\0') {
            if ((current->plist) != NULL) {
                return current;
            } else {
                return NULL;
            }
        }
        level = current -> children;
    }
    return NULL;
}

void Trie_destroy(TrieNodePtr *root) { //postorder recursive deletion of trie

    TrieNodePtr trav = *root;
    TrieNodePtr temp = *root;
    if (trav == NULL) {
        return;
    }

    Trie_destroy(&(trav->children));

    Trie_destroy(&(trav->next));

    if ((trav->plist) != NULL)
        PList_destroy(&(trav->plist));

    temp = trav;

    free(temp);
}

void Trie_traverse(MapInfoPtr map, TrieNodePtr root, int *count) { //postorder recursive traversal of trie
    TrieNodePtr trav = root;
    if (trav == NULL) {
        return;
    }

    Trie_traverse(map, trav->children, count);

    Trie_traverse(map, trav->next, count);

    if ((trav->plist) != NULL) {
        PListNodePtr pnode = trav->plist->head;

        int error = 0;

        while (pnode != NULL && error == 0) {
            printf("%s: %s docid:%d line:%d count:%d \n", root->plist->word, map->array[pnode->data.docid].filepath, pnode->data.docid, pnode->data.lineid, pnode->data.count);

            PList_next(trav->plist, &pnode, &error);
        }

        (*count)++;
    }
}


