/* -------------------------------------------------------------------------
 *
 * pg_constraint.cpp
 *	  routines to support manipulation of the pg_constraint relation
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/common/backend/catalog/pg_constraint.cpp
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"
#include "knl/knl_variable.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "access/sysattr.h"
#include "catalog/dependency.h"
#include "catalog/indexing.h"
#include "catalog/objectaccess.h"
#include "catalog/pg_constraint.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/rel_gs.h"
#include "utils/syscache.h"
#include "utils/snapmgr.h"

static void UpdateConstraintDependencyOn(
    Oid relId, Oid domainId, const int16* constraintKey, int constraintNTotalKeys, ObjectAddress* conobject)
{
    if (OidIsValid(relId)) {
        /*
         * Register auto dependency from constraint to owning relation, or to
         * specific column(s) if any are mentioned.
         */
        ObjectAddress relobject;

        relobject.classId = RelationRelationId;
        relobject.objectId = relId;
        if (constraintNTotalKeys > 0) {
            for (int i = 0; i < constraintNTotalKeys; i++) {
                relobject.objectSubId = constraintKey[i];
                recordDependencyOn(conobject, &relobject, DEPENDENCY_AUTO);
            }
        } else {
            relobject.objectSubId = 0;

            recordDependencyOn(conobject, &relobject, DEPENDENCY_AUTO);
        }
    }

    if (OidIsValid(domainId)) {
        /*
         * Register auto dependency from constraint to owning domain
         */
        ObjectAddress domobject;

        domobject.classId = TypeRelationId;
        domobject.objectId = domainId;
        domobject.objectSubId = 0;

        recordDependencyOn(conobject, &domobject, DEPENDENCY_AUTO);
    }
}

/*
 * CreateConstraintEntry
 *	Create a constraint table entry.
 *
 * Subsidiary records (such as triggers or indexes to implement the
 * constraint) are *not* created here.	But we do make dependency links
 * from the constraint to the things it depends on.
 */
