#ifndef C_QUEUE_H
#define C_QUEUE_H

#define MAX_URL_LEN 1024
#include "templist.h"

typedef struct QueueNode * QueueNodePtr;
typedef struct QueueInfo * QueueInfoPtr;

typedef struct QueueInfo {
    int size;
    QueueNodePtr head;
    QueueNodePtr tail;
    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty;
//    pthread_cond_t cond_nonfull;
} QueueInfo;

typedef struct QueueElem {
    int value;
    char url[MAX_URL_LEN];
} QueueElem;

typedef struct QueueNode {
    QueueElem data;
    QueueNodePtr next;
} QueueNode;


QueueInfoPtr Queue_create();
void Queue_destroy(QueueInfoPtr * qinfo);
void Queue_insert(QueueInfoPtr * const qinfo, QueueElem elem, int *error);
void Queue_remove(QueueInfoPtr * const qinfo,QueueNodePtr *pointer,QueueElem * qe, int *error);
void Queue_delete_node(QueueInfoPtr * const qinfo, QueueNodePtr *ptr, int * const error);
int Queue_isEqual(QueueElem t1, QueueElem t2);
void Queue_find(QueueInfoPtr const qinfo, QueueElem elem, QueueNodePtr *prepointer, int *found);
void Queue_next(const QueueInfoPtr qinfo, QueueNodePtr * const p, int * const error);
void Queue_previous(const QueueInfoPtr qinfo, QueueNodePtr * const p, int * const error);
void Queue_getValue(QueueInfoPtr const qinfo, const QueueNodePtr p, QueueElem *val, int * const error);
void Queue_setValue(QueueElem* data, QueueElem temp);


void Queue_enqueue(QueueInfoPtr * const qinfo, QueueElem elem, int *error,TListInfoPtr* tinfo);
void Queue_dequeue(QueueInfoPtr * const qinfo, QueueElem * elem, int *error);




#endif /* QUEUE_H */

