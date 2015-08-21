#include "map/hash_map.h"
#include "math/hash.h"


/*===========================================================================*
 *                        The container private data                         *
 *===========================================================================*/
const uint32_t aMagicPrimes[] = {
    769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319,
    201326611, 402653189, 805306457, 1610612741,
};
const int32_t iCountPrime_ = sizeof(aMagicPrimes) / sizeof(uint32_t);


typedef struct _SlotNode {
    Pair *pPair;
    struct _SlotNode *pNext;
} SlotNode;

struct _HashMapData {
    bool bEnd_;
    int32_t iSize_;
    int32_t iIdxPrime_;
    int32_t iCountSlot_;
    int32_t iIterIdx_;
    SlotNode **aSlot_;
    SlotNode *pIterNode_;
    uint32_t (*pHash_) (Key, size_t);
    void (*pDestroy_) (Pair*);
};


/*===========================================================================*
 *                  Definition for internal operations                       *
 *===========================================================================*/
#define CHECK_INIT(self)                                                        \
            do {                                                                \
                if (!self)                                                      \
                    return ERR_NOINIT;                                          \
                if (!(self->pData))                                             \
                    return ERR_NOINIT;                                          \
                if (!(self->pData->aSlot_))                                     \
                    return ERR_NOINIT;                                          \
            } while (0);


/*===========================================================================*
 *               Implementation for the exported operations                  *
 *===========================================================================*/
int32_t HashMapInit(HashMap **ppObj)
{
    *ppObj = (HashMap*)malloc(sizeof(HashMap));
    if (!(*ppObj))
        return ERR_NOMEM;
    HashMap *pObj = *ppObj;

    pObj->pData = (HashMapData*)malloc(sizeof(HashMapData));
    if (!(pObj->pData)) {
        free(*ppObj);
        *ppObj = NULL;
        return ERR_NOMEM;
    }
    HashMapData *pData = pObj->pData;

    pData->aSlot_ = (SlotNode**)malloc(sizeof(SlotNode*) * aMagicPrimes[0]);
    if (!(pData->aSlot_)) {
        free(pObj->pData);
        free(*ppObj);
        *ppObj = NULL;
        return ERR_NOMEM;
    }
    int32_t iIdx;
    for (iIdx = 0 ; iIdx < aMagicPrimes[0] ; iIdx++)
        pData->aSlot_[iIdx] = NULL;

    pData->iSize_ = 0;
    pData->iIdxPrime_ = 0;
    pData->iCountSlot_ = aMagicPrimes[0];
    pData->pHash_ = HashMurMur32;
    pData->pDestroy_ = NULL;

    pObj->put = HashMapPut;
    pObj->get = HashMapGet;
    pObj->find = HashMapFind;
    pObj->remove = HashMapRemove;
    pObj->size = HashMapSize;
    pObj->iterate = HashMapIterate;
    pObj->set_destroy = HashMapSetDestroy;
    pObj->set_hash = HashMapSetHash;

    return SUCC;
}

void HashMapDeinit(HashMap **ppObj)
{
    if (!(*ppObj))
        goto EXIT;

    HashMap *pObj = *ppObj;
    if (!(pObj->pData))
        goto FREE_MAP;

    HashMapData *pData = pObj->pData;
    if (!(pData->aSlot_))
        goto FREE_DATA;

    free(pData->aSlot_);

FREE_DATA:
    free(pObj->pData);
FREE_MAP:
    free(*ppObj);
    *ppObj = NULL;
EXIT:
    return;
}

int32_t HashMapPut(HashMap *self, Pair *pPair, size_t size)
{
    CHECK_INIT(self);
    if (size == 0)
        return ERR_KEYSIZE;

    HashMapData *pData = self->pData;
    SlotNode **aSlot = pData->aSlot_;

    /* Calculate the slot index. */
    uint32_t uiValue = pData->pHash_(pPair->key, size);
    uiValue = uiValue % pData->iCountSlot_;

    /* Check if the pair conflicts with a certain one stored in the map. If yes,
       replace that one. */
    SlotNode *pCurr = aSlot[uiValue];
    while (pCurr) {
        int32_t iRes = memcmp(pCurr->pPair->key, pPair->key, size);
        if (iRes == 0) {
            if (pData->pDestroy_)
                pData->pDestroy_(pCurr->pPair);
            pCurr->pPair = pPair;
            return SUCC;
        }
        pCurr = pCurr->pNext;
    }

    /* Insert the new pair into the slot list. */
    SlotNode *pNew = (SlotNode*)malloc(sizeof(SlotNode));
    pNew->pPair = pPair;
    if (!(aSlot[uiValue])) {
        pNew->pNext = NULL;
        aSlot[uiValue] = pNew;
    } else {
        pNew->pNext = aSlot[uiValue];
        aSlot[uiValue] = pNew;
    }
    pData->iSize_++;

    return SUCC;
}

