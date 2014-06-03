#include "singly_linked_list.h"

/*===========================================================================*
 *                  Simulation for private variables                         *
 *===========================================================================*/
static unsigned long _ulSize;
static int (*_pCompare)(const void*, const void*);
static void (*_pDestroy)(void*);


/*===========================================================================*
 *                  Definition for internal functions                        *
 *===========================================================================*/
/**
 * The default comparison strategy for the items of a pair of list nodes.
 *
 * @param pSrc          The pointer to the source item.
 * @param pDst          The pointer to the target item.
 *
 * @return              1 : The source item goes after the target one with certain ordering criteria.
 *                      0 : The source item equals the target one with certain ordering criteria.
 *                     -1 : The source item goes before the target one with certain ordering criteria.
 */
int _SLListCompare(const void *pSrc, const void *pTge);


/**
 * The default deallocation strategy for a node item.
 * 
 * @param pItem         The pointer to the item which is to be deallocated.
 */
void _SLListDestroy(void *pItem);


/*===========================================================================*
 *                Implementation for exported functions                      *
 *===========================================================================*/
void SLListInit(SinglyLinkedList *self) {

    self->pHead = NULL;
    self->pTail = NULL;
    self->iSize = 0;

    /* Let the function pointers pointing to the appropriate functions. */
    self->compare = SLListCompare;
    self->destroy = SLListDestroy;

    self->append = SLListAppend;
    self->insert = SLListInsert;
    self->remove = SLListRemove;
    self->pop = SLListPop;

    self->reverse = SLListReverse;
    self->search = SLListSearch;

    return;
}


void SLListDeinit(SinglyLinkedList *self) {
    SinglyLinkedNode *next;

    /* Release the memory allocated by SinglyLinkedList structure.*/
    while (self->pHead != NULL) {
        next = self->pHead->pNext;
        self->destroy(self->pHead->pItem);
        free(self->pHead);        
        self->pHead = next;
    } 

    return;
}


/**
 * SLListAppend(): Append an item to the tail of the list.
 */
SinglyLinkedNode* SLListAppend(SinglyLinkedList *self, void *pItem) {
    SinglyLinkedNode *new;    

    new = malloc(sizeof(SinglyLinkedNode));    
    if (new != NULL) {
        new->pItem = pItem;
        new->pNext = NULL;    

        if (self->pHead == NULL) {
            self->pHead = new;
            self->pTail = new;    
        } else {
            self->pTail->pNext = new;
            self->pTail = new;            
        }
        self->iSize++;
    }

    return new;
}


/**
 * SLListInsert(): Insert an item to the designated position of the list.
 */
SinglyLinkedNode* SLListInsert(SinglyLinkedList *self, unsigned long idx, void *pItem) {
    int i;    
    SinglyLinkedNode *pred, *new, *succ;
    
    new = NULL;
    if (idx <= self->iSize) {
        pred = NULL;
        succ = self->pHead;
        for (i = 0 ; i < idx ; i++) {
            pred = succ;
            succ = succ->pNext;
        }

        new = malloc(sizeof(SinglyLinkedNode));
        if (new != NULL) {        
            new->pItem = pItem;
            new->pNext = succ;

            if (pred != NULL)
                pred->pNext = new;
            else
                self->pHead = new;
            self->iSize++;   
        }
    }

    return new;
}


/*
 * SLListRemove(): Remove the first item with the key equal to the designated item from the list,
 * and return it.
 */
void* SLListRemove(SinglyLinkedList *self, void *pItem) {
    int     rc;
    void    *pReturn;
    SinglyLinkedNode *pred, *curr, *succ;

    pReturn = NULL;
    pred = NULL;    
    curr = self->pHead;
    while (curr != NULL) {
        rc = self->compare(pItem, curr->pItem);        
        
        if (rc == 0) {
            pReturn = curr->pItem;
            succ = curr->pNext;
            free(curr);        

            if (pred != NULL)
                pred->pNext = succ;
            else
                self->pHead = succ;

            self->iSize--;
            break;
        }
        pred = curr;
        curr = curr->pNext;
    }

    return pReturn;
}


/**
 * SLListPop(): Remove the item from the designated position of the list and returns it.
 */
void* SLListPop(SinglyLinkedList *self, unsigned long idx) {
    int     i;    
    void    *pReturn; 
    SinglyLinkedNode *pred, *curr, *succ;

    pReturn = NULL;
    if (idx < self->iSize) {
        pred = NULL;
        curr = self->pHead;
        for (i = 0 ; i < idx ; i++) {
            pred = curr;
            curr = curr->pNext;        
        }

        pReturn = curr->pItem;
        succ = curr->pNext;
        free(curr);        

        if (pred != NULL)
            pred->pNext = succ;
        else
            self->pHead = succ;
        self->iSize--;
    }

    return pReturn;
}


/**
 * SLListSearch(): Check whethere the list has the designated item.
 */
bool SLListSearch(SinglyLinkedList *self, void *pItem) {
    int rc;    
    SinglyLinkedNode *curr;

    curr = self->pHead;
    while (curr != NULL) {
        rc = self->compare(pItem, curr->pItem);
        if (rc == 0)
            return true;     
        else
            curr = curr->pNext;
    }    
    
    return false;
}


/**
 * SLListReverse(): Reverse the list in place.
 */
void SLListReverse(SinglyLinkedList *self) {
    SinglyLinkedNode *pred, *curr, *succ;    

    pred = NULL;
    while (self->pHead != NULL) {
        succ = self->pHead->pNext;
        self->pHead->pNext = pred;
        pred = self->pHead;
        self->pHead = succ;
    }
    self->pHead = pred;

    return;
}


/*===========================================================================*
 *                Implementation for internal functions                      *
 *===========================================================================*/
/**
 * _SLListCompare(): It considers source and target items as primitive data and 
 *                   gives the larger order to the one with larger value.
 */
int _SLListCompare(const void *pSrc, const void *pTge) {
    
    if ((size_t)pSrc == (size_t)pTge)
        return 0;
    else {
        if ((size_t)pSrc > (size_t)pTge)
            return 1;
        else
            return -1;    
    }
}


/**
 * SLListDestroy(): By default, the item is a primitive data, and thus no further operations are required.
 */
void _SLListDestroy(void *pItem) {

    return;
}

