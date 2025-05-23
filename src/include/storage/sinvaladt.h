/* -------------------------------------------------------------------------
 *
 * sinvaladt.h
 *	  openGauss shared cache invalidation data manager.
 *
 * The shared cache invalidation manager is responsible for transmitting
 * invalidation messages between backends.	Any message sent by any backend
 * must be delivered to all already-running backends before it can be
 * forgotten.  (If we run out of space, we instead deliver a "RESET"
 * message to backends that have fallen too far behind.)
 *
 * The struct type SharedInvalidationMessage, defining the contents of
 * a single message, is defined in sinval.h.
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/storage/sinvaladt.h
 *
 * -------------------------------------------------------------------------
 */
#ifndef SINVALADT_H
#define SINVALADT_H

#include "storage/proc.h"
#include "storage/sinval.h"

/*
 * prototypes for functions in sinvaladt.c
 */
extern Size SInvalShmemSize(void);
extern void CreateSharedInvalidationState(void);
extern void CleanupWorkSessionInvalidation(void);
extern void SharedInvalBackendInit(bool sendOnly, bool worksession);
extern PGPROC* BackendIdGetProc(int backendID);

extern void SIInsertDataEntries(const SharedInvalidationMessage* data, int n, XLogRecPtr lsn = 0);
extern int SIGetDataEntries(SharedInvalidationMessage* data, int datasize, bool worksession);
extern void SICleanupQueue(bool callerHasWriteLock, int minFree);

extern LocalTransactionId GetNextLocalTransactionId(void);

#endif /* SINVALADT_H */
