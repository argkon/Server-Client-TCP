#include <stdio.h>
#include <stdlib.h>

#include "searchlist.h"


SListInfoPtr SList_create() {
    SListInfoPtr slinfo;
    slinfo = malloc(sizeof(SListInfo));
    slinfo->size = 0;
    slinfo->head = NULL;
    return slinfo;
}

void SList_destroy(SListInfoPtr * slinfo) {
    SListNodePtr todel, todel2;
    todel = (*slinfo)->head;
    while (todel != NULL) {
        todel2 = todel;
        todel = (todel)-> next;
        free(todel2);
    }
    (*slinfo)->head = NULL;
    free(*slinfo);
    (*slinfo) = NULL;
}

void SList_setValue(SListElem* data, SListElem temp) {
    data->docid = temp.docid;
    data->lineid = temp.lineid;
}

void SList_getValue(SListInfoPtr const slinfo, const SListNodePtr p, SListElem *val, int * const error) { //epistrefei sto val to stoixeio/periexomeno tou ekastote komvou
    *error = 0;
    if (p != NULL)
        SList_setValue(val, p->data);
    else
        *error = 1;
}

void SList_insert(SListInfoPtr * const slinfo, SListElem elem, int *error) {
    SListNodePtr temp;
    temp = malloc(sizeof (SListNode));
    if (temp == NULL) {
        *error = 1;
        return;
    }
    SList_setValue(&temp->data, elem);
    temp->next = (*slinfo)->head; //vale to neo stoixeio na deixnei sto head tou voithitikou
    (*slinfo)->head = temp; //vale ton vohthitiko na deixnei sto neo stoixeio 
    (*slinfo)->size++; //au3hse to megethos ths listas

}

int SList_isEqual(SListElem t1, SListElem t2) {
    return (t1.docid == t2.docid && t1.lineid == t2.lineid); //anazhtame me vasi to id tou document kai tin grammi
}

void SList_find(SListInfoPtr const slinfo, SListElem stoixeio, SListNodePtr *prepointer, int *found) {

    SListElem temp;
    SListNodePtr current;
    int error = 0;
    current = slinfo->head; //3ekina apo ton voithitiko
    *prepointer = NULL;
    *found = 0;
    while ((!(*found)) && (current != NULL)) {
        SList_getValue(slinfo, current, &temp, &error);
        if (SList_isEqual(temp, stoixeio)) { //an to vreis return kai found=1
            *found = 1;
            *prepointer = current;
            return;
        } else { //alliws proxora ston epomeno komvo
            *prepointer = current;
            SList_next(slinfo, &current, &error);
            if (error)
                current = NULL;
        }
    }
}

void SList_next(const SListInfoPtr slinfo, SListNodePtr * const p, int * const error) {

    *error = 0;
    if ((*p) != NULL) {
        if ((*p)->next != NULL)
            *p = (*p)->next;
        else
            *error = 1;
    } else
        *error = 2;
}

void SList_previous(const SListInfoPtr slinfo, SListNodePtr * const p, int * const error) {


    *error = 0;
    if ((*p) != NULL) /*  lista oxi adeia  */ {
        if ((*p) == slinfo->head) /* an deixnei ston proto kombo tis listas */
            *error = 1;
        else {
            SListNodePtr temp = slinfo->head; //alliws psaxnei apo tin arxi ton proigoumeno
            while (temp->next != *p)
                temp = temp->next;
            *p = temp;
        }
    } else
        *error = 2;
}