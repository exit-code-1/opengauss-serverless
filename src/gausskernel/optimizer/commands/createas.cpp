/* -------------------------------------------------------------------------
 *
 * createas.cpp
 *	  Execution of CREATE TABLE ... AS, a/k/a SELECT INTO
 *
 * We implement this by diverting the query's normal output to a
 * specialized DestReceiver type.
 *
 * Formerly, this command was implemented as a variant of SELECT, which led
 * to assorted legacy behaviors that we still try to preserve, notably that
 * we must return a tuples-processed count in the completionTag.
 *
 * Portions Copyright (c) 2020 Huawei Technologies Co.,Ltd.
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 * Portions Copyright (c) 2021, openGauss Contributors
 *
 *
 * IDENTIFICATION
 *	  src/gausskernel/optimizer/commands/createas.cpp
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"
#include "knl/knl_variable.h"

#include "access/reloptions.h"
#include "access/sysattr.h"
#include "access/tableam.h"
#include "access/xact.h"
#include "access/xlog.h"
#include "catalog/toasting.h"
#include "catalog/gs_matview.h"
#include "commands/createas.h"
#include "commands/matview.h"
#include "commands/prepare.h"
#include "commands/tablecmds.h"
#include "commands/view.h"
#include "optimizer/planner.h"
#include "parser/analyze.h"
#include "parser/parse_clause.h"
#include "parser/parse_utilcmd.h"
#include "rewrite/rewriteHandler.h"
#include "storage/smgr/smgr.h"
#include "tcop/tcopprot.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/rel_gs.h"
#include "utils/snapmgr.h"

typedef struct {
    DestReceiver pub; /* publicly-known function pointers */
    IntoClause* into; /* target relation specification */
    Query* viewParse; /* the query which defines/populates data */
    /* These fields are filled by intorel_startup: */
    Relation rel;            /* relation to write to */
    CommandId output_cid;    /* cmin to insert in output tuples */
    ObjectAddress reladdr;      /* address of rel, for ExecCreateTableAs */
    int hi_options;          /* heap_insert performance options */
    BulkInsertState bistate; /* bulk insert state */
} DR_intorel;

static void intorel_startup(DestReceiver* self, int operation, TupleDesc typeinfo);
static void intorel_receive(TupleTableSlot* slot, DestReceiver* self);
static void intorel_shutdown(DestReceiver* self);
static void intorel_destroy(DestReceiver* self);

/*
 * Common setup needed by both normal execution and EXPLAIN ANALYZE.
 */
Query *
SetupForCreateTableAs(Query *query, IntoClause *into, const char *queryString,
                      ParamListInfo params, DestReceiver *dest)
{
   List       *rewritten = NIL;
   Query      *viewParse = NULL;

   Assert(query->commandType == CMD_SELECT);

   if (into->relkind == RELKIND_MATVIEW) {
       viewParse = (Query *) parse_analyze((Node *) copyObject(query),
                                           queryString, NULL, 0)->utilityStmt;
   }

   /*
    * Parse analysis was done already, but we still have to run the rule
    * rewriter.  We do not do AcquireRewriteLocks: we assume the query either
    * came straight from the parser, or suitable locks were acquired by
    * plancache.c.
    */
   rewritten = QueryRewrite(query);

   /* SELECT should never rewrite to more or less than one SELECT query */
   if (list_length(rewritten) != 1) {
       elog(ERROR, "unexpected rewrite result for CREATE TABLE AS SELECT");
   }

   query = (Query *) linitial(rewritten);

   /* Save the query after rewrite but before planning. */
   ((DR_intorel *) dest)->viewParse = viewParse;
   ((DR_intorel *) dest)->into = into;

   if (into->relkind == RELKIND_MATVIEW)
   {
       /*
        * A materialized view would either need to save parameters for use in
        * maintaining or loading the data or prohibit them entirely. The
        * latter seems safer and more sane.
        */
       if (params != NULL && params->numParams > 0)
           ereport(ERROR,
                   (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                    errmsg("materialized views may not be defined using bound parameters")));
   }

   return query;
}

/*
 * ExecCreateTableAs -- execute a CREATE TABLE AS command
 */
