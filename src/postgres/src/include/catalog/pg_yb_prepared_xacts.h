/*-------------------------------------------------------------------------
 *
 * pg_yb_prepared_xacts.h
 *	  definition of the "prepared transactions" system catalog
 *	  (pg_yb_prepared_xacts)
 *
 *
 * Copyright (c) YugaByte, Inc.
 *
 * src/include/catalog/pg_yb_prepared_xacts.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_YB_PREPARED_XACTS_H
#define PG_YB_PREPARED_XACTS_H

#include "catalog/genbki.h"
#include "catalog/pg_yb_prepared_xacts_d.h"

/* ----------------
 *		pg_yb_prepared_xacts definition.  cpp turns this into
 *		typedef struct FormData_pg_yb_prepared_xacts
 * ----------------
 */
CATALOG(pg_yb_prepared_xacts,8040,YbPreparedXactsRelationId) BKI_ROWTYPE_OID(8042,YbPreparedXactsRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
	TransactionId 	transaction_uuid; 	/* transaction id */
	char[GIDSIZE]	global_name;		/* transaction name */
	TimestampTz 	prepared_at;		/* time of preparation */
	Oid				ownerid;			/* owner of transaction */
	Oid				dbid;				/* database of transaction */
} FormData_pg_yb_prepared_xacts;

/* ----------------
 *		Form_pg_yb_prepared_xacts corresponds to a pointer to a tuple with
 *		the format of pg_yb_prepared_xacts relation.
 * ----------------
 */
typedef FormData_pg_yb_prepared_xacts *Form_pg_yb_prepared_xacts;

#endif							/* PG_YB_PREPARED_XACTS_H */
