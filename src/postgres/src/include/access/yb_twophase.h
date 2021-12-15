/*-------------------------------------------------------------------------
 *
 * yb_twophase.h
 *	  Two-phase-commit related declarations.
 *
 *
 * Copyright (c) YugaByte, Inc.
 *
 * src/include/access/yb_twophase.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef YB_TWOPHASE_H
#define YB_TWOPHASE_H

/* GUC variable */
extern PGDLLIMPORT int max_prepared_xacts;

extern void AtAbort_Twophase(void);
extern void PostPrepare_Twophase(void);

extern GlobalTransaction MarkAsPreparing(TransactionId xid, const char *gid,
				TimestampTz prepared_at,
				Oid owner, Oid databaseid);

extern void StartPrepare(GlobalTransaction gxact);
extern void EndPrepare(GlobalTransaction gxact);

extern void FinishPreparedTransaction(const char *gid, bool isCommit);

#endif							/* YB_TWOPHASE_H */
