#include <stdio.h>
#include <stdlib.h>

#include "templist.h"
#include "queue.h"
#include <string.h>

TListInfoPtr TList_create() {
    TListInfoPtr tlinfo;
    tlinfo = malloc(sizeof(TListInfo));
    tlinfo->size = 0;
    tlinfo->head = NULL;
    return tlinfo;
}

void TList_destroy(TListInfoPtr * tlinfo) {
    TListNodePtr todel, todel2;
    todel = (*tlinfo)->head;
    while (todel != NULL) {
        todel2 = todel;
        todel = (todel)-> next;
        free(todel2);
    }
    (*tlinfo)->head = NULL;
    free(*tlinfo);
    (*tlinfo) = NULL;
}

void TList_setValue(TListElem* data, TListElem temp) {
    memset(data->url,0,MAX_URL_LEN);
    strcpy(data->url,temp.url);
}

void TList_getValue(TListInfoPtr const tlinfo, const TListNodePtr p, TListElem *val, int * const error) { //epistrefei sto val to stoixeio/periexomeno tou ekastote komvou
    *error = 0;
    if (p != NULL)
        TList_setValue(val, p->data);
    else
        *error = 1;
}

void TList_insert(TListInfoPtr * const tlinfo, TListElem elem, int *error) {
    TListNodePtr temp;
    temp = malloc(sizeof (TListNode));
    if (temp == NULL) {
        *error = 1;
        return;
    }
    TList_setValue(&temp->data, elem);
    temp->next = (*tlinfo)->head; //vale to neo stoixeio na deixnei sto head tou voithitikou
    (*tlinfo)->head = temp; //vale ton vohthitiko na deixnei sto neo stoixeio 
    (*tlinfo)->size++; //au3hse to megethos ths listas

}

int TList_isEqual(TListElem t1, TListElem t2) {
    if (strcmp(t1.url, t2.url) == 0)
        return 1; //anazhtame me vasi to url
    else
        return 0;
}

void TList_find(TListInfoPtr const tlinfo, TListElem stoixeio, TListNodePtr *prepointer, int *found) {

    TListElem temp;
    TListNodePtr current;
    int error = 0;
    current = tlinfo->head; //3ekina apo ton voithitiko
    *prepointer = NULL;
    *found = 0;
    while ((!(*found)) && (current != NULL)) {
        TList_getValue(tlinfo, current, &temp, &error);
        if (TList_isEqual(temp, stoixeio)) { //an to vreis return kai found=1
            *found = 1;
            *prepointer = current;
            return;
        } else { //alliws proxora ston epomeno komvo
            *prepointer = current;
            TList_next(tlinfo, &current, &error);
            if (error)
                current = NULL;
        }
    }
}

void TList_next(const TListInfoPtr tlinfo, TListNodePtr * const p, int * const error) {

    *error = 0;
    if ((*p) != NULL) {
        if ((*p)->next != NULL)
            *p = (*p)->next;
        else
            *error = 1;
    } else
        *error = 2;
}

void TList_previous(const TListInfoPtr tlinfo, TListNodePtr * const p, int * const error) {


    *error = 0;
    if ((*p) != NULL) /*  lista oxi adeia  */ {
        if ((*p) == tlinfo->head) /* an deixnei ston proto kombo tis listas */
            *error = 1;
        else {
            TListNodePtr temp = tlinfo->head; //alliws psaxnei apo tin arxi ton proigoumeno
            while (temp->next != *p)
                temp = temp->next;
            *p = temp;
        }
    } else
        *error = 2;
}