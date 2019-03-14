#ifndef STRUCTS_H
#define STRUCTS_H


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#include "queue.h"
#include "templist.h"

#define MAX_ID_LENGTH 5

typedef struct TrieNode * TrieNodePtr;
typedef struct PListNode * PListNodePtr;
typedef struct PListInfo * PListInfoPtr;
typedef struct Map * MapPtr;
typedef struct MapInfo * MapInfoPtr;
typedef struct Statistics * StatisticsPtr;
typedef struct Response * ResponsePtr;
typedef struct Request * RequestPtr;
typedef struct Args * ArgsPtr;

/* ---------------------------- Trie ------------------------ */

typedef struct TrieNode {
    char key;
    TrieNodePtr next;
    TrieNodePtr children;
    PListInfoPtr plist;
} TrieNode;

/* ---------------------------- Posting List ------------------------ */
typedef struct PListInfo {
    char* word;
    int size;
    PListNodePtr head;
} PListInfo;

typedef struct PListElem {
    int docid; // file id
    int lineid; // line id
    int count; // frequency (how many times word exists in specific line)
} PListElem;

typedef struct PListNode {
    PListElem data;
    PListNodePtr next;
} PListNode;

/* ---------------------------- Map ------------------------ */

typedef struct Map {
    char * filepath; // paths
    char** lines; // lines of a document
    int charcount; // total chars
    int wordcount; // total words
    int linecount; // total lines
} Map;

typedef struct MapInfo {
    int size;
    MapPtr array;
} MapInfo;

/* ---------------------------- Response ------------------------ */

typedef struct Response {
    int code;
    char * currtime;
    long int content_length;
    char msg[1000];
} Response;

/* ---------------------------- Request ------------------------ */

typedef struct Request {
    char * filepath;
    char * host;
    int port;
} Request;

/* ---------------------------- Stats ------------------------ */

typedef struct Statistics {
    pthread_mutex_t mtx;
    long int totaltime;
    int numpages;
    int totalbytes;
} Statistics;

/* ---------------------------- Args ------------------------ */

typedef struct Args {
    pthread_mutex_t * filemtx;
    StatisticsPtr statistics;
    QueueInfoPtr queue;
    TListInfoPtr templist;
    char * save_dir;
    char * host;
    int port;
} Args;


/* ---------------------------- Functions ------------------------ */

PListInfoPtr PList_create();
void PList_destroy(PListInfoPtr * plinfo);
void PList_insert(PListInfoPtr * const plinfo, PListElem elem, int *error);
void PList_delete_node(PListInfoPtr * const plinfo, PListNodePtr *ptr, int * const error);
int PList_isEqual(PListElem t1, PListElem t2);
void PList_find(PListInfoPtr const plinfo, PListElem elem, PListNodePtr *prepointer, int *found);
void PList_next(const PListInfoPtr plinfo, PListNodePtr * const p, int * const error);
void PList_previous(const PListInfoPtr plinfo, PListNodePtr * const p, int * const error);
void PList_getValue(PListInfoPtr const plinfo, const PListNodePtr p, PListElem *val, int * const error);
void PList_setValue(PListElem* data, PListElem temp);


void Trie_create(TrieNodePtr * root);
TrieNodePtr Trie_node_create(char key);
void Trie_destroy(TrieNodePtr *root);
void Trie_insert(TrieNodePtr *root, char* word, char* original, int docid, int id);
TrieNodePtr Trie_find(TrieNodePtr root, char* word);
void Trie_traverse(MapInfoPtr map, TrieNodePtr root, int* count);


MapInfoPtr Map_create(int size);
int Map_insert_document(MapInfoPtr minfo, int index, const char * filepath);
void Map_destroy(MapInfoPtr minfo, int size);

StatisticsPtr STATISTICS_create();
void STATISTICS_print(StatisticsPtr statistika);
void STATISTICS_send(StatisticsPtr statistika, int fd);
void STATISTICS_destroy(StatisticsPtr * p);

ResponsePtr Response_create();
ResponsePtr Response_build(const char * filepath);
void Response_send(ResponsePtr response, int newsock);
void Response_destroy(ResponsePtr * p);

ArgsPtr Args_create();
void Args_destroy(ArgsPtr * args);

RequestPtr Request_create();
RequestPtr Request_build(char * filepath,char * host, int port);
void Request_send(RequestPtr request, int newsock);
void Request_destroy(RequestPtr * p);



#endif /* STRUCTS_H */