Oid CreateConstraintEntry(const char* constraintName, Oid constraintNamespace, char constraintType, bool isDeferrable,
    bool isDeferred, bool isValidated, Oid relId, const int16* constraintKey, int constraintNKeys,
    int constraintNTotalKeys, Oid domainId, Oid indexRelId, Oid foreignRelId, const int16* foreignKey,
    const Oid* pfEqOp, const Oid* ppEqOp, const Oid* ffEqOp, int foreignNKeys, char foreignUpdateType,
    char foreignDeleteType, char foreignMatchType, const Oid* exclOp, Node* conExpr, const char* conBin,
    const char* conSrc, bool conIsLocal, int conInhCount, bool conNoInherit,
    InformationalConstraint* inforConstraint, bool condisable) /* @hdfs informatinal constaint */
{
    Relation conDesc = NULL;
    Oid conOid;
    HeapTuple tup = NULL;
    bool nulls[Natts_pg_constraint];
    Datum values[Natts_pg_constraint];
    ArrayType* conkeyArray = NULL;
    ArrayType* conincludingArray = NULL;
    ArrayType* confkeyArray = NULL;
    ArrayType* conpfeqopArray = NULL;
    ArrayType* conppeqopArray = NULL;
    ArrayType* conffeqopArray = NULL;
    ArrayType* conexclopArray = NULL;
    NameData cname;
    int i;
    ObjectAddress conobject;

    conDesc = heap_open(ConstraintRelationId, RowExclusiveLock);

    Assert(constraintName);
    (void)namestrcpy(&cname, constraintName);

    /*
     * Convert C arrays into openGauss arrays.
     */
    if (constraintNKeys > 0) {
        Datum* conkey = NULL;

        conkey = (Datum*)palloc(constraintNKeys * sizeof(Datum));
        for (i = 0; i < constraintNKeys; i++)
            conkey[i] = Int16GetDatum(constraintKey[i]);
        conkeyArray = construct_array(conkey, constraintNKeys, INT2OID, 2, true, 's');
    } else {
        conkeyArray = NULL;
    }

    if (constraintNTotalKeys > constraintNKeys) {
        int j = 0;
        int constraintNIncludedKeys = constraintNTotalKeys - constraintNKeys;
        Datum* conincluding = (Datum*)palloc(constraintNIncludedKeys * sizeof(Datum));
        for (i = constraintNKeys; i < constraintNTotalKeys; i++) {
            conincluding[j++] = Int16GetDatum(constraintKey[i]);
        }
        conincludingArray = construct_array(conincluding, constraintNIncludedKeys, INT2OID, 2, true, 's');
        pfree_ext(conincluding);
    } else {
        conincludingArray = NULL;
    }

    if (foreignNKeys > 0) {
        Datum* fkdatums = NULL;

        fkdatums = (Datum*)palloc(foreignNKeys * sizeof(Datum));
        for (i = 0; i < foreignNKeys; i++)
            fkdatums[i] = Int16GetDatum(foreignKey[i]);
        confkeyArray = construct_array(fkdatums, foreignNKeys, INT2OID, 2, true, 's');
        for (i = 0; i < foreignNKeys; i++)
            fkdatums[i] = ObjectIdGetDatum(pfEqOp[i]);
        conpfeqopArray = construct_array(fkdatums, foreignNKeys, OIDOID, sizeof(Oid), true, 'i');
        for (i = 0; i < foreignNKeys; i++)
            fkdatums[i] = ObjectIdGetDatum(ppEqOp[i]);
        conppeqopArray = construct_array(fkdatums, foreignNKeys, OIDOID, sizeof(Oid), true, 'i');
        for (i = 0; i < foreignNKeys; i++)
            fkdatums[i] = ObjectIdGetDatum(ffEqOp[i]);
        conffeqopArray = construct_array(fkdatums, foreignNKeys, OIDOID, sizeof(Oid), true, 'i');
    } else {
        confkeyArray = NULL;
        conpfeqopArray = NULL;
        conppeqopArray = NULL;
        conffeqopArray = NULL;
    }

    if (exclOp != NULL) {
        Datum* opdatums = NULL;

        opdatums = (Datum*)palloc(constraintNKeys * sizeof(Datum));
        for (i = 0; i < constraintNKeys; i++)
            opdatums[i] = ObjectIdGetDatum(exclOp[i]);
        conexclopArray = construct_array(opdatums, constraintNKeys, OIDOID, sizeof(Oid), true, 'i');
    } else
        conexclopArray = NULL;

    /* initialize nulls and values */
    for (i = 0; i < Natts_pg_constraint; i++) {
        nulls[i] = false;
        values[i] = (Datum)NULL;
    }

    values[Anum_pg_constraint_conname - 1] = NameGetDatum(&cname);
    values[Anum_pg_constraint_connamespace - 1] = ObjectIdGetDatum(constraintNamespace);
    values[Anum_pg_constraint_contype - 1] = CharGetDatum(constraintType);
    values[Anum_pg_constraint_condeferrable - 1] = BoolGetDatum(isDeferrable);
    values[Anum_pg_constraint_condeferred - 1] = BoolGetDatum(isDeferred);
    values[Anum_pg_constraint_convalidated - 1] = BoolGetDatum(isValidated);
    values[Anum_pg_constraint_conrelid - 1] = ObjectIdGetDatum(relId);
    values[Anum_pg_constraint_contypid - 1] = ObjectIdGetDatum(domainId);
    values[Anum_pg_constraint_conindid - 1] = ObjectIdGetDatum(indexRelId);
    values[Anum_pg_constraint_confrelid - 1] = ObjectIdGetDatum(foreignRelId);
    values[Anum_pg_constraint_confupdtype - 1] = CharGetDatum(foreignUpdateType);
    values[Anum_pg_constraint_confdeltype - 1] = CharGetDatum(foreignDeleteType);
    values[Anum_pg_constraint_confmatchtype - 1] = CharGetDatum(foreignMatchType);
    values[Anum_pg_constraint_conislocal - 1] = BoolGetDatum(conIsLocal);
    values[Anum_pg_constraint_coninhcount - 1] = Int32GetDatum(conInhCount);
    values[Anum_pg_constraint_connoinherit - 1] = BoolGetDatum(conNoInherit);
    values[Anum_pg_constraint_condisable - 1] = BoolGetDatum(condisable);
    if (inforConstraint == NULL) {
        values[Anum_pg_constraint_consoft - 1] = BoolGetDatum(false);
        values[Anum_pg_constraint_conopt - 1] = BoolGetDatum(false);
    } else {
        values[Anum_pg_constraint_consoft - 1] = BoolGetDatum(inforConstraint->nonforced);
        values[Anum_pg_constraint_conopt - 1] = BoolGetDatum(inforConstraint->enableOpt);
    }

    if (conkeyArray != NULL)
        values[Anum_pg_constraint_conkey - 1] = PointerGetDatum(conkeyArray);
    else
        nulls[Anum_pg_constraint_conkey - 1] = true;

    if (conincludingArray) {
        values[Anum_pg_constraint_conincluding - 1] = PointerGetDatum(conincludingArray);
    } else {
        nulls[Anum_pg_constraint_conincluding - 1] = true;
    }

    if (confkeyArray != NULL)
        values[Anum_pg_constraint_confkey - 1] = PointerGetDatum(confkeyArray);
    else
        nulls[Anum_pg_constraint_confkey - 1] = true;

    if (conpfeqopArray != NULL)
        values[Anum_pg_constraint_conpfeqop - 1] = PointerGetDatum(conpfeqopArray);
    else
        nulls[Anum_pg_constraint_conpfeqop - 1] = true;

    if (conppeqopArray != NULL)
        values[Anum_pg_constraint_conppeqop - 1] = PointerGetDatum(conppeqopArray);
    else
        nulls[Anum_pg_constraint_conppeqop - 1] = true;

    if (conffeqopArray != NULL)
        values[Anum_pg_constraint_conffeqop - 1] = PointerGetDatum(conffeqopArray);
    else
        nulls[Anum_pg_constraint_conffeqop - 1] = true;

    if (conexclopArray != NULL)
        values[Anum_pg_constraint_conexclop - 1] = PointerGetDatum(conexclopArray);
    else
        nulls[Anum_pg_constraint_conexclop - 1] = true;

    /*
     * initialize the binary form of the check constraint.
     */
    if (conBin != NULL)
        values[Anum_pg_constraint_conbin - 1] = CStringGetTextDatum(conBin);
    else
        nulls[Anum_pg_constraint_conbin - 1] = true;

    /*
     * initialize the text form of the check constraint
     */
    if (conSrc != NULL)
        values[Anum_pg_constraint_consrc - 1] = CStringGetTextDatum(conSrc);
    else
        nulls[Anum_pg_constraint_consrc - 1] = true;

    tup = heap_form_tuple(RelationGetDescr(conDesc), values, nulls);

    conOid = simple_heap_insert(conDesc, tup);

    /* update catalog indexes */
    CatalogUpdateIndexes(conDesc, tup);

    conobject.classId = ConstraintRelationId;
    conobject.objectId = conOid;
    conobject.objectSubId = 0;

    heap_close(conDesc, RowExclusiveLock);

    UpdateConstraintDependencyOn(relId, domainId, constraintKey, constraintNTotalKeys, &conobject);

    if (OidIsValid(foreignRelId)) {
        /*
         * Register normal dependency from constraint to foreign relation, or
         * to specific column(s) if any are mentioned.
         */
        ObjectAddress relobject;

        relobject.classId = RelationRelationId;
        relobject.objectId = foreignRelId;
        if (foreignNKeys > 0) {
            for (i = 0; i < foreignNKeys; i++) {
                relobject.objectSubId = foreignKey[i];

                recordDependencyOn(&conobject, &relobject, DEPENDENCY_NORMAL);
            }
        } else {
            relobject.objectSubId = 0;

            recordDependencyOn(&conobject, &relobject, DEPENDENCY_NORMAL);
        }
    }

    if (OidIsValid(indexRelId) && constraintType == CONSTRAINT_FOREIGN) {
        /*
         * Register normal dependency on the unique index that supports a
         * foreign-key constraint.	(Note: for indexes associated with unique
         * or primary-key constraints, the dependency runs the other way, and
         * is not made here.)
         */
        ObjectAddress relobject;

        relobject.classId = RelationRelationId;
        relobject.objectId = indexRelId;
        relobject.objectSubId = 0;

        recordDependencyOn(&conobject, &relobject, DEPENDENCY_NORMAL);
    }

    if (foreignNKeys > 0) {
        /*
         * Register normal dependencies on the equality operators that support
         * a foreign-key constraint.  If the PK and FK types are the same then
         * all three operators for a column are the same; otherwise they are
         * different.
         */
        ObjectAddress oprobject;

        oprobject.classId = OperatorRelationId;
        oprobject.objectSubId = 0;

        for (i = 0; i < foreignNKeys; i++) {
            oprobject.objectId = pfEqOp[i];
            recordDependencyOn(&conobject, &oprobject, DEPENDENCY_NORMAL);
            if (ppEqOp[i] != pfEqOp[i]) {
                oprobject.objectId = ppEqOp[i];
                recordDependencyOn(&conobject, &oprobject, DEPENDENCY_NORMAL);
            }
            if (ffEqOp[i] != pfEqOp[i]) {
                oprobject.objectId = ffEqOp[i];
                recordDependencyOn(&conobject, &oprobject, DEPENDENCY_NORMAL);
            }
        }
    }

    /*
     * We don't bother to register dependencies on the exclusion operators of
     * an exclusion constraint.  We assume they are members of the opclass
     * supporting the index, so there's an indirect dependency via that. (This
     * would be pretty dicey for cross-type operators, but exclusion operators
     * can never be cross-type.)
     */

    if (conExpr != NULL) {
        /*
         * Register dependencies from constraint to objects mentioned in CHECK
         * expression.
         */
        recordDependencyOnSingleRelExpr(&conobject, conExpr, relId, DEPENDENCY_NORMAL, DEPENDENCY_NORMAL);
    }

    /* Post creation hook for new constraint */
    InvokeObjectAccessHook(OAT_POST_CREATE, ConstraintRelationId, conOid, 0, NULL);

    return conOid;
}

