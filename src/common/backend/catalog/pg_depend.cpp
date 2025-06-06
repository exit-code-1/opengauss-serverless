/* -------------------------------------------------------------------------
 *
 * pg_depend.cpp
 *	  routines to support manipulation of the pg_depend relation
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 * Portions Copyright (c) 2021, openGauss Contributors
 *
 *
 * IDENTIFICATION
 *	  src/common/backend/catalog/pg_depend.cpp
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"
#include "knl/knl_variable.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "access/transam.h"
#include "catalog/dependency.h"
#include "catalog/gs_package.h"
#include "catalog/indexing.h"
#include "catalog/pg_constraint.h"
#include "catalog/pg_depend.h"
#include "catalog/pg_extension.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_synonym.h"
#include "commands/extension.h"
#include "miscadmin.h"
#include "storage/tcap.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/rel_gs.h"
#include "utils/snapmgr.h"
#include "catalog/pg_proc.h"
#include "catalog/pg_rewrite.h"
#include "catalog/gs_dependencies_fn.h"
#include "catalog/gs_dependencies_obj.h"
#include "catalog/pg_object.h"

static bool isObjectPinned(const ObjectAddress* object, Relation rel);

/*
 * Record a dependency between 2 objects via their respective objectAddress.
 * The first argument is the dependent object, the second the one it
 * references.
 *
 * This simply creates an entry in pg_depend, without any other processing.
 */
void recordDependencyOn(const ObjectAddress* depender, const ObjectAddress* referenced, DependencyType behavior,
    GsDependParamBody* gsdependParamBody)
{
    recordMultipleDependencies(depender, referenced, 1, behavior, gsdependParamBody);
}

/*
 * Record multiple dependencies (of the same kind) for a single dependent
 * object.	This has a little less overhead than recording each separately.
 */
void recordMultipleDependencies(
    const ObjectAddress* depender, const ObjectAddress* referenced, int nreferenced, DependencyType behavior,
    GsDependParamBody* gsdependParamBody)
{
    Relation dependDesc;
    CatalogIndexState indstate;
    HeapTuple tup;
    int i;
    bool nulls[Natts_pg_depend];
    Datum values[Natts_pg_depend];

    if (nreferenced <= 0)
        return; /* nothing to do */

    /*
     * During bootstrap, do nothing since pg_depend may not exist yet. initdb
     * will fill in appropriate pg_depend entries after bootstrap.
     */
    if (IsBootstrapProcessingMode())
        return;

    dependDesc = heap_open(DependRelationId, RowExclusiveLock);

    /* Don't open indexes unless we need to make an update */
    indstate = NULL;

    errno_t rc = memset_s(nulls, sizeof(nulls), false, sizeof(nulls));
    securec_check(rc, "", "");

    TrForbidAccessRbDependencies(dependDesc, depender, referenced, nreferenced);

    for (i = 0; i < nreferenced; i++, referenced++) {
        /*
         * If the referenced object is pinned by the system, there's no real
         * need to record dependencies on it.  This saves lots of space in
         * pg_depend, so it's worth the time taken to check.
         */
        bool isPinned = isObjectPinned(referenced, dependDesc);
        if (enable_plpgsql_gsdependency() && DEPENDENCY_NORMAL == behavior &&
            gsplsql_need_build_gs_dependency(gsdependParamBody, referenced, isPinned)) {
            bool ret = gsplsql_build_gs_type_dependency(gsdependParamBody, referenced);
            if (ret) {
                gsdependParamBody->hasDependency = true;
            }
        }
        if (isPinned) {
            continue;
        }
        if (!enable_plpgsql_gsdependency() || NULL == gsdependParamBody ||
            (GSDEPEND_REFOBJ_POS_IN_TYPE == gsdependParamBody->refPosType
             && GSDEPEND_OBJECT_TYPE_TYPE == gsdependParamBody->type)) {
            /*
             * Record the Dependency.  Note we don't bother to check for
             * duplicate dependencies; there's no harm in them.
             */
            values[Anum_pg_depend_classid - 1] = ObjectIdGetDatum(depender->classId);
            values[Anum_pg_depend_objid - 1] = ObjectIdGetDatum(depender->objectId);
            values[Anum_pg_depend_objsubid - 1] = Int32GetDatum(depender->objectSubId);

            values[Anum_pg_depend_refclassid - 1] = ObjectIdGetDatum(referenced->classId);
            values[Anum_pg_depend_refobjid - 1] = ObjectIdGetDatum(referenced->objectId);
            values[Anum_pg_depend_refobjsubid - 1] = Int32GetDatum(referenced->objectSubId);

            values[Anum_pg_depend_deptype - 1] = CharGetDatum((char)behavior);

            tup = heap_form_tuple(dependDesc->rd_att, values, nulls);

            (void)simple_heap_insert(dependDesc, tup);

            /* keep indexes current */
            if (indstate == NULL)
                indstate = CatalogOpenIndexes(dependDesc);

            CatalogIndexInsert(indstate, tup);

            heap_freetuple_ext(tup);
        }
    }

    if (indstate != NULL)
        CatalogCloseIndexes(indstate);

    heap_close(dependDesc, RowExclusiveLock);
}