ObjectAddress ExecCreateTableAs(CreateTableAsStmt* stmt, const char* queryString, ParamListInfo params, char* completionTag)
{
    Query* query = (Query*)stmt->query;
    IntoClause* into = stmt->into;
    DestReceiver* dest = NULL;
    PlannedStmt* plan = NULL;
    QueryDesc* queryDesc = NULL;
    ScanDirection dir;
    ObjectAddress address;
    
    if (stmt->into->ivm) {
        return ExecCreateMatViewInc(stmt, queryString, params);
    }

    /*
     * Create the tuple receiver object and insert info it will need
     */
    dest = CreateIntoRelDestReceiver(into);

    /*
     * The contained Query could be a SELECT, or an EXECUTE utility command.
     * If the latter, we just pass it off to ExecuteQuery.
     */
    Assert(IsA(query, Query));
    if (query->commandType == CMD_UTILITY && IsA(query->utilityStmt, ExecuteStmt)) {
        ExecuteStmt* estmt = (ExecuteStmt*)query->utilityStmt;

        ExecuteQuery(estmt, into, queryString, params, dest, completionTag);

        /* get object address that intorel_startup saved for us */
        address = ((DR_intorel *) dest)->reladdr;

        return address;
    }

    query = SetupForCreateTableAs(query, into, queryString, params, dest);

    /* plan the query */
    int nest_level = apply_set_hint(query);
    PG_TRY();
    {
        plan = pg_plan_query(query, 0, params);
    }
    PG_CATCH();
    {
        recover_set_hint(nest_level);
        PG_RE_THROW();
    }
    PG_END_TRY();

    recover_set_hint(nest_level);

    /*
     * Use a snapshot with an updated command ID to ensure this query sees
     * results of any previously executed queries.	(This could only matter if
     * the planner executed an allegedly-stable function that changed the
     * database contents, but let's do it anyway to be parallel to the EXPLAIN
     * code path.)
     */
    PushCopiedSnapshot(GetActiveSnapshot());
    UpdateActiveSnapshotCommandId();

    /* Create a QueryDesc, redirecting output to our tuple receiver */
    queryDesc = CreateQueryDesc(plan, queryString, GetActiveSnapshot(), InvalidSnapshot, dest, params, 0);

    if (ENABLE_WORKLOAD_CONTROL && (IS_PGXC_COORDINATOR || IS_SINGLE_NODE)) {
        /* Check if need track resource */
        u_sess->exec_cxt.need_track_resource = WLMNeedTrackResource(queryDesc);
    }

    /* call ExecutorStart to prepare the plan for execution */
    ExecutorStart(queryDesc, GetIntoRelEFlags(into));

    /* workload client manager */
    if (ENABLE_WORKLOAD_CONTROL) {
        WLMInitQueryPlan(queryDesc);
        dywlm_client_manager(queryDesc);
    }

    /*
     * Normally, we run the plan to completion; but if skipData is specified,
     * just do tuple receiver startup and shutdown.
     *
     * On DNs, normal materialized view creation will reach here. We only create the mv here.
     * Data population will be handled by a separate SelectInto.
     */
    if (into->skipData
#ifdef ENABLE_MULTIPLE_NODES
            || (IS_PGXC_DATANODE && stmt->relkind == OBJECT_MATVIEW))
#else
            )
#endif
        dir = NoMovementScanDirection;
    else
        dir = ForwardScanDirection;

    /* run the plan */
    ExecutorRun(queryDesc, dir, 0L);

    /* save the rowcount if we're given a completionTag to fill */
    if (completionTag != NULL
#ifndef ENABLE_MULTIPLE_NODES
        && stmt->relkind != OBJECT_MATVIEW
#endif
       ) {
        errno_t rc;
        rc = snprintf_s(completionTag,
            COMPLETION_TAG_BUFSIZE,
            COMPLETION_TAG_BUFSIZE - 1,
            "SELECT %lu",
            queryDesc->estate->es_processed);
        securec_check_ss(rc, "\0", "\0");
    }

    /* get object address that intorel_startup saved for us */
    address = ((DR_intorel *) dest)->reladdr;

    /* and clean up */
    ExecutorFinish(queryDesc);
    ExecutorEnd(queryDesc);

    FreeQueryDesc(queryDesc);

    PopActiveSnapshot();
    return address;
}

/*
 * GetIntoRelEFlags --- compute executor flags needed for CREATE TABLE AS
 *
 * This is exported because EXPLAIN and PREPARE need it too.  (Note: those
 * callers still need to deal explicitly with the skipData flag; since they
 * use different methods for suppressing execution, it doesn't seem worth
 * trying to encapsulate that part.)
 */
int GetIntoRelEFlags(IntoClause* intoClause)
{
    int     flags;

    /*
     * We need to tell the executor whether it has to produce OIDs or not,
     * because it doesn't have enough information to do so itself (since we
     * can't build the target relation until after ExecutorStart).
     */
    if (interpretOidsOption(intoClause->options))
        flags = EXEC_FLAG_WITH_OIDS;
    else
        flags = EXEC_FLAG_WITHOUT_OIDS;

    if (intoClause->skipData)
        flags |= EXEC_FLAG_WITH_NO_DATA;

    return flags;
}

/*
 * CreateIntoRelDestReceiver -- create a suitable DestReceiver object
 *
 * intoClause will be NULL if called from CreateDestReceiver(), in which
 * case it has to be provided later.  However, it is convenient to allow
 * self->into to be filled in immediately for other callers.
 */