/*
 * Test whether given name is currently used as a constraint name
 * for the given object (relation or domain).
 *
 * This is used to decide whether to accept a user-specified constraint name.
 * It is deliberately not the same test as ChooseConstraintName uses to decide
 * whether an auto-generated name is OK: here, we will allow it unless there
 * is an identical constraint name in use *on the same object*.
 *
 * NB: Caller should hold exclusive lock on the given object, else
 * this test can be fooled by concurrent additions.
 */
bool ConstraintNameIsUsed(ConstraintCategory conCat, Oid objId, Oid objNamespace, const char* conname)
{
    bool found = false;
    Relation conDesc = NULL;
    SysScanDesc conscan = NULL;
    ScanKeyData skey[2];
    HeapTuple tup = NULL;

    conDesc = heap_open(ConstraintRelationId, AccessShareLock);

    found = false;

    ScanKeyInit(&skey[0], Anum_pg_constraint_conname, BTEqualStrategyNumber, F_NAMEEQ, CStringGetDatum(conname));

    ScanKeyInit(
        &skey[1], Anum_pg_constraint_connamespace, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(objNamespace));

    conscan = systable_beginscan(conDesc, ConstraintNameNspIndexId, true, NULL, 2, skey);

    while (HeapTupleIsValid(tup = systable_getnext(conscan))) {
        Form_pg_constraint con = (Form_pg_constraint)GETSTRUCT(tup);

        if (conCat == CONSTRAINT_RELATION && con->conrelid == objId) {
            found = true;
            break;
        } else if (conCat == CONSTRAINT_DOMAIN && con->contypid == objId) {
            found = true;
            break;
        }
    }

    systable_endscan(conscan);
    heap_close(conDesc, AccessShareLock);

    return found;
}