/*
 * If we are executing a CREATE EXTENSION operation, mark the given object
 * as being a member of the extension.	Otherwise, do nothing.
 *
 * This must be called during creation of any user-definable object type
 * that could be a member of an extension.
 *
 * If isReplace is true, the object already existed (or might have already
 * existed), so we must check for a pre-existing extension membership entry.
 * Passing false is a guarantee that the object is newly created, and so
 * could not already be a member of any extension.
 */
void recordDependencyOnCurrentExtension(const ObjectAddress* object, bool isReplace)
{
    /* Only whole objects can be extension members */
    Assert(object->objectSubId == 0);

    if (creating_extension) {
        ObjectAddress extension;

        /* Only need to check for existing membership if isReplace */
        if (isReplace) {
            Oid oldext;

            oldext = getExtensionOfObject(object->classId, object->objectId);
            if (OidIsValid(oldext)) {
                /* If already a member of this extension, nothing to do */
                if (oldext == u_sess->cmd_cxt.CurrentExtensionObject)
                    return;
                /* Already a member of some other extension, so reject */
                ereport(ERROR,
                    (errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
                        errmsg("%s is already a member of extension \"%s\"",
                            getObjectDescription(object),
                            get_extension_name(oldext))));
            }

            /* It's a free-standing object, so reject */
            ereport(ERROR,
                (errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
                    errmsg("%s is not a member of extension \"%s\"",
                        getObjectDescription(object),
                        get_extension_name(u_sess->cmd_cxt.CurrentExtensionObject)),
                        errdetail("An extension is not allowed to replace an object that it does not own.")));
        }

        /* OK, record it as a member of CurrentExtensionObject */
        extension.classId = ExtensionRelationId;
        extension.objectId = u_sess->cmd_cxt.CurrentExtensionObject;
        extension.objectSubId = 0;

        recordDependencyOn(object, &extension, DEPENDENCY_EXTENSION);
    }
}

/*
 * If we are executing a CREATE EXTENSION operation, check that the given
 * object is a member of the extension, and throw an error if it isn't.
 * Otherwise, do nothing.
 *
 * This must be called whenever a CREATE IF NOT EXISTS operation (for an
 * object type that can be an extension member) has found that an object of
 * the desired name already exists.  It is insecure for an extension to use
 * IF NOT EXISTS except when the conflicting object is already an extension
 * member; otherwise a hostile user could substitute an object with arbitrary
 * properties.
 */
void checkMembershipInCurrentExtension(const ObjectAddress *object)
{
	/*
	 * This is actually the same condition tested in
	 * recordDependencyOnCurrentExtension; but we want to issue a
	 * differently-worded error, and anyway it would be pretty confusing to
	 * call recordDependencyOnCurrentExtension in these circumstances.
	 */

	/* Only whole objects can be extension members */
	Assert(object->objectSubId == 0);

	if (creating_extension) {
            Oid oldext;

            oldext = getExtensionOfObject(object->classId, object->objectId);
            /* If already a member of this extension, OK */
            if (oldext == u_sess->cmd_cxt.CurrentExtensionObject)
                return;
            /* Else complain */
            ereport(ERROR,
                (errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
                    errmsg("%s is not a member of extension \"%s\"",
                        getObjectDescription(object),
                        get_extension_name(u_sess->cmd_cxt.CurrentExtensionObject)),
                        errdetail("An extension may only use CREATE ... IF NOT EXISTS to skip object creation"
                            "if the conflicting object is one that it already owns.")));
	}
}