DestReceiver* CreateIntoRelDestReceiver(IntoClause* intoClause)
{
    DR_intorel* self = (DR_intorel*)palloc0(sizeof(DR_intorel));

    self->pub.receiveSlot = intorel_receive;
    self->pub.rStartup = intorel_startup;
    self->pub.rShutdown = intorel_shutdown;
    self->pub.rDestroy = intorel_destroy;
    self->pub.mydest = DestIntoRel;
    self->pub.tmpContext = NULL;
    self->into = intoClause;
    /* other private fields will be set during intorel_startup */
    return (DestReceiver*)self;
}

/*
 * intorel_startup --- executor startup
 */
static void intorel_startup(DestReceiver* self, int operation, TupleDesc typeinfo)
{
    DR_intorel* myState = (DR_intorel*)self;
    IntoClause* into = myState->into;
    CreateStmt* create = NULL;
    Oid intoRelationId;
    ObjectAddress intoRelationAddr;
    Relation intoRelationDesc;
    RangeTblEntry* rte = NULL;
    Datum toast_options;
    ListCell* lc = NULL;
    int attnum;
    static const char* const validnsps[] = HEAP_RELOPT_NAMESPACES;

    Assert(into != NULL); /* else somebody forgot to set it */

    /*
     * Create the target relation by faking up a CREATE TABLE parsetree and
     * passing it to DefineRelation.
     */
    create = makeNode(CreateStmt);
    create->relation = into->rel;
    create->tableElts = NIL; /* will fill below */
    create->inhRelations = NIL;
    create->ofTypename = NULL;
    create->constraints = NIL;
    create->options = (List*)copyObject(into->options);
    create->oncommit = into->onCommit;
    create->row_compress = into->row_compress;
    create->tablespacename = into->tableSpaceName;
    create->if_not_exists = false;
    create->charset = PG_INVALID_ENCODING;

    /* Using Materialized view only */
    create->ivm = into->ivm;

    /*
     * Build column definitions using "pre-cooked" type and collation info. If
     * a column name list was specified in CREATE TABLE AS, override the
     * column names derived from the query.  (Too few column names are OK, too
     * many are not.)
     */
    lc = list_head(into->colNames);
    for (attnum = 0; attnum < typeinfo->natts; attnum++) {
        Form_pg_attribute attribute = &typeinfo->attrs[attnum];
        ColumnDef* col = makeNode(ColumnDef);
        TypeName* coltype = makeNode(TypeName);

        if (lc != NULL) {
            col->colname = strVal(lfirst(lc));
            lc = lnext(lc);
        } else
            col->colname = NameStr(attribute->attname);
        col->typname = coltype;
        col->inhcount = 0;
        col->is_local = true;
        col->is_not_null = false;
        col->is_from_type = false;
        col->storage = 0;
        col->generatedCol = '\0';
        col->kvtype = attribute->attkvtype;
        col->cmprs_mode = attribute->attcmprmode;
        col->raw_default = NULL;
        col->update_default = NULL;
        col->cooked_default = NULL;
        col->collClause = NULL;
        col->collOid = attribute->attcollation;
        col->constraints = NIL;
        col->fdwoptions = NIL;

        coltype->names = NIL;
        coltype->typeOid = attribute->atttypid;
        coltype->setof = false;
        coltype->pct_type = false;
        coltype->typmods = NIL;
        coltype->typemod = attribute->atttypmod;
        coltype->arrayBounds = NIL;
        coltype->location = -1;
        coltype->pct_rowtype = false;
        coltype->charset = get_charset_by_collation(attribute->attcollation);

        /*
         * It's possible that the column is of a collatable type but the
         * collation could not be resolved, so double-check.  (We must check
         * this here because DefineRelation would adopt the type's default
         * collation rather than complaining.)
         */
        if (!OidIsValid(col->collOid) && type_is_collatable(coltype->typeOid))
            ereport(ERROR,
                (errcode(ERRCODE_INDETERMINATE_COLLATION),
                    errmsg("no collation was derived for column \"%s\" with collatable type %s",
                        col->colname,
                        format_type_be(coltype->typeOid)),
                    errhint("Use the COLLATE clause to set the collation explicitly.")));

        create->tableElts = lappend(create->tableElts, col);
    }

    if (lc != NULL)
        ereport(ERROR,
                (errcode(ERRCODE_SYNTAX_ERROR),
                errmsg("too many column names are specified")));

    /*
     * Enforce validations needed for materialized views only.
     */
    if (into->relkind == RELKIND_MATVIEW)
    {
        /*
        * Prohibit a data-modifying CTE in the query used to create a
        * materialized view. It's not sufficiently clear what the user would
        * want to happen if the MV is refreshed or incrementally maintained.
        */
        if (myState->viewParse->hasModifyingCTE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("materialized views must not use data-modifying statements in WITH")));
    }

    /*
     * Actually create the target table
     */
    intoRelationAddr = DefineRelation(create, into->relkind, InvalidOid, NULL);
    intoRelationId = intoRelationAddr.objectId;
    /*
     * If necessary, create a TOAST table for the target table.  Note that
     * AlterTableCreateToastTable ends with CommandCounterIncrement(), so that
     * the TOAST table will be visible for insertion.
     */
    CommandCounterIncrement();

    /* parse and validate reloptions for the toast table */
    toast_options = transformRelOptions((Datum)0, create->options, "toast", validnsps, true, false);

    (void)heap_reloptions(RELKIND_TOASTVALUE, toast_options, true);

    AlterTableCreateToastTable(intoRelationId, toast_options);

    /* Create the "view" part of a materialized view. */
    if (into->relkind == RELKIND_MATVIEW)
    {
        StoreViewQuery(intoRelationId, myState->viewParse, false);
        CommandCounterIncrement();
    }

    /*
     * Finally we can open the target table
     */
    intoRelationDesc = heap_open(intoRelationId, AccessExclusiveLock);

    if (into->relkind == RELKIND_MATVIEW && !into->skipData) {
        /* Make sure the heap looks good even if no rows are written. */
        SetRelationIsScannable(intoRelationDesc);
    }

    /* insert catalog into pg_matview. */
    if (into->relkind == RELKIND_MATVIEW) {
        Oid mapid = InvalidOid;
        if (into->ivm) {
            /* build relationship map between matview-tup and rel-tup */
            mapid = create_matview_map(intoRelationId);

            /* build relationships of matviewid, relid and mlogid*/
            build_matview_dependency(intoRelationId, intoRelationDesc);
        }
        create_matview_tuple(intoRelationId, mapid, create->ivm);
    }

    /*
     * Check INSERT permission on the constructed table.
     *
     * XXX: It would arguably make sense to skip this check if into->skipData
     * is true.
     */
    rte = makeNode(RangeTblEntry);
    rte->rtekind = RTE_RELATION;
    rte->relid = intoRelationId;
    rte->relkind = into->relkind;
    rte->isResultRel = true;
    rte->requiredPerms = ACL_INSERT;

    for (attnum = 1; attnum <= intoRelationDesc->rd_att->natts; attnum++)
        rte->insertedCols = bms_add_member(rte->insertedCols, attnum - FirstLowInvalidHeapAttributeNumber);

    (void)ExecCheckRTPerms(list_make1(rte), true);

    /*
     * Fill private fields of myState for use by later routines
     */
    myState->rel = intoRelationDesc;
    myState->output_cid = GetCurrentCommandId(true);
    myState->reladdr = intoRelationAddr;
    /*
     * We can skip WAL-logging the insertions, unless PITR or streaming
     * replication is in use. We can skip the FSM in any case.
     */
    myState->hi_options = TABLE_INSERT_SKIP_FSM | (XLogIsNeeded() ? 0 : TABLE_INSERT_SKIP_WAL);
    myState->bistate = GetBulkInsertState();

    /* Not using WAL requires smgr_targblock be initially invalid */
    Assert(RelationGetTargetBlock(intoRelationDesc) == InvalidBlockNumber);
}

