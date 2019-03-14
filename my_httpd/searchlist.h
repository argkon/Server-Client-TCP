#ifndef SEARCHLIST_H
#define SEARCHLIST_H

/* ---------------------------- Searchlist List ------------------------ */

typedef struct SListNode * SListNodePtr;
typedef struct SListInfo * SListInfoPtr;


typedef struct SListInfo {
    int size;
    SListNodePtr head;
} SListInfo;

typedef struct SListElem {
    int docid;
    int lineid;
} SListElem;

typedef struct SListNode {
    SListElem data;
    SListNodePtr next;
} SListNode;


SListInfoPtr SList_create();
void SList_destroy(SListInfoPtr * slinfo);
void SList_insert(SListInfoPtr * const slinfo, SListElem elem, int *error);
void SList_delete_node(SListInfoPtr * const slinfo, SListNodePtr *ptr, int * const error);
int SList_isEqual(SListElem t1, SListElem t2);
void SList_find(SListInfoPtr const slinfo, SListElem elem, SListNodePtr *prepointer, int *found);
void SList_next(const SListInfoPtr slinfo, SListNodePtr * const p, int * const error);
void SList_previous(const SListInfoPtr slinfo, SListNodePtr * const p, int * const error);
void SList_getValue(SListInfoPtr const slinfo, const SListNodePtr p, SListElem *val, int * const error);
void SList_setValue(SListElem* data, SListElem temp);



#endif /* SEARCHLIST_H */