/*
 * Record pinned dependency for  fabricated system tables during in-place upgrade.
 */
void recordPinnedDependency(const ObjectAddress* object)
{
    Relation dependDesc;
    CatalogIndexState indstate;
    HeapTuple tup;
    bool nulls[Natts_pg_depend];
    Datum values[Natts_pg_depend];
    errno_t errorno = EOK;

    /*
     * During bootstrap, do nothing since pg_depend may not exist yet. initdb
     * will fill in appropriate pg_depend entries after bootstrap.
     */
    if (IsBootstrapProcessingMode())
        return;

    dependDesc = heap_open(DependRelationId, RowExclusiveLock);

    /* Don't open indexes unless we need to make an update */
    indstate = NULL;

    errorno = memset_s(nulls, sizeof(nulls), false, sizeof(nulls));
    securec_check(errorno, "\0", "\0");

    /*
     * Record the Dependency.  Note we don't bother to check for
     * duplicate dependencies; there's no harm in them.
     */
    values[Anum_pg_depend_classid - 1] = ObjectIdGetDatum(InvalidOid);
    values[Anum_pg_depend_objid - 1] = ObjectIdGetDatum(InvalidOid);
    values[Anum_pg_depend_objsubid - 1] = Int32GetDatum(InvalidOid);

    values[Anum_pg_depend_refclassid - 1] = ObjectIdGetDatum(object->classId);
    values[Anum_pg_depend_refobjid - 1] = ObjectIdGetDatum(object->objectId);
    values[Anum_pg_depend_refobjsubid - 1] = Int32GetDatum(object->objectSubId);

    values[Anum_pg_depend_deptype - 1] = CharGetDatum((char)DEPENDENCY_PIN);

    tup = heap_form_tuple(dependDesc->rd_att, values, nulls);

    (void)simple_heap_insert(dependDesc, tup);

    /* keep indexes current */
    if (indstate == NULL)
        indstate = CatalogOpenIndexes(dependDesc);

    CatalogIndexInsert(indstate, tup);

    heap_freetuple_ext(tup);

    if (indstate != NULL)
        CatalogCloseIndexes(indstate);

    heap_close(dependDesc, RowExclusiveLock);
}

/*
 * deleteDependencyRecordsFor -- delete all records with given depender
 * classId/objectId.  Returns the number of records deleted.
 *
 * This is used when redefining an existing object.  Links leading to the
 * object do not change, and links leading from it will be recreated
 * (possibly with some differences from before).
 *
 * If skipExtensionDeps is true, we do not delete any dependencies that
 * show that the given object is a member of an extension.	This avoids
 * needing a lot of extra logic to fetch and recreate that dependency.
 */
long deleteDependencyRecordsFor(Oid classId, Oid objectId, bool skipExtensionDeps)
{
    long count = 0;
    Relation depRel;
    ScanKeyData key[2];
    SysScanDesc scan;
    HeapTuple tup;

    depRel = heap_open(DependRelationId, RowExclusiveLock);

    ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(classId));
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(objectId));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        if (skipExtensionDeps && ((Form_pg_depend)GETSTRUCT(tup))->deptype == DEPENDENCY_EXTENSION)
            continue;

        simple_heap_delete(depRel, &tup->t_self);
        count++;
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);

    CommandCounterIncrement();

    return count;
}

/*
 * deleteDependencyRecordsForClass -- delete all records with given depender
 * classId/objectId, dependee classId, and deptype.
 * Returns the number of records deleted.
 *
 * This is a variant of deleteDependencyRecordsFor, useful when revoking
 * an object property that is expressed by a dependency record (such as
 * extension membership).
 */
long deleteDependencyRecordsForClass(Oid classId, Oid objectId, Oid refclassId, char deptype)
{
    long count = 0;
    Relation depRel = NULL;
    ScanKeyData key[2];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    depRel = heap_open(DependRelationId, RowExclusiveLock);

    ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(classId));
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(objectId));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        Form_pg_depend depform = (Form_pg_depend)GETSTRUCT(tup);

        if (depform->refclassid == refclassId && depform->deptype == deptype) {
            simple_heap_delete(depRel, &tup->t_self);
            count++;
        }
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);

    return count;
}

