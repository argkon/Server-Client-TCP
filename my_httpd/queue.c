#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"

QueueInfoPtr Queue_create() {
    QueueInfoPtr qinfo=NULL;
    qinfo = malloc(sizeof (QueueInfo));
    qinfo->size = 0;
    qinfo->head = NULL;
    qinfo->tail = NULL;
    pthread_mutex_init(&(qinfo->mtx), 0);
    pthread_cond_init(&(qinfo->cond_nonempty), 0);
    //    pthread_cond_init(&qinfo->cond_nonfull, 0);

    return qinfo;
}

void Queue_destroy(QueueInfoPtr * qinfo) {
    pthread_cond_destroy(&((*qinfo)->cond_nonempty));
    //    pthread_cond_destroy(&qinfo->cond_nonfull);
    pthread_mutex_destroy(&((*qinfo)->mtx));

    QueueNodePtr todel, todel2;
    todel = (*qinfo)->head;
    while (todel != NULL) {
        todel2 = todel;
        todel = (todel)-> next;
        free(todel2);
    }
    (*qinfo)->head = NULL;
    (*qinfo)->tail = NULL;
    free(*qinfo);
    (*qinfo) = NULL;
}

void Queue_setValue(QueueElem* data, QueueElem temp) {
    data->value = temp.value;
}

void Queue_getValue(QueueInfoPtr const qinfo, const QueueNodePtr p, QueueElem *val, int * const error) { //epistrefei sto val to stoixeio/periexomeno tou ekastote komvou
    *error = 0;
    if (p != NULL)
        Queue_setValue(val, p->data);
    else
        *error = 1;
}

void Queue_insert(QueueInfoPtr * const qinfo, QueueElem elem, int *error) {         //insert to end
    QueueNodePtr temp = NULL;
    temp =(QueueNodePtr) malloc(sizeof (QueueNode));
    if (temp == NULL) {
        *error = 1;
        return;
    }
    Queue_setValue(&(temp)->data, elem);
    
    if ((*qinfo)->tail == NULL){
        (*qinfo)->head = (*qinfo)->tail = temp;
        (*qinfo)->tail->next=NULL;
        printf("Inserting First Elem \n");
        (*qinfo)->size++; //au3hse to megethos ths listas
        return;
    }
    printf("Inserting New Elem \n");
    (*qinfo)->tail->next = temp;
    (*qinfo)->tail = temp;
    (*qinfo)->tail->next=NULL;
    (*qinfo)->size++; //au3hse to megethos ths listas

}

void Queue_remove(QueueInfoPtr * const qinfo,QueueNodePtr *pointer,QueueElem * qe, int *error) {    //remove from start
   
    if ((*qinfo)->head == NULL){
        *pointer = NULL;
        printf("Queue is empty!!\n");
        return;
    }
    
    *pointer = (*qinfo)->head;
    (*qinfo)->head = (*qinfo)->head->next;
    (*pointer)->next = NULL;
    
    if ((*qinfo)->head == NULL){
        (*qinfo)->tail = NULL;
    }
    
    Queue_getValue(*qinfo,*pointer,qe,error);
        
    free(*pointer);
    *pointer=NULL;
    (*qinfo)->size--;	
}

int Queue_isEqual(QueueElem t1, QueueElem t2) {
    return (t1.value == t2.value);
}

void Queue_find(QueueInfoPtr const qinfo, QueueElem stoixeio, QueueNodePtr *prepointer, int *found) {

    QueueElem temp;
    QueueNodePtr current;
    int error = 0;
    current = qinfo->head; //3ekina apo ton voithitiko
    *prepointer = NULL;
    *found = 0;
    while ((!(*found)) && (current != NULL)) {
        Queue_getValue(qinfo, current, &temp, &error);
        if (Queue_isEqual(temp, stoixeio)) { //an to vreis return kai found=1
            *found = 1;
            *prepointer = current;
            return;
        } else { //alliws proxora ston epomeno komvo
            *prepointer = current;
            Queue_next(qinfo, &current, &error);
            if (error)
                current = NULL;
        }
    }
}

void Queue_next(const QueueInfoPtr qinfo, QueueNodePtr * const p, int * const error) {

    *error = 0;
    if ((*p) != NULL) {
        if ((*p)->next != NULL)
            *p = (*p)->next;
        else
            *error = 1;
    } else
        *error = 2;
}

void Queue_previous(const QueueInfoPtr qinfo, QueueNodePtr * const p, int * const error) {


    *error = 0;
    if ((*p) != NULL) /*  lista oxi adeia  */ {
        if ((*p) == qinfo->head) /* an deixnei ston proto kombo tis listas */
            *error = 1;
        else {
            QueueNodePtr temp = qinfo->head; //alliws psaxnei apo tin arxi ton proigoumeno
            while (temp->next != *p)
                temp = temp->next;
            *p = temp;
        }
    } else
        *error = 2;
}

void Queue_enqueue(QueueInfoPtr * const qinfo, QueueElem elem, int *error) {

    pthread_mutex_lock(&((*qinfo)->mtx));
  //  printf("Locking \n");

    Queue_insert(qinfo, elem, error);

    pthread_cond_broadcast(&((*qinfo)->cond_nonempty));
//    printf("Signal workers that queue is not empty \n");

    pthread_mutex_unlock(&((*qinfo)->mtx));
  //  printf("Unlocking \n");
}

void Queue_dequeue(QueueInfoPtr * const qinfo, QueueElem * elem, int *error) {
    QueueNodePtr temp;
    pthread_mutex_lock(&((*qinfo)->mtx));
  //  printf("Locking \n");
    
    while ((*qinfo)->size == 0) {
        printf("Wait: Queue is empty \n");
        pthread_cond_wait(&((*qinfo)->cond_nonempty), &(*qinfo)->mtx);
    }

    Queue_remove(qinfo, &temp,elem, error);
    
    pthread_mutex_unlock(&((*qinfo)->mtx));
    //printf("Unlocking \n");
}