/*
 * Select a nonconflicting name for a new constraint.
 *
 * The objective here is to choose a name that is unique within the
 * specified namespace.  openGauss does not require this, but the SQL
 * spec does, and some apps depend on it.  Therefore we avoid choosing
 * default names that so conflict.
 *
 * name1, name2, and label are used the same way as for makeObjectName(),
 * except that the label can't be NULL; digits will be appended to the label
 * if needed to create a name that is unique within the specified namespace.
 *
 * 'others' can be a list of string names already chosen within the current
 * command (but not yet reflected into the catalogs); we will not choose
 * a duplicate of one of these either.
 *
 * Note: it is theoretically possible to get a collision anyway, if someone
 * else chooses the same name concurrently.  This is fairly unlikely to be
 * a problem in practice, especially if one is holding an exclusive lock on
 * the relation identified by name1.
 *
 * Returns a palloc'd string.
 */
char* ChooseConstraintName(const char* name1, const char* name2, const char* label, Oid namespaceid, List* others)
{
    int pass = 0;
    char* conname = NULL;
    char modlabel[NAMEDATALEN];
    Relation conDesc;
    SysScanDesc conscan;
    ScanKeyData skey[2];
    bool found = false;
    ListCell* l = NULL;

    conDesc = heap_open(ConstraintRelationId, AccessShareLock);

    /* try the unmodified label first */
    errno_t rc = strncpy_s(modlabel, NAMEDATALEN, label, strlen(label));
    securec_check(rc, "\0", "\0");

    for (;;) {
        conname = makeObjectName(name1, name2, modlabel);

        found = false;

        foreach (l, others) {
            if (strcmp((char*)lfirst(l), conname) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            ScanKeyInit(
                &skey[0], Anum_pg_constraint_conname, BTEqualStrategyNumber, F_NAMEEQ, CStringGetDatum(conname));

            ScanKeyInit(&skey[1],
                Anum_pg_constraint_connamespace,
                BTEqualStrategyNumber,
                F_OIDEQ,
                ObjectIdGetDatum(namespaceid));

            conscan = systable_beginscan(conDesc, ConstraintNameNspIndexId, true, NULL, 2, skey);

            found = (HeapTupleIsValid(systable_getnext(conscan)));

            systable_endscan(conscan);
        }

        if (!found)
            break;

        /* found a conflict, so try a new name component */
        pfree_ext(conname);
        rc = snprintf_s(modlabel, sizeof(modlabel), sizeof(modlabel) - 1, "%s%d", label, ++pass);
        securec_check_ss(rc, "", "");
    }

    heap_close(conDesc, AccessShareLock);

    return conname;
}

/*
 * Delete a single constraint record.
 */
void RemoveConstraintById(Oid conId)
{
    Relation conDesc = NULL;
    HeapTuple tup = NULL;
    Form_pg_constraint con = NULL;

    conDesc = heap_open(ConstraintRelationId, RowExclusiveLock);

    tup = SearchSysCache1(CONSTROID, ObjectIdGetDatum(conId));
    if (!HeapTupleIsValid(tup)) /* should not happen */
        ereport(ERROR, (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for constraint %u", conId)));
    con = (Form_pg_constraint)GETSTRUCT(tup);

    /*
     * Special processing depending on what the constraint is for.
     */
    if (OidIsValid(con->conrelid)) {
        Relation rel = NULL;

        /*
         * If the constraint is for a relation, open and exclusive-lock the
         * relation it's for.
         */
        rel = heap_open(con->conrelid, AccessExclusiveLock);

        /*
         * We need to update the relcheck count if it is a check constraint
         * being dropped.  This update will force backends to rebuild relcache
         * entries when we commit.
         */
        if (con->contype == CONSTRAINT_CHECK) {
            Relation pgrel = NULL;
            HeapTuple relTup = NULL;
            Form_pg_class classForm = NULL;

            pgrel = heap_open(RelationRelationId, RowExclusiveLock);
            relTup = SearchSysCacheCopy1(RELOID, ObjectIdGetDatum(con->conrelid));
            if (!HeapTupleIsValid(relTup))
                ereport(ERROR,
                    (errcode(ERRCODE_CACHE_LOOKUP_FAILED),
                        errmsg("cache lookup failed for relation %u", con->conrelid)));
            classForm = (Form_pg_class)GETSTRUCT(relTup);

            if (classForm->relchecks == 0) /* should not happen */
                ereport(ERROR,
                    (errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
                        errmsg("relation \"%s\" has relchecks = 0", RelationGetRelationName(rel))));
            classForm->relchecks--;

            simple_heap_update(pgrel, &relTup->t_self, relTup);

            CatalogUpdateIndexes(pgrel, relTup);

            heap_freetuple_ext(relTup);

            heap_close(pgrel, RowExclusiveLock);
        }

        /* Keep lock on constraint's rel until end of xact */
        heap_close(rel, NoLock);
    } else if (OidIsValid(con->contypid)) {
        /*
         * XXX for now, do nothing special when dropping a domain constraint
         *
         * Probably there should be some form of locking on the domain type,
         * but we have no such concept at the moment.
         */
    } else
        ereport(
            ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("constraint %u is not of a known type", conId)));

    /* Fry the constraint itself */
    simple_heap_delete(conDesc, &tup->t_self);

    /* Clean up */
    ReleaseSysCache(tup);
    heap_close(conDesc, RowExclusiveLock);
}

/*
 * RenameConstraintById
 *		Rename a constraint.
 *
 * Note: this isn't intended to be a user-exposed function; it doesn't check
 * permissions etc.  Currently this is only invoked when renaming an index
 * that is associated with a constraint, but it's made a little more general
 * than that with the expectation of someday having ALTER TABLE RENAME
 * CONSTRAINT.
 */
void RenameConstraintById(Oid conId, const char* newname)
{
    Relation conDesc = NULL;
    HeapTuple tuple = NULL;
    Form_pg_constraint con = NULL;

    conDesc = heap_open(ConstraintRelationId, RowExclusiveLock);

    tuple = SearchSysCacheCopy1(CONSTROID, ObjectIdGetDatum(conId));
    if (!HeapTupleIsValid(tuple))
        ereport(ERROR, (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for constraint %u", conId)));
    con = (Form_pg_constraint)GETSTRUCT(tuple);

    /*
     * We need to check whether the name is already in use --- note that there
     * currently is not a unique index that would catch this.
     */
    if (OidIsValid(con->conrelid) &&
        ConstraintNameIsUsed(CONSTRAINT_RELATION, con->conrelid, con->connamespace, newname))
        ereport(ERROR,
            (errcode(ERRCODE_DUPLICATE_OBJECT),
                errmsg("constraint \"%s\" for relation \"%s\" already exists", newname, get_rel_name(con->conrelid))));
    if (OidIsValid(con->contypid) && ConstraintNameIsUsed(CONSTRAINT_DOMAIN, con->contypid, con->connamespace, newname))
        ereport(ERROR,
            (errcode(ERRCODE_DUPLICATE_OBJECT),
                errmsg("constraint \"%s\" for domain %s already exists", newname, format_type_be(con->contypid))));

    /* OK, do the rename --- tuple is a copy, so OK to scribble on it */
    (void)namestrcpy(&(con->conname), newname);

    simple_heap_update(conDesc, &tuple->t_self, tuple);

    /* update the system catalog indexes */
    CatalogUpdateIndexes(conDesc, tuple);

    heap_freetuple_ext(tuple);
    heap_close(conDesc, RowExclusiveLock);
}

/*
 * AlterConstraintNamespaces
 *		Find any constraints belonging to the specified object,
 *		and move them to the specified new namespace.
 *
 * isType indicates whether the owning object is a type or a relation.
 */
void AlterConstraintNamespaces(Oid ownerId, Oid oldNspId, Oid newNspId, bool isType, ObjectAddresses* objsMoved)
{
    Relation conRel = NULL;
    ScanKeyData key[1];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    conRel = heap_open(ConstraintRelationId, RowExclusiveLock);

    if (isType) {
        ScanKeyInit(&key[0], Anum_pg_constraint_contypid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(ownerId));

        scan = systable_beginscan(conRel, ConstraintTypidIndexId, true, NULL, 1, key);
    } else {
        ScanKeyInit(&key[0], Anum_pg_constraint_conrelid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(ownerId));

        scan = systable_beginscan(conRel, ConstraintRelidIndexId, true, NULL, 1, key);
    }

    while (HeapTupleIsValid((tup = systable_getnext(scan)))) {
        Form_pg_constraint conform = (Form_pg_constraint)GETSTRUCT(tup);
        ObjectAddress thisobj;

        thisobj.classId = ConstraintRelationId;
        thisobj.objectId = HeapTupleGetOid(tup);
        thisobj.objectSubId = 0;

        if (object_address_present(&thisobj, objsMoved))
            continue;

        if (conform->connamespace == oldNspId) {
            tup = heap_copytuple(tup);
            conform = (Form_pg_constraint)GETSTRUCT(tup);

            conform->connamespace = newNspId;

            simple_heap_update(conRel, &tup->t_self, tup);
            CatalogUpdateIndexes(conRel, tup);

            /*
             * Note: currently, the constraint will not have its own
             * dependency on the namespace, so we don't need to do
             * changeDependencyFor().
             */
        }

        add_exact_object_address(&thisobj, objsMoved);
    }

    systable_endscan(scan);

    heap_close(conRel, RowExclusiveLock);
}

/*
 * get_constraint_relation_oids
 *		Find the IDs of the relations to which a constraint refers.
 */
void get_constraint_relation_oids(Oid constraint_oid, Oid* conrelid, Oid* confrelid)
{
    HeapTuple tup = NULL;
    Form_pg_constraint con = NULL;

    tup = SearchSysCache1(CONSTROID, ObjectIdGetDatum(constraint_oid));
    if (!HeapTupleIsValid(tup)) /* should not happen */
    {
        ereport(ERROR,
            (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for constraint %u", constraint_oid)));
    }

    con = (Form_pg_constraint)GETSTRUCT(tup);
    *conrelid = con->conrelid;
    *confrelid = con->confrelid;
    ReleaseSysCache(tup);
}

/*
 * get_relation_constraint_oid
 *		Find a constraint on the specified relation with the specified name.
 *		Returns constraint's OID.
 */
Oid get_relation_constraint_oid(Oid relid, const char* conname, bool missing_ok)
{
    Relation pg_constraint;
    HeapTuple tuple = NULL;
    SysScanDesc scan = NULL;
    ScanKeyData skey[1];
    Oid conOid = InvalidOid;

    /*
     * Fetch the constraint tuple from pg_constraint.  There may be more than
     * one match, because constraints are not required to have unique names;
     * if so, error out.
     */
    pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

    ScanKeyInit(&skey[0], Anum_pg_constraint_conrelid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(relid));

    scan = systable_beginscan(pg_constraint, ConstraintRelidIndexId, true, NULL, 1, skey);

    while (HeapTupleIsValid(tuple = systable_getnext(scan))) {
        Form_pg_constraint con = (Form_pg_constraint)GETSTRUCT(tuple);

        if (strcmp(NameStr(con->conname), conname) == 0) {
            if (OidIsValid(conOid))
                ereport(ERROR,
                    (errcode(ERRCODE_DUPLICATE_OBJECT),
                        errmsg("table \"%s\" has multiple constraints named \"%s\"", get_rel_name(relid), conname)));
            conOid = HeapTupleGetOid(tuple);
        }
    }

    systable_endscan(scan);

    /* If no such constraint exists, complain */
    if (!OidIsValid(conOid) && !missing_ok)
        ereport(ERROR,
            (errcode(ERRCODE_UNDEFINED_OBJECT),
                errmsg("constraint \"%s\" for table \"%s\" does not exist", conname, get_rel_name(relid))));

    heap_close(pg_constraint, AccessShareLock);

    return conOid;
}

/*
 * get_domain_constraint_oid
 *		Find a constraint on the specified domain with the specified name.
 *		Returns constraint's OID.
 */
Oid get_domain_constraint_oid(Oid typid, const char* conname, bool missing_ok)
{
    Relation pg_constraint = NULL;
    HeapTuple tuple = NULL;
    SysScanDesc scan = NULL;
    ScanKeyData skey[1];
    Oid conOid = InvalidOid;

    /*
     * Fetch the constraint tuple from pg_constraint.  There may be more than
     * one match, because constraints are not required to have unique names;
     * if so, error out.
     */
    pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

    ScanKeyInit(&skey[0], Anum_pg_constraint_contypid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(typid));

    scan = systable_beginscan(pg_constraint, ConstraintTypidIndexId, true, NULL, 1, skey);

    while (HeapTupleIsValid(tuple = systable_getnext(scan))) {
        Form_pg_constraint con = (Form_pg_constraint)GETSTRUCT(tuple);

        if (strcmp(NameStr(con->conname), conname) == 0) {
            if (OidIsValid(conOid))
                ereport(ERROR,
                    (errcode(ERRCODE_DUPLICATE_OBJECT),
                        errmsg("domain \"%s\" has multiple constraints named \"%s\"", format_type_be(typid), conname)));
            conOid = HeapTupleGetOid(tuple);
        }
    }

    systable_endscan(scan);

    /* If no such constraint exists, complain */
    if (!OidIsValid(conOid) && !missing_ok)
        ereport(ERROR,
            (errcode(ERRCODE_UNDEFINED_OBJECT),
                errmsg("constraint \"%s\" for domain \"%s\" does not exist", conname, format_type_be(typid))));

    heap_close(pg_constraint, AccessShareLock);

    return conOid;
}

/*
 * Determine whether a relation can be proven functionally dependent on
 * a set of grouping columns.  If so, return TRUE and add the pg_constraint
 * OIDs of the constraints needed for the proof to the *constraintDeps list.
 *
 * grouping_columns is a list of grouping expressions, in which columns of
 * the rel of interest are Vars with the indicated varno/varlevelsup.
 *
 * Currently we only check to see if the rel has a primary key that is a
 * subset of the grouping_columns.	We could also use plain unique constraints
 * if all their columns are known not null, but there's a problem: we need
 * to be able to represent the not-null-ness as part of the constraints added
 * to *constraintDeps.
 */
bool check_functional_grouping(Oid relid, Index varno, Index varlevelsup, List* grouping_columns, List** constraintDeps)
{
    bool result = false;
    Relation pg_constraint = NULL;
    HeapTuple tuple = NULL;
    SysScanDesc scan = NULL;
    ScanKeyData skey[1];

    /* Scan pg_constraint for constraints of the target rel */
    pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

    ScanKeyInit(&skey[0], Anum_pg_constraint_conrelid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(relid));

    scan = systable_beginscan(pg_constraint, ConstraintRelidIndexId, true, NULL, 1, skey);

    while (HeapTupleIsValid(tuple = systable_getnext(scan))) {
        Form_pg_constraint con = (Form_pg_constraint)GETSTRUCT(tuple);
        Datum adatum;
        bool isNull = false;
        ArrayType* arr = NULL;
        int16* attnums = NULL;
        int numkeys;
        int i;
        bool found_col = false;

        /* Only PK constraints are of interest for now, see comment above */
        if (con->contype != CONSTRAINT_PRIMARY)
            continue;
        /* Constraint must be non-deferrable */
        if (con->condeferrable)
            continue;

        /* Extract the conkey array, ie, attnums of PK's columns */
        adatum = heap_getattr(tuple, Anum_pg_constraint_conkey, RelationGetDescr(pg_constraint), &isNull);
        if (isNull)
            ereport(ERROR,
                (errcode(ERRCODE_UNEXPECTED_NULL_VALUE),
                    errmsg("null conkey for constraint %u", HeapTupleGetOid(tuple))));
        arr = DatumGetArrayTypeP(adatum); /* ensure not toasted */
        numkeys = ARR_DIMS(arr)[0];
        if (ARR_NDIM(arr) != 1 || numkeys < 0 || ARR_HASNULL(arr) || ARR_ELEMTYPE(arr) != INT2OID)
            ereport(ERROR,
                (errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE), errmsg("conkey is not a 1-D smallint array")));
        attnums = (int16*)ARR_DATA_PTR(arr);

        found_col = false;
        for (i = 0; i < numkeys; i++) {
            AttrNumber attnum = attnums[i];
            ListCell* gl = NULL;

            found_col = false;
            foreach (gl, grouping_columns) {
                Var* gvar = (Var*)lfirst(gl);

                if (IsA(gvar, Var) && gvar->varno == varno && gvar->varlevelsup == varlevelsup &&
                    gvar->varattno == attnum) {
                    found_col = true;
                    break;
                }
            }
            if (!found_col)
                break;
        }

        if (found_col) {
            /* The PK is a subset of grouping_columns, so we win */
            *constraintDeps = lappend_oid(*constraintDeps, HeapTupleGetOid(tuple));
            result = true;
            break;
        }
    }

    systable_endscan(scan);

    heap_close(pg_constraint, AccessShareLock);

    return result;
}

/*
 * Get relid list of rels that has constraint dependency, from the pg_constraint oids.
 * @in conids: List of pg_constraint oids.
 * @return: List of relids.
 */
List* get_parse_dependency_rel_list(List* conids)
{
    List* result = NIL;
    ListCell* lc = NULL;
    Relation pg_constraint = NULL;
    HeapTuple tuple = NULL;
    SysScanDesc scan = NULL;
    ScanKeyData skey[1];

    if (conids == NIL)
        return result;

    /* Scan pg_constraint for constraints of the target rel */
    pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

    foreach (lc, conids) {
        Oid conid = lfirst_oid(lc);
        ScanKeyInit(&skey[0], ObjectIdAttributeNumber, BTEqualStrategyNumber, F_OIDEQ, conid);

        scan = systable_beginscan(pg_constraint, ConstraintOidIndexId, true, NULL, 1, skey);

        /* Only one record should be qualified, and get the relid */
        if (HeapTupleIsValid(tuple = systable_getnext(scan))) {
            Form_pg_constraint con = (Form_pg_constraint)GETSTRUCT(tuple);
            result = lappend_oid(result, con->conrelid);
        }

        systable_endscan(scan);
    }

    heap_close(pg_constraint, AccessShareLock);

    return result;
}

#ifdef PGXC
/*
 * pgxc_find_primarykey finds either primary key or unique index
 * defined for the passed relation.
 * Returns the number of columns in the primary key or unique index
 * ZERO means no primary key or unique index is defined.
 * The column attributes of the primary key or unique index are returned
 * in the passed indexed_col_numbers.
 * The function allocates space for indexed_col_numbers, the caller is
 * supposed to free it after use.
 * @in relid: relation oid.
 * @out indexed_col: index column  index.
 * @in check_is_immediate: Check index's indimmediate.
 */
int pgxc_find_primarykey(Oid relid, int16** indexed_col, bool check_is_immediate)
{
    HeapTuple indexTuple = NULL;
    HeapTuple primarykeyTuple = NULL;
    Form_pg_index indexStruct = NULL;
    ListCell* item = NULL;
    AttrNumber counter;
    Relation rel = NULL;

    /* Get necessary information about relation */
    rel = relation_open(relid, AccessShareLock);
    foreach (item, RelationGetIndexList(rel)) {
        Oid indexoid = lfirst_oid(item);

        indexTuple = SearchSysCache1(INDEXRELID, ObjectIdGetDatum(indexoid));
        if (!HeapTupleIsValid(indexTuple))
            ereport(
                ERROR, (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for index %u", indexoid)));

        indexStruct = (Form_pg_index)GETSTRUCT(indexTuple);

        if (!IndexIsValid(indexStruct)) {
            ReleaseSysCache(indexTuple);
            continue;
        }
        /*
         * This index can be returned  if uniqueness have't be enforced immediately. It can lead to
         * two or more row updated/deleted, that is not allow in not-stream plan, may be lead to system crash.
         */
        if (check_is_immediate && !indexStruct->indimmediate) {
            ReleaseSysCache(indexTuple);
            continue;
        }

        if (indexStruct->indisprimary) {
            primarykeyTuple = indexTuple;
            break;
        }

        if (indexStruct->indisunique) {
            int2vector* idxVec = &(indexStruct->indkey);
            bool equalPrimaryKey = true;

            Assert(idxVec->dim1 == indexStruct->indnatts);

            for (counter = 0; counter < indexStruct->indnatts; counter++) {
                AttrNumber idxKey = idxVec->values[counter];

                Assert(idxKey <= rel->rd_att->natts);

                /*
                 * 1.  skip expression index( it is an expression index when index attribute is zero )
                 * 2.  skip the idex if the index key is null
                 */
                if (!idxKey || !rel->rd_att->attrs[idxKey - 1].attnotnull) {
                    equalPrimaryKey = false;
                    break;
                }
            }

            if (equalPrimaryKey) {
                primarykeyTuple = indexTuple;
                break;
            }
        }

        ReleaseSysCache(indexTuple);
    }
    relation_close(rel, AccessShareLock);

    if (!primarykeyTuple)
        return 0;

    indexStruct = (Form_pg_index)GETSTRUCT(primarykeyTuple);
    *indexed_col = (int16*)palloc(indexStruct->indnatts * sizeof(AttrNumber));

    /*
     * Now get the list of PK attributes from the indkey definition (we
     * assume a primary key cannot have expressional elements)
     */
    for (counter = 0; counter < indexStruct->indnatts; counter++) {
        (*indexed_col)[counter] = indexStruct->indkey.values[counter];
    }

    ReleaseSysCache(primarykeyTuple);

    return indexStruct->indnatts;
}
#endif