/*
 * Adjust dependency record(s) to point to a different object of the same type
 *
 * classId/objectId specify the referencing object.
 * refClassId/oldRefObjectId specify the old referenced object.
 * newRefObjectId is the new referenced object (must be of class refClassId).
 *
 * Note the lack of objsubid parameters.  If there are subobject references
 * they will all be readjusted.
 *
 * Returns the number of records updated.
 */
long changeDependencyFor(Oid classId, Oid objectId, Oid refClassId, Oid oldRefObjectId, Oid newRefObjectId)
{
    long count = 0;
    Relation depRel = NULL;
    ScanKeyData key[2];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;
    ObjectAddress objAddr;
    bool newIsPinned = false;

    depRel = heap_open(DependRelationId, RowExclusiveLock);

    /*
     * If oldRefObjectId is pinned, there won't be any dependency entries on
     * it --- we can't cope in that case.  (This isn't really worth expending
     * code to fix, in current usage; it just means you can't rename stuff out
     * of pg_catalog, which would likely be a bad move anyway.)
     */
    objAddr.classId = refClassId;
    objAddr.objectId = oldRefObjectId;
    objAddr.objectSubId = 0;

    if (isObjectPinned(&objAddr, depRel))
        ereport(ERROR,
            (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                errmsg(
                    "cannot remove dependency on %s because it is a system object", getObjectDescription(&objAddr))));

    /*
     * We can handle adding a dependency on something pinned, though, since
     * that just means deleting the dependency entry.
     */
    objAddr.objectId = newRefObjectId;

    newIsPinned = isObjectPinned(&objAddr, depRel);

    /* Now search for dependency records */
    ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(classId));
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(objectId));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid((tup = systable_getnext(scan)))) {
        Form_pg_depend depform = (Form_pg_depend)GETSTRUCT(tup);

        if (depform->refclassid == refClassId && depform->refobjid == oldRefObjectId) {
            if (newIsPinned)
                simple_heap_delete(depRel, &tup->t_self);
            else {
                /* make a modifiable copy */
                tup = heap_copytuple(tup);
                depform = (Form_pg_depend)GETSTRUCT(tup);

                depform->refobjid = newRefObjectId;

                simple_heap_update(depRel, &tup->t_self, tup);
                CatalogUpdateIndexes(depRel, tup);

                heap_freetuple_ext(tup);
            }

            count++;
        }
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);

    return count;
}

bool IsPinnedObject(Oid classOid, Oid objOid) {
    ObjectAddress objAddr;
    objAddr.classId = classOid;
    objAddr.objectId = objOid;
    objAddr.objectSubId = 0;
    Relation dependDesc = heap_open(DependRelationId, AccessShareLock);
    bool isPinned = isObjectPinned(&objAddr, dependDesc);
    heap_close(dependDesc, AccessShareLock);
    return isPinned;
}

/*
 * Adjust all dependency records to come from a different object of the same type
 *
 * classId/oldObjectId specify the old referencing object.
 * newObjectId is the new referencing object (must be of class classId).
 */
void changeDependenciesOf(Oid classId, Oid oldObjectId, Oid newObjectId)
{
    Relation    depRel;
    ScanKeyData key[2];
    SysScanDesc scan;
    HeapTuple   tup;

    depRel = heap_open(DependRelationId, RowExclusiveLock);

    ScanKeyInit(&key[0],
                Anum_pg_depend_classid,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(classId));
    ScanKeyInit(&key[1],
                Anum_pg_depend_objid,
                BTEqualStrategyNumber, F_OIDEQ,
                ObjectIdGetDatum(oldObjectId));

    scan = systable_beginscan(depRel, DependDependerIndexId, true,
                              NULL, 2, key);

    while (HeapTupleIsValid((tup = systable_getnext(scan))))
    {
        Form_pg_depend depform = (Form_pg_depend) GETSTRUCT(tup);

        /* make a modifiable copy */
        tup = heap_copytuple(tup);
        depform = (Form_pg_depend) GETSTRUCT(tup);

        depform->objid = newObjectId;

        simple_heap_update(depRel, &tup->t_self, tup);
        CatalogUpdateIndexes(depRel, tup);

        heap_freetuple_ext(tup);
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);
}

/*
 * Adjust all dependency records to point to a different object of the same type
 *
 * refClassId/oldRefObjectId specify the old referenced object.
 * newRefObjectId is the new referenced object (must be of class refClassId).
 */
