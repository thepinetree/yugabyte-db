/*-------------------------------------------------------------------------
 *
 * twophase.c
 *		Two-phase commit support functions.
 *
 * Copyright (c) YugaByte, Inc.
 *
 * IDENTIFICATION
 *		src/backend/access/transam/yb_twophase.c
 *
 *-------------------------------------------------------------------------
 */
#include "c.h"
#include "miscadmin.h"
#include "pg_yb_utils.h"
#include "postgres.h"

#include "access/twophase.h"
#include "storage/lockdefs.h"

static Oid get_prepared_txn_gid(const char *gid);

/*
 * MarkAsPreparing
 *		Reserve the GID for the given transaction.
 */
GlobalTransaction
MarkAsPreparing(TransactionId xid, const char *gid,
				TimestampTz prepared_at, Oid owner, Oid databaseid)
{
	Relation			rel;
	Datum				values[Natts_pg_yb_prepared_xact];
	bool				nulls[Natts_pg_yb_prepared_xact];
	HeapTuple			tuple;
	Oid 				prepared_xact_oid;
	int					count;

	if (!YbPreparedXactCatalogExists)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("Prepared transaction system catalog does not exist.")));

	if (strlen(gid) >= GIDSIZE)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("transaction identifier \"%s\" is too long",
						gid)));

	/* fail immediately if feature is disabled */
	if (max_prepared_xacts == 0)
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("prepared transactions are disabled"),
				 errhint("Set max_prepared_transactions to a nonzero value.")));

	/* Check for conflicting GID */
	if (OidIsValid(get_prepared_txn_gid(gid)))
		ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_OBJECT),
				 errmsg("transaction identifier \"%s\" is already in use",
						gid)));

	// TODO(Deepayan): Determine way to get count here
	if (count >= max_prepared_xacts)
		ereport(ERROR,
			(errcode(ERRCODE_OUT_OF_MEMORY),
			 errmsg("maximum number of prepared transactions reached"),
			 errhint("Increase max_prepared_transactions (currently %d).",
					 max_prepared_xacts)));

	/*
	 * Insert tuple into pg_yb_prepared_xact.
	 */
	rel = heap_open(YbPreparedXactRelationId, RowExclusiveLock);

	MemSet(nulls, false, sizeof(nulls));

	values[Anum_pg_yb_prepared_xact_transaction_uuid - 1] =
		TransactionIdGetDatum(xid);
	values[Anum_pg_yb_prepared_xact_global_name - 1] =
		DirectFunctionCall1(namein, CStringGetDatum(gid));
	values[Anum_pg_yb_prepared_xact_prepared_at - 1] = GetCurrentTimestamp();
	values[Anum_pg_yb_prepared_xact_ownerid - 1] = ObjectIdGetDatum(owner);
	values[Anum_pg_yb_prepared_xact_dbid - 1] = ObjectIdGetDatum(databaseid);

	tuple = heap_form_tuple(rel->rd_att, values, nulls);

	prepared_xact_oid = CatalogTupleInsert(rel, tuple);

	heap_freetuple(tuple);

	/* Post creation hook for new tablespace */
	InvokeObjectPostCreateHook(YbPreparedXactRelationId, tablespaceoid, 0);

	/* We keep the lock on pg_tablegroup until commit */
	heap_close(rel, NoLock);

	return prepared_xact_oid;
}


/*
 * FinishPreparedTransaction: execute COMMIT PREPARED or ROLLBACK PREPARED
 */
// TODO(Deepayan): Finish this function
void
FinishPreparedTransaction(const char *gid, bool isCommit)
{
	HeapScanDesc scandesc;
	Relation			rel;
	HeapTuple			tuple;


	if (!YbPreparedXactCatalogExists)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("Prepared transaction system catalog does not "
				 		"exist.")));

	/*
	 * The order of operations here is critical: make the XLOG entry for
	 * commit or abort, then mark the transaction committed or aborted in
	 * pg_xact, then remove its PGPROC from the global ProcArray (which means
	 * TransactionIdIsInProgress will stop saying the prepared xact is in
	 * progress), then run the post-commit or post-abort callbacks. The
	 * callbacks will release the locks the transaction held.
	 */
	//  TODO: Actually determine how COMMIT and ABORT messages should be sent?
	if (isCommit)
		RecordTransactionCommitPrepared(xid,
										hdr->nsubxacts, children,
										hdr->ncommitrels, commitrels,
										hdr->ninvalmsgs, invalmsgs,
										hdr->initfileinval, gid);
	else
		RecordTransactionAbortPrepared(xid,
									   hdr->nsubxacts, children,
									   hdr->nabortrels, abortrels,
									   gid);

	ScanKeyInit(&entry[0],
				Anum_pg_prepared_xact_gid,
				BTEqualStrategyNumber, F_NAMEEQ,
				CStringGetDatum(gid));
	scandesc = heap_beginscan_catalog(rel, 1, entry);
	tuple = heap_getnext(scandesc, ForwardScanDirection);

	if (!HeapTupleIsValid(tuple))
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("tablespace \"%s\" does not exist",
						tablespacename)));

	prepared_xact_oid = HeapTupleGetOid(tuple);

	/* Must be tablespace owner */
	if (!pg_prepared_xact_ownercheck(prepared_xact_oid, GetUserId()))
		aclcheck_error(ACLCHECK_NOT_OWNER, OBJECT_PREPARED_XACT, gid);

	/* DROP hook for the prepared transaction being removed */
	InvokeObjectDropHook(YbPreparedXactRelationId, prepared_xact_oid, 0);

	/*
	 * Remove the pg_tablespace tuple (this will roll back if we fail below)
	 */
	CatalogTupleDelete(rel, tuple);

	/*
	 * Force synchronous commit, to minimize the window between removing the
	 * files on-disk and marking the transaction committed.  It's not great
	 * that there is any window at all, but definitely we don't want to make
	 * it larger than necessary.
	 */
	ForceSyncCommit();


	/* We keep the lock on pg_tablespace until commit */
	heap_close(rel, NoLock);
}

/*
 * get_prepared_txn_gid - given a transaction GID, look up the OID
 */
Oid
get_prepared_txn_gid(const char *gid)
{
	Oid				result;
	HeapTuple		tuple;
	ScanKeyData		entry[1];

	if (!YbPreparedXactCatalogExists)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("Prepared transaction system catalog does not "
				 		"exist.")));

	tuple = SearchSysCache1(YbPreparedXactRelationId, CStringGetDatum(gid));

	/* We assume that there can be at most one matching tuple */
	if (HeapTupleIsValid(tuple))
		result = HeapTupleGetOid(tuple);
	else
		result = InvalidOid;

	ReleaseSysCache(tuple);

	return result;
}

