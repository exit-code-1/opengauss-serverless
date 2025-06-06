/* -------------------------------------------------------------------------
 *
 * knl_uvisibility.h
 * Tuple visibility interfaces of inplace update engine.
 *
 * Portions Copyright (c) 2020 Huawei Technologies Co.,Ltd.
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 * src/include/access/ustore/knl_uvisibility.h
 * -------------------------------------------------------------------------
 */

#ifndef KNL_UVISIBILITY_H
#define KNL_UVISIBILITY_H

#include "postmaster/postmaster.h"

typedef struct UHeapTupleTransInfo {
    int td_slot;
    TransactionId xid;
    CommandId cid;
    UndoRecPtr urec_add;
} UHeapTupleTransInfo;

typedef enum {
    UTUPLETID_NEW,      /* inserted */
    UTUPLETID_MODIFIED, /* in-place update or lock */
    UTUPLETID_GONE      /* non-in-place update or delete */
} UTupleTidOp;

typedef enum {
    UVERSION_NONE,
    UVERSION_CURRENT,
    UVERSION_OLDER,
    UVERSION_CHECK_CID
} UVersionSelector;

#define SNAPSHOT_REQUESTS_SUBXID 0x0001

/* Result codes for UHeapTupleSatisfiesOldestXmin */
typedef enum {
    UHEAPTUPLE_DEAD,               /* tuple is dead and deletable */
    UHEAPTUPLE_LIVE,               /* tuple is live (committed, no deleter) */
    UHEAPTUPLE_RECENTLY_DEAD,      /* tuple is dead, but not deletable yet */
    UHEAPTUPLE_INSERT_IN_PROGRESS, /* inserting xact is still in progress */
    UHEAPTUPLE_DELETE_IN_PROGRESS, /* deleting xact is still in progress */
    UHEAPTUPLE_ABORT_IN_PROGRESS   /* rollback is still pending */
} UHTSVResult;

typedef struct UstoreUndoScanDescData {
    UHeapTupleTransInfo uinfo;
    TransactionId prevUndoXid;
    UHeapTuple currentUHeapTuple;
} UstoreUndoScanDescData;
typedef UstoreUndoScanDescData* UstoreUndoScanDesc;

bool UHeapTupleFetch(Relation rel, Buffer buffer, OffsetNumber offnum, Snapshot snapshot, UHeapTuple *visibleTuple,
    ItemPointer newCtid, bool keepTup, UHeapTupleTransInfo *savedUinfo = NULL, bool *gotTdInfo = NULL,
    const UHeapTuple *saved_tuple = NULL, int16 lastVar = -1, bool *boolArr = NULL, bool *has_cur_xact_write = NULL);

bool UHeapTupleSatisfiesVisibility(UHeapTuple uhtup, Snapshot snapshot, Buffer buffer,
    TransactionId *tdXmin = NULL);
extern TransactionId UDiskTupleGetModifiedXid(UHeapDiskTuple diskTup, Page page);

TM_Result UHeapTupleSatisfiesUpdate(Relation rel, Snapshot snapshot, ItemPointer tid, UHeapTuple utuple,
    CommandId cid, Buffer buffer, ItemPointer ctid, UHeapTupleTransInfo *uinfo,
    SubTransactionId *updateSubXid, TransactionId *lockerXid, SubTransactionId *lockerSubXid, bool lockedForUpdate,
    bool multixidIsMyself, bool *inplaceUpdated, bool selfVisible = false, bool isLockForUpdate = false,
    TransactionId conflictXid = InvalidTransactionId, bool isUpsert = false);

TransactionId UHeapTupleGetTransXid(UHeapTuple uhtup, Buffer buf, bool nobuflock, bool* has_cur_xact_write = NULL);

UndoTraversalState UHeapTupleGetTransInfo(Buffer buf, OffsetNumber offnum, UHeapTupleTransInfo *txactinfo,
    bool* has_cur_xact_write = NULL, TransactionId *lastXid = NULL, UndoRecPtr *urp = NULL);

UHTSVResult UHeapTupleSatisfiesOldestXmin(UHeapTuple inplacehtup, TransactionId OldestXmin, Buffer buffer,
    bool resolve_abort_in_progress, UHeapTuple *preabort_tuple, TransactionId *xid, SubTransactionId *subxid,
    Relation rel, bool *inplaceUpdated = NULL, TransactionId *lastXid = NULL);

CommandId UHeapTupleGetCid(UHeapTuple uhtup, Buffer buf);

void GetTDSlotInfo(Buffer buf, int tdId, UHeapTupleTransInfo *tdinfo);

UndoTraversalState FetchTransInfoFromUndo(BlockNumber blocknum, OffsetNumber offnum, TransactionId xid,
    UHeapTupleTransInfo *txactinfo, ItemPointer newCtid, bool needByPass, TransactionId *lastXid, UndoRecPtr *urp);

bool UHeapTupleIsSurelyDead(UHeapTuple uhtup, Buffer buffer, OffsetNumber offnum,
    const UHeapTupleTransInfo *cachedTdInfo, const bool useCachedTdInfo);

bool UHeapTupleHasSerializableConflictOut(bool visible, Relation relation, ItemPointer tid, Buffer buffer,
    TransactionId *xid);

void UHeapTupleCheckVisible(Snapshot snapshot, UHeapTuple tuple, Buffer buffer);

void UHeapUpdateTDInfo(int tdSlot, Buffer buffer, OffsetNumber offnum, UHeapTupleTransInfo* uinfo);

extern bool UHeapSearchBufferShowAnyTuplesFirstCall(ItemPointer tid, Relation relation,
    Buffer buffer, UstoreUndoScanDesc xc_undo_scan);

extern bool UHeapSearchBufferShowAnyTuplesFromUndo(ItemPointer tid, Relation relation,
    Buffer buffer, UstoreUndoScanDesc xc_undo_scan);

inline bool TransactionIdOlderThanAllUndo(TransactionId xid)
{
    /* to slove standby read consistency problem */
    if (RecoveryInProgress()) {
        uint64 standby_recycle_xid = pg_atomic_read_u64(&g_instance.undo_cxt.hotStandbyRecycleXid);
        return xid < standby_recycle_xid;
    }

    uint64 cutoff = pg_atomic_read_u64(&g_instance.undo_cxt.globalRecycleXid);
    return xid < cutoff;
}

inline bool TransactionIdOlderThanFrozenXid(TransactionId xid)
{
    /* to slove standby read consistency problem */
    if (RecoveryInProgress()) {
        uint64 standby_recycle_xid = pg_atomic_read_u64(&g_instance.undo_cxt.hotStandbyRecycleXid);
        return xid < standby_recycle_xid;
    }

    uint64 cutoff = pg_atomic_read_u64(&g_instance.undo_cxt.globalFrozenXid);
    return xid < cutoff;
}
#endif