void changeDependenciesOn(Oid refClassId, Oid oldRefObjectId, Oid newRefObjectId)
{
    Relation depRel = NULL;
    ScanKeyData key[2];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;
    ObjectAddress objAddr;
    bool newIsPinned = false;

    depRel = heap_open(DependRelationId, RowExclusiveLock);
    /*
     * If oldRefObjectId is pinned, there won't be any dependency entries on
     * it --- we can't cope in that case. (This isn't really worth expending
     * code to fix, in current usage; it just means you can't rename stuff out
     * of pg_catalog, which would likely be a bad move anyway.)
     */
    objAddr.classId = refClassId;
    objAddr.objectId = oldRefObjectId;
    objAddr.objectSubId = 0;

    if (isObjectPinned(&objAddr, depRel))
        ereport (ERROR, 
            (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                errmsg("cannot remove dependency on %s because it is a system object",
                    getObjectDescription(&objAddr))));

    /*
     * We can handle adding a dependency on something pinned, though, since
     * that just means deleting the dependency entry.
     */
    objAddr.objectId = newRefObjectId;

    newIsPinned = isObjectPinned(&objAddr, depRel);

    /* Now search for dependency records */
    ScanKeyInit(&key[0], Anum_pg_depend_refclassid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(refClassId));
    ScanKeyInit(&key[1], Anum_pg_depend_refobjid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(oldRefObjectId));

    scan = systable_beginscan(depRel, DependReferenceIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan)))
    {
        Form_pg_depend depform = (Form_pg_depend) GETSTRUCT(tup);

        if(newIsPinned)
            simple_heap_delete(depRel, &tup->t_self);
        else {
            /* make a modifiable copy */
            tup = heap_copytuple(tup);
            depform = (Form_pg_depend) GETSTRUCT(tup);

            depform->refobjid = newRefObjectId;

            simple_heap_update(depRel, &tup->t_self, tup);
            CatalogUpdateIndexes(depRel, tup);

            heap_freetuple_ext(tup);
        }
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);
}

/*
 * isObjectPinned()
 *
 * Test if an object is required for basic database functionality.
 * Caller must already have opened pg_depend.
 *
 * The passed subId, if any, is ignored; we assume that only whole objects
 * are pinned (and that this implies pinning their components).
 */
static bool isObjectPinned(const ObjectAddress* object, Relation rel)
{
    bool ret = false;
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;
    ScanKeyData key[2];

    /* we need to handle the builtin functions in pg_depend here,if the object is builtin function,we treat it as it
     * pinned in pg_proc because we delete them from pg_proc
     */
    if (object->classId == ProcedureRelationId && IsSystemObjOid(object->objectId)) {
        return true;
    }

    ScanKeyInit(&key[0], Anum_pg_depend_refclassid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(object->classId));

    ScanKeyInit(&key[1], Anum_pg_depend_refobjid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(object->objectId));

    scan = systable_beginscan(rel, DependReferenceIndexId, true, NULL, 2, key);

    /*
     * Since we won't generate additional pg_depend entries for pinned
     * objects, there can be at most one entry referencing a pinned object.
     * Hence, it's sufficient to look at the first returned tuple; we don't
     * need to loop.
     */
    tup = systable_getnext(scan);
    if (HeapTupleIsValid(tup)) {
        Form_pg_depend foundDep = (Form_pg_depend)GETSTRUCT(tup);

        if (foundDep->deptype == DEPENDENCY_PIN)
            ret = true;
    }

    systable_endscan(scan);

    return ret;
}

/*
 * Various special-purpose lookups and manipulations of pg_depend.
 */

/*
 * Find the extension containing the specified object, if any
 *
 * Returns the OID of the extension, or InvalidOid if the object does not
 * belong to any extension.
 *
 * Extension membership is marked by an EXTENSION dependency from the object
 * to the extension.  Note that the result will be indeterminate if pg_depend
 * contains links from this object to more than one extension ... but that
 * should never happen.
 */