int32_t HashMapGet(HashMap *self, Key key, size_t size, Value *pValue)
{
    CHECK_INIT(self);
    if (!pValue)
        return ERR_GET;
    if (size == 0)
        return ERR_KEYSIZE;

    HashMapData *pData = self->pData;

    /* Calculate the slot index. */
    uint32_t uiValue = pData->pHash_(key, size);
    uiValue = uiValue % pData->iCountSlot_;

    /* Search the slot list to check if there is a pair having the same key
       with the designated one. */
    SlotNode *pCurr = pData->aSlot_[uiValue];
    while (pCurr) {
        int32_t iRes = memcmp(pCurr->pPair->key, key, size);
        if (iRes == 0) {
            *pValue = pCurr->pPair->value;
            return SUCC;
        }
        pCurr = pCurr->pNext;
    }

    *pValue = NULL;
    return NOKEY;
}

int32_t HashMapFind(HashMap *self, Key key, size_t size)
{
    CHECK_INIT(self);
    if (size == 0)
        return ERR_KEYSIZE;

    HashMapData *pData = self->pData;

    /* Calculate the slot index. */
    uint32_t uiValue = pData->pHash_(key, size);
    uiValue = uiValue % pData->iCountSlot_;

    /* Search the slot list to check if there is a pair having the same key
       with the designated one. */
    SlotNode *pCurr = pData->aSlot_[uiValue];
    while (pCurr) {
        int32_t iRes = memcmp(pCurr->pPair->key, key, size);
        if (iRes == 0)
            return SUCC;
        pCurr = pCurr->pNext;
    }

    return NOKEY;
}

int32_t HashMapRemove(HashMap *self, Key key, size_t size)
{
    CHECK_INIT(self);
    if (size == 0)
        return ERR_KEYSIZE;

    HashMapData *pData = self->pData;
    SlotNode **aSlot = pData->aSlot_;

    /* Calculate the slot index. */
    uint32_t uiValue = pData->pHash_(key, size);
    uiValue = uiValue % pData->iCountSlot_;

    /* Search the slot list for the deletion target. */
    SlotNode *pPred = NULL;
    SlotNode *pCurr = aSlot[uiValue];
    while (pCurr) {
        int32_t iRes = memcmp(pCurr->pPair->key, key, size);
        if (iRes == 0) {
            if (pData->pDestroy_)
                pData->pDestroy_(pCurr->pPair);
            if (!pPred)
                aSlot[uiValue] = pCurr->pNext;
            else
                pPred->pNext = pCurr->pNext;
            free(pCurr);
            return SUCC;
        }
        pPred = pCurr;
        pCurr = pCurr->pNext;
    }

    return ERR_NODATA;
}

int32_t HashMapSize(HashMap *self)
{
    CHECK_INIT(self);
    return self->pData->iSize_;
}

int32_t HashMapIterate(HashMap *self, bool bReset, Pair **ppPair)
{
    CHECK_INIT(self);

    HashMapData *pData = self->pData;

    if (bReset) {
        pData->iIterIdx_ = 0;
        pData->pIterNode_ = pData->aSlot_[0];
        pData->bEnd_ = false;
        return SUCC;
    }

    if (!ppPair)
        return ERR_GET;

    if (pData->bEnd_) {
        *ppPair = NULL;
        return END;
    }

    SlotNode **aSlot = pData->aSlot_;
    do {
        while (pData->pIterNode_) {
            *ppPair = pData->pIterNode_->pPair;
            pData->pIterNode_ = pData->pIterNode_->pNext;
            return SUCC;
        }
        pData->iIterIdx_++;
        if (pData->iIterIdx_ == pData->iCountSlot_)
            break;
        pData->pIterNode_ = aSlot[pData->iIterIdx_];
    } while (true);

    pData->bEnd_ = true;
    *ppPair = NULL;
    return END;
}

int32_t HashMapSetDestroy(HashMap *self, void (*pFunc) (Pair*))
{
    CHECK_INIT(self);
    self->pData->pDestroy_ = pFunc;
    return SUCC;
}

int32_t HashMapSetHash(HashMap *self, uint32_t (*pFunc) (Key, size_t))
{
    CHECK_INIT(self);
    self->pData->pHash_ = pFunc;
    return SUCC;
}


/*===========================================================================*
 *               Implementation for internal operations                      *
 *===========================================================================*/
