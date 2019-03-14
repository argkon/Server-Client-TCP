#ifndef TEMPLIST_H
#define TEMPLIST_H

#define MAX_URL_LEN 1024

/* ---------------------------- Temporary List ------------------------ */

typedef struct TListNode * TListNodePtr;
typedef struct TListInfo * TListInfoPtr;

typedef struct TListInfo {
    int size;
    TListNodePtr head;
} TListInfo;

typedef struct TListElem {
    char url[MAX_URL_LEN];
} TListElem;

typedef struct TListNode {
    TListElem data;
    TListNodePtr next;
} TListNode;


TListInfoPtr TList_create();
void TList_destroy(TListInfoPtr * tlinfo);
void TList_insert(TListInfoPtr * const tlinfo, TListElem elem, int *error);
void TList_delete_node(TListInfoPtr * const tlinfo, TListNodePtr *ptr, int * const error);
int TList_isEqual(TListElem t1, TListElem t2);
void TList_find(TListInfoPtr const tlinfo, TListElem elem, TListNodePtr *prepointer, int *found);
void TList_next(const TListInfoPtr tlinfo, TListNodePtr * const p, int * const error);
void TList_previous(const TListInfoPtr tlinfo, TListNodePtr * const p, int * const error);
void TList_getValue(TListInfoPtr const tlinfo, const TListNodePtr p, TListElem *val, int * const error);
void TList_setValue(TListElem* data, TListElem temp);



#endif /* TEMPLIST_H */