Oid getExtensionOfObject(Oid classId, Oid objectId)
{
    Oid result = InvalidOid;
    Relation depRel = NULL;
    ScanKeyData key[2];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    depRel = heap_open(DependRelationId, AccessShareLock);

    ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(classId));
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(objectId));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid((tup = systable_getnext(scan)))) {
        Form_pg_depend depform = (Form_pg_depend)GETSTRUCT(tup);

        if (depform->refclassid == ExtensionRelationId && depform->deptype == DEPENDENCY_EXTENSION) {
            result = depform->refobjid;
            break; /* no need to keep scanning */
        }
    }

    systable_endscan(scan);

    heap_close(depRel, AccessShareLock);

    return result;
}

/*
 * Detect whether a sequence is marked as "owned" by a column
 *
 * An ownership marker is an AUTO dependency from the sequence to the
 * column.	If we find one, store the identity of the owning column
 * into *tableId and *colId and return TRUE; else return FALSE.
 *
 * Note: if there's more than one such pg_depend entry then you get
 * a random one of them returned into the out parameters.  This should
 * not happen, though.
 */
bool sequenceIsOwned(Oid seqId, Oid* tableId, int32* colId)
{
    bool ret = false;
    Relation depRel = NULL;
    ScanKeyData key[2];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    depRel = heap_open(DependRelationId, AccessShareLock);

    ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(RelationRelationId));
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(seqId));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid((tup = systable_getnext(scan)))) {
        Form_pg_depend depform = (Form_pg_depend)GETSTRUCT(tup);

        if (depform->refclassid == RelationRelationId && depform->deptype == DEPENDENCY_AUTO) {
            *tableId = depform->refobjid;
            *colId = depform->refobjsubid;
            ret = true;
            break; /* no need to keep scanning */
        }
    }

    systable_endscan(scan);

    heap_close(depRel, AccessShareLock);

    return ret;
}

/*
 * Remove any existing "owned" markers for the specified sequence.
 *
 * Note: we don't provide a special function to install an "owned"
 * marker; just use recordDependencyOn().
 */
void markSequenceUnowned(Oid seqId)
{
    (void)deleteDependencyRecordsForClass(RelationRelationId, seqId, RelationRelationId, DEPENDENCY_AUTO);
}

/*
 * Collect a list of OIDs of all sequences owned by the specified relation.
 */
List* getOwnedSequences(Oid relid, List* attrList)
{
    List* result = NIL;
    Relation depRel = NULL;
    ScanKeyData key[2];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    depRel = heap_open(DependRelationId, AccessShareLock);

    ScanKeyInit(
        &key[0], Anum_pg_depend_refclassid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(RelationRelationId));
    ScanKeyInit(&key[1], Anum_pg_depend_refobjid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(relid));

    scan = systable_beginscan(depRel, DependReferenceIndexId, true, NULL, 2, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        Form_pg_depend deprec = (Form_pg_depend)GETSTRUCT(tup);

        /*
         * We assume any auto dependency of a sequence on a column must be
         * what we are looking for.  (We need the relkind test because indexes
         * can also have auto dependencies on columns.)
         */
        if (deprec->classid == RelationRelationId && deprec->objsubid == 0 && deprec->refobjsubid != 0 &&
            deprec->deptype == DEPENDENCY_AUTO && RELKIND_IS_SEQUENCE(get_rel_relkind(deprec->objid))) {
            if (attrList != NULL) {
                if (list_member_int(attrList, deprec->refobjsubid))
                    result = lappend_oid(result, deprec->objid);
            } else {
                result = lappend_oid(result, deprec->objid);
            }
        }
    }

    systable_endscan(scan);

    heap_close(depRel, AccessShareLock);

    return result;
}

/*
 * get_constraint_index
 *		Given the OID of a unique or primary-key constraint, return the
 *		OID of the underlying unique index.
 *
 * Return InvalidOid if the index couldn't be found; this suggests the
 * given OID is bogus, but we leave it to caller to decide what to do.
 */