/*
 * intorel_receive --- receive one tuple
 */
static void intorel_receive(TupleTableSlot* slot, DestReceiver* self)
{
    DR_intorel* myState = (DR_intorel*)self;
    HeapTuple tuple;

    /*
     * get the heap tuple out of the tuple table slot, making sure we have a
     * writable copy
     */
    tuple = ExecMaterializeSlot(slot);

    /*
     * force assignment of new OID (see comments in ExecInsert)
     */
    if (myState->rel->rd_rel->relhasoids)
        HeapTupleSetOid(tuple, InvalidOid);

    (void)tableam_tuple_insert(myState->rel, tuple, 
        myState->output_cid, myState->hi_options, myState->bistate);
    /* We know this is a newly created relation, so there are no indexes */
}

/*
 * intorel_shutdown --- executor end
 */
static void intorel_shutdown(DestReceiver* self)
{
    DR_intorel* myState = (DR_intorel*)self;

    FreeBulkInsertState(myState->bistate);

    /* If we skipped using WAL, must heap_sync before commit */
    if (((myState->hi_options & TABLE_INSERT_SKIP_WAL) || enable_heap_bcm_data_replication())
        && !RelationIsSegmentTable(myState->rel))
        heap_sync(myState->rel);

    /* close rel, but keep lock until commit */
    heap_close(myState->rel, NoLock);
    myState->rel = NULL;
}

/*
 * intorel_destroy --- release DestReceiver object
 */
static void intorel_destroy(DestReceiver* self)
{
    pfree_ext(self);
}
