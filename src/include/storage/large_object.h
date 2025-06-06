/* -------------------------------------------------------------------------
 *
 * large_object.h
 *	  Declarations for openGauss large objects.  POSTGRES 4.2 supported
 *	  zillions of large objects (internal, external, jaquith, inversion).
 *	  Now we only support inversion.
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/storage/large_object.h
 *
 * -------------------------------------------------------------------------
 */
#ifndef LARGE_OBJECT_H
#define LARGE_OBJECT_H

#include "utils/snapshot.h"

/* ----------
 * Data about a currently-open large object.
 *
 * id is the logical OID of the large object
 * snapshot is the snapshot to use for read/write operations
 * subid is the subtransaction that opened the desc (or currently owns it)
 * offset is the current seek offset within the LO
 * flags contains some flag bits
 *
 * NOTE: in current usage, flag bit IFS_RDLOCK is *always* set, and we don't
 * bother to test for it.  Permission checks are made at first read or write
 * attempt, not during inv_open(), so we have other bits to remember that.
 *
 * NOTE: before 7.1, we also had to store references to the separate table
 * and index of a specific large object.  Now they all live in pg_largeobject
 * and are accessed via a common relation descriptor.
 * ----------
 */
typedef struct LargeObjectDesc {
    Oid id;                 /* LO's identifier */
    Snapshot snapshot;      /* snapshot to use */
    SubTransactionId subid; /* owning subtransaction ID */
    uint64 offset;          /* current seek pointer */
    int flags;              /* locking info, etc */

/* flag bits: */
#define IFS_RDLOCK (1 << 0)
#define IFS_WRLOCK (1 << 1)
#define IFS_RD_PERM_OK (1 << 2) /* read permission has been verified */
#define IFS_WR_PERM_OK (1 << 3) /* write permission has been verified */

} LargeObjectDesc;

/*
 * Each "page" (tuple) of a large object can hold this much data
 *
 * We could set this as high as BLCKSZ less some overhead, but it seems
 * better to make it a smaller value, so that not as much space is used
 * up when a page-tuple is updated.  Note that the value is deliberately
 * chosen large enough to trigger the tuple toaster, so that we will
 * attempt to compress page tuples in-line.  (But they won't be moved off
 * unless the user creates a toast-table for pg_largeobject...)
 *
 * Also, it seems to be a smart move to make the page size be a power of 2,
 * since clients will often be written to send data in power-of-2 blocks.
 * This avoids unnecessary tuple updates caused by partial-page writes.
 */
#define LOBLKSIZE (BLCKSZ / 4)

/*
 * Maximum length in bytes for a large object.  To make this larger, we'd
 * have to widen pg_largeobject.pageno as well as various internal variables.
 */
#define MAX_LARGE_OBJECT_SIZE ((int64) INT_MAX * LOBLKSIZE)

/* compatible with C code for pljava, otherwise pljava will met compile error */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function definitions...
 */

/* inversion stuff in inv_api.c */
extern void close_lo_relation(bool isCommit);
extern Oid inv_create(Oid lobjId);
extern LargeObjectDesc* inv_open(Oid lobjId, int flags, MemoryContext mcxt);
extern void inv_close(LargeObjectDesc* obj_desc);
extern int inv_drop(Oid lobjId);
extern int64 inv_seek(LargeObjectDesc* obj_desc, int64 offset, int whence);
extern int64 inv_tell(LargeObjectDesc* obj_desc);
extern int inv_read(LargeObjectDesc* obj_desc, char* buf, int nbytes);
extern int inv_write(LargeObjectDesc* obj_desc, const char* buf, int nbytes);
extern void inv_truncate(LargeObjectDesc* obj_desc, int64 len);

#ifdef __cplusplus
}
#endif

#endif /* LARGE_OBJECT_H */