Oid get_constraint_index(Oid constraintId)
{
    Oid indexId = InvalidOid;
    Relation depRel = NULL;
    ScanKeyData key[3];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;
    char relkind;

    /* Search the dependency table for the dependent index */
    depRel = heap_open(DependRelationId, AccessShareLock);

    ScanKeyInit(
        &key[0], Anum_pg_depend_refclassid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(ConstraintRelationId));
    ScanKeyInit(&key[1], Anum_pg_depend_refobjid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(constraintId));
    ScanKeyInit(&key[2], Anum_pg_depend_refobjsubid, BTEqualStrategyNumber, F_INT4EQ, Int32GetDatum(0));

    scan = systable_beginscan(depRel, DependReferenceIndexId, true, NULL, 3, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        Form_pg_depend deprec = (Form_pg_depend)GETSTRUCT(tup);

        /*
         * We assume any internal dependency of an index on the constraint
         * must be what we are looking for.  (The relkind test is just
         * paranoia; there shouldn't be any such dependencies otherwise.)
         */
        if (deprec->classid == RelationRelationId && deprec->objsubid == 0 && deprec->deptype == DEPENDENCY_INTERNAL) {
            relkind = get_rel_relkind(deprec->objid);
            if (relkind == RELKIND_INDEX || relkind == RELKIND_GLOBAL_INDEX) {
                indexId = deprec->objid;
                break;
            }
        }
    }

    systable_endscan(scan);
    heap_close(depRel, AccessShareLock);

    return indexId;
}

/*
 * get_index_constraint
 *		Given the OID of an index, return the OID of the owning unique or
 *		primary-key constraint, or InvalidOid if no such constraint.
 */
Oid get_index_constraint(Oid indexId)
{
    Oid constraintId = InvalidOid;
    Relation depRel = NULL;
    ScanKeyData key[3];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    /* Search the dependency table for the index */
    depRel = heap_open(DependRelationId, AccessShareLock);

    ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(RelationRelationId));
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(indexId));
    ScanKeyInit(&key[2], Anum_pg_depend_objsubid, BTEqualStrategyNumber, F_INT4EQ, Int32GetDatum(0));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, 3, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        Form_pg_depend deprec = (Form_pg_depend)GETSTRUCT(tup);

        /*
         * We assume any internal dependency on a constraint must be what we
         * are looking for.
         */
        if (deprec->refclassid == ConstraintRelationId && deprec->refobjsubid == 0 &&
            deprec->deptype == DEPENDENCY_INTERNAL) {
            constraintId = deprec->refobjid;
            break;
        }
    }

    systable_endscan(scan);
    heap_close(depRel, AccessShareLock);

    return constraintId;
}

/*
 * get_index_ref_constraints
 *     Given the OID of an index, return the OID of all foreign key
 *     constraints which reference the index.
 */
List* get_index_ref_constraints(Oid indexId)
{
    List* result = NULL;
    Relation depRel = NULL;
    ScanKeyData key[3];
    SysScanDesc scan = NULL;
    HeapTuple tup = NULL;

    /* search the dependency table for the index */
    depRel = heap_open(DependRelationId,AccessShareLock);

    ScanKeyInit(&key[0], Anum_pg_depend_refclassid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(RelationRelationId));
    ScanKeyInit(&key[1], Anum_pg_depend_refobjid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(indexId));
    ScanKeyInit(&key[2], Anum_pg_depend_refobjsubid, BTEqualStrategyNumber, F_INT4EQ, Int32GetDatum(0));

    scan = systable_beginscan(depRel, DependReferenceIndexId, true, NULL, 3, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan)))
    {
        Form_pg_depend deprec = (Form_pg_depend) GETSTRUCT(tup);

        /*
         * We assume any normal dependency from a constraint must be what we
         * are looking for.
         */
        if (deprec->classid == ConstraintRelationId && deprec->objsubid == 0 &&
            deprec->deptype == DEPENDENCY_NORMAL)
        {
            result = lappend_oid(result, deprec->objid);
        }
    }

    systable_endscan(scan);
    heap_close(depRel, AccessShareLock);

    return result;
}

long DeleteTypesDenpendOnPackage(Oid classId, Oid objectId, bool isSpec)
{
    long count = 0;
    Relation depRel;
    ScanKeyData key[2];
    SysScanDesc scan;
    HeapTuple tup;
    ObjectAddresses* objects = new_object_addresses();
    ObjectAddress address;
    const int keyNumber = 2;
    char* typName = NULL;
    bool isPkgDepTyp = false;
    Form_pg_depend depTuple = NULL;

    depRel = heap_open(DependRelationId, RowExclusiveLock);

    ScanKeyInit(&key[0], Anum_pg_depend_refclassid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(classId));
    ScanKeyInit(&key[1], Anum_pg_depend_refobjid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(objectId));

    scan = systable_beginscan(depRel, DependReferenceIndexId, true, NULL, keyNumber, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        depTuple = (Form_pg_depend)GETSTRUCT(tup);
        isPkgDepTyp = (depTuple->deptype == DEPENDENCY_AUTO) &&
            (depTuple->classid == TypeRelationId || depTuple->classid == PgSynonymRelationId);
        if (!isPkgDepTyp) {
            continue;
        }

        if (!isSpec) {
            /* body just delete private type */
            if (depTuple->classid == TypeRelationId) {
                typName = get_typename(depTuple->objid);
            } else {
                typName = pstrdup(GetQualifiedSynonymName(depTuple->objid, false));
            }
            if (typName[0] != '$') {
                pfree(typName);
                continue;
            }
            pfree(typName);
        }

        address.classId = depTuple->classid;
        address.objectId = depTuple->objid;
        address.objectSubId = 0;
        add_exact_object_address(&address, objects);
        count++;
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);

    CommandCounterIncrement();
    performMultipleDeletions(objects, DROP_CASCADE, PERFORM_DELETION_INTERNAL, true);

    return count;
}

bool IsPackageDependType(Oid typOid, Oid pkgOid, bool isRefCur)
{
    Relation depRel;
    ScanKeyData key[2];
    SysScanDesc scan;
    HeapTuple tup;
    const int keyNumber = 2;
    bool isFind = false;

    depRel = heap_open(DependRelationId, RowExclusiveLock);
    if (isRefCur) {
        ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber,
            F_OIDEQ, ObjectIdGetDatum(PgSynonymRelationId));
    } else {
        ScanKeyInit(&key[0], Anum_pg_depend_classid, BTEqualStrategyNumber,
            F_OIDEQ, ObjectIdGetDatum(TypeRelationId));
    }
    ScanKeyInit(&key[1], Anum_pg_depend_objid, BTEqualStrategyNumber,
        F_OIDEQ, ObjectIdGetDatum(typOid));

    scan = systable_beginscan(depRel, DependDependerIndexId, true, NULL, keyNumber, key);

    while (HeapTupleIsValid(tup = systable_getnext(scan))) {
        if (((Form_pg_depend)GETSTRUCT(tup))->deptype == DEPENDENCY_AUTO) {
            if (OidIsValid(pkgOid)) {
                if (((Form_pg_depend)GETSTRUCT(tup))->refobjid == pkgOid) {
                    isFind = true;
                    break;
                }
            } else {
                if (((Form_pg_depend)GETSTRUCT(tup))->refclassid == PackageRelationId
                    || ((Form_pg_depend)GETSTRUCT(tup))->refclassid == ProcedureRelationId) {
                    isFind = true;
                    break;
                }
            }
        }
    }

    systable_endscan(scan);

    heap_close(depRel, RowExclusiveLock);

    return isFind;
}

void DeletePgDependObject(const ObjectAddress* object, const ObjectAddress* ref_object)
{
    int keyNum = 0;
    ScanKeyData key[2];
    ScanKeyInit(&key[keyNum++], Anum_pg_depend_classid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(object->classId));
    ScanKeyInit(&key[keyNum++], Anum_pg_depend_objid, BTEqualStrategyNumber, F_OIDEQ, ObjectIdGetDatum(object->objectId));
    Relation relation = heap_open(DependRelationId, RowExclusiveLock);
    SysScanDesc scan = systable_beginscan(relation, DependDependerIndexId, true, NULL, keyNum, key);
    bool is_null = false;
    HeapTuple tuple;
    while (HeapTupleIsValid(tuple = systable_getnext(scan))) {
        
        Datum refclassid_datum = heap_getattr(tuple, Anum_pg_depend_refclassid,
                                               RelationGetDescr(relation), &is_null);
        Datum refobjid_datum = heap_getattr(tuple, Anum_pg_depend_refobjid,
                                        RelationGetDescr(relation), &is_null);
        if (ref_object->classId == DatumGetObjectId(refclassid_datum) &&
            ref_object->objectId == DatumGetObjectId(refobjid_datum)) {
            simple_heap_delete(relation, &tuple->t_self);
            systable_endscan(scan);
            heap_close(relation, RowExclusiveLock);
            CommandCounterIncrement();
            return;
        }
    }
    systable_endscan(scan);
    heap_close(relation, RowExclusiveLock);
}