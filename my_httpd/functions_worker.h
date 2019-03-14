#ifndef WORKER_FUNCTIONS_H
#define WORKER_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include <math.h>
#include <dirent.h>


void get_current_gmt_time(char ** atime);

//parsing functions
int worker_parseCommand(char * line, char **commands);
void parseWords(char * line, TrieNodePtr Trie, int docid,int lineid,int *wordcount,int *charcount);
char * getText(char * line,int * count);
char * getLine(char * line);
int requestValidation(char * msg);
char * multi_tok(char *input, char ** string, char *delimiter);


//functions for structs
void df(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount);
void tf(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount);
void search(int descriptors[3],MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount, FILE * log_fp,int *nstrings);
void maxcount(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount,int *max,int *maxid);
void mincount(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount,int *min,int *minid);
//void search(MapInfoPtr map, TrieNodePtr root, char** commands, int wordcount);

//functions for services
void debugmode(MapInfoPtr map, TrieNodePtr trie);
void worker_commands(int descriptors[3], MapInfoPtr map, TrieNodePtr trie, FILE * log_fp);
int count_files(const char ** paths, int npaths);






#endif /* FUNCTIONS_H */
