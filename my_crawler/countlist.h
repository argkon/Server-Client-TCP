#ifndef COUNTLIST_H
#define COUNTLIST_H

/* ---------------------------- Countlist List ------------------------ */

typedef struct CListNode * CListNodePtr;
typedef struct CListInfo * CListInfoPtr;


typedef struct CListInfo {
    int size;
    CListNodePtr head;
} CListInfo;

typedef struct CListElem {
    int id;
    int count;
} CListElem;

typedef struct CListNode {
    CListElem data;
    CListNodePtr next;
} CListNode;


CListInfoPtr CList_create();
void CList_destroy(CListInfoPtr * clinfo);
void CList_insert(CListInfoPtr * const clinfo, CListElem elem, int *error);
void CList_delete_node(CListInfoPtr * const clinfo, CListNodePtr *ptr, int * const error);
int CList_isEqual(CListElem t1, CListElem t2);
void CList_find(CListInfoPtr const clinfo, CListElem elem, CListNodePtr *prepointer, int *found);
void CList_next(const CListInfoPtr clinfo, CListNodePtr * const p, int * const error);
void CList_previous(const CListInfoPtr clinfo, CListNodePtr * const p, int * const error);
void CList_getValue(CListInfoPtr const clinfo, const CListNodePtr p, CListElem *val, int * const error);
void CList_setValue(CListElem* data, CListElem temp);

#endif /* COUNTLIST_H */