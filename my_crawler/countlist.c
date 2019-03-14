
#include <stdio.h>
#include <stdlib.h>

#include "countlist.h"


CListInfoPtr CList_create() {
    CListInfoPtr clinfo;
    clinfo = malloc(sizeof(CListInfo));
    clinfo->size = 0;
    clinfo->head = NULL;
    return clinfo;
}

void CList_destroy(CListInfoPtr * clinfo) {
    CListNodePtr todel, todel2;
    todel = (*clinfo)->head;
    while (todel != NULL) {
        todel2 = todel;
        todel = (todel)-> next;
        free(todel2);
    }
    (*clinfo)->head = NULL;
    free(*clinfo);
    (*clinfo) = NULL;
}

void CList_setValue(CListElem* data, CListElem temp) {
    data->id = temp.id;
    data->count = temp.count;
}

void CList_getValue(CListInfoPtr const clinfo, const CListNodePtr p, CListElem *val, int * const error) { //epistrefei sto val to stoixeio/periexomeno tou ekastote komvou
    *error = 0;
    if (p != NULL)
        CList_setValue(val, p->data);
    else
        *error = 1;
}

void CList_insert(CListInfoPtr * const clinfo, CListElem elem, int *error) {
    CListNodePtr temp;
    temp = malloc(sizeof (CListNode));
    if (temp == NULL) {
        *error = 1;
        return;
    }
    CList_setValue(&temp->data, elem);
    temp->next = (*clinfo)->head; //vale to neo stoixeio na deixnei sto head tou voithitikou
    (*clinfo)->head = temp; //vale ton vohthitiko na deixnei sto neo stoixeio 
    (*clinfo)->size++; //au3hse to megethos ths listas

}

int CList_isEqual(CListElem t1, CListElem t2) {
    return (t1.id == t2.id); //anazhtame me vasi to id tou document
}

void CList_find(CListInfoPtr const clinfo, CListElem stoixeio, CListNodePtr *prepointer, int *found) {

    CListElem temp;
    CListNodePtr current;
    int error = 0;
    current = clinfo->head; //3ekina apo ton voithitiko
    *prepointer = NULL;
    *found = 0;
    while ((!(*found)) && (current != NULL)) {
        CList_getValue(clinfo, current, &temp, &error);
        if (CList_isEqual(temp, stoixeio)) { //an to vreis return kai found=1
            *found = 1;
            *prepointer = current;
            return;
        } else { //alliws proxora ston epomeno komvo
            *prepointer = current;
            CList_next(clinfo, &current, &error);
            if (error)
                current = NULL;
        }
    }
}

void CList_next(const CListInfoPtr clinfo, CListNodePtr * const p, int * const error) {

    *error = 0;
    if ((*p) != NULL) {
        if ((*p)->next != NULL)
            *p = (*p)->next;
        else
            *error = 1;
    } else
        *error = 2;
}

void CList_previous(const CListInfoPtr clinfo, CListNodePtr * const p, int * const error) {


    *error = 0;
    if ((*p) != NULL) /*  lista oxi adeia  */ {
        if ((*p) == clinfo->head) /* an deixnei ston proto kombo tis listas */
            *error = 1;
        else {
            CListNodePtr temp = clinfo->head; //alliws psaxnei apo tin arxi ton proigoumeno
            while (temp->next != *p)
                temp = temp->next;
            *p = temp;
        }
    } else
        *error = 2;
}