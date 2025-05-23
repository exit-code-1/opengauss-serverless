/* -------------------------------------------------------------------------
 *
 * packagecmds.cpp
 *
 *	  Routines for CREATE and DROP PACKAGE commands and CREATE and DROP
 *	  CAST commands.
 *
 * Portions Copyright (c) 2021 Huawei Technologies Co.,Ltd.
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/gausskernel/optimizer/commands/functioncmds.cpp
 *
 * DESCRIPTION
 *	  These routines take the parse tree and pick out the
 *	  appropriate arguments/flags, and pass the results to the
 *	  corresponding "FooDefine" routines (in src/catalog) that do
 *	  the actual catalog-munging.  These routines also verify permission
 *	  of the user to execute the command.
 *
 * NOTES
 *	  These things must be defined and committed in the following order:
 *		"create package":
 *				input/output, recv/send procedures
 *		"create type":
 *				type
 *		"create operator":
 *				operators
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"
#include "knl/knl_variable.h"

#include "access/heapam.h"
#include "access/tableam.h"
#include "access/transam.h"
#include "catalog/dependency.h"
#include "catalog/indexing.h"
#include "catalog/namespace.h"
#include "catalog/objectaccess.h"
#include "catalog/pg_control.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_object.h"
#include "catalog/gs_package.h"
#include "catalog/gs_package_fn.h"
#include "catalog/gs_db_privilege.h"
#include "commands/defrem.h"
#include "commands/typecmds.h"
#include "pgxc/pgxc.h"
#include "storage/tcap.h"
#include "utils/acl.h"
#include "utils/builtins.h"
#include "utils/rel.h"
#include "utils/rel_gs.h"
#include "utils/syscache.h"
#include "utils/lsyscache.h"
#include "tcop/utility.h"


static void create_package_error_callback(void *arg)
{
    CreatePackageStmt* stmt = (CreatePackageStmt*)arg;
    int cur_pos;

    if (stmt->pkgspec_location <= 0)
        return;

    cur_pos = geterrposition();
    if (cur_pos > 0)
    {
        cur_pos += (stmt->pkgspec_location - stmt->pkgspec_prefix_len);
        errposition(cur_pos);
    }
}

void CreatePackageCommand(CreatePackageStmt* stmt, const char* queryString)
{
#ifdef ENABLE_MULTIPLE_NODES
        ereport(ERROR, (errcode(ERRCODE_INVALID_PACKAGE_DEFINITION),
                    errmsg("not support create package in distributed database")));
#endif
    u_sess->plsql_cxt.debug_query_string = pstrdup(queryString);
    Oid packageId;
    Oid namespaceId;
    char* pkgname = NULL;
    char* pkgspecsrc = NULL;
    Oid pkgOwner;
    ErrorContextCallback err_context;

    /* Convert list of names to a name and namespace */
    namespaceId = QualifiedNameGetCreationNamespace(stmt->pkgname, &pkgname);
    if (!OidIsValid(namespaceId)) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PACKAGE_DEFINITION),
                    errmsg("Invalid namespace"))); 
    }

    if (u_sess->attr.attr_sql.sql_compatibility != A_FORMAT) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PACKAGE_DEFINITION),
                    errmsg("Package only allowed create in A compatibility"))); 
    }
    /*
     * isAlter is true, change the owner of the objects as the owner of the
     * namespace, if the owner of the namespce has the same name as the namescpe
     */
    bool isAlter = false;
    bool anyResult = CheckCreatePrivilegeInNamespace(namespaceId, GetUserId(), CREATE_ANY_PACKAGE);

    //@Temp Table. Lock Cluster after determine whether is a temp object,
    // so we can decide if locking other coordinator
    pgxc_lock_for_utility_stmt((Node*)stmt, namespaceId == u_sess->catalog_cxt.myTempNamespace);

    pkgspecsrc = stmt->pkgspec;

    if (u_sess->attr.attr_sql.enforce_a_behavior) {
        pkgOwner = GetUserIdFromNspId(namespaceId, false, anyResult);

        if (!OidIsValid(pkgOwner))
            pkgOwner = GetUserId();

        else if (pkgOwner != GetUserId())
            isAlter = true;

        if (isAlter) {
            (void)CheckCreatePrivilegeInNamespace(namespaceId, pkgOwner, CREATE_ANY_PACKAGE);
        }
    } else {
        pkgOwner = GetUserId();
    }

    /* Create the package */
    if (pkgname == NULL) {
        ereport(ERROR,
            (errmodule(MOD_PLSQL), errcode(ERRCODE_INVALID_NAME),
                errmsg("package name is null"),
                errdetail("empty package name is not allowed"),
                errcause("package name is null"),
                erraction("rename package")));
    }

    err_context.arg = (void*)stmt;
    err_context.previous = t_thrd.log_cxt.error_context_stack;
    err_context.callback = create_package_error_callback;
    t_thrd.log_cxt.error_context_stack = &err_context;
    packageId = PackageSpecCreate(namespaceId, pkgname, pkgOwner, pkgspecsrc, stmt->replace, stmt->pkgsecdef);
    t_thrd.log_cxt.error_context_stack = err_context.previous;

    /* Advance cmd counter to make the package visible */
    if (OidIsValid(packageId)) {
        CommandCounterIncrement();
    } else {
        ereport(ERROR,
            (errmodule(MOD_PLSQL), errcode(ERRCODE_INVALID_NAME),
                errmsg("package spec has error"),
                errdetail("empty package spec is not allowed"),
                errcause("debug mode"),
                erraction("check package spec")));   
    }
    if (u_sess->plsql_cxt.debug_query_string) {
        pfree_ext(u_sess->plsql_cxt.debug_query_string);
    }
}


void CreatePackageBodyCommand(CreatePackageBodyStmt* stmt, const char* queryString)
{
#ifdef ENABLE_MULTIPLE_NODES
        ereport(ERROR, (errcode(ERRCODE_INVALID_PACKAGE_DEFINITION),
                    errmsg("not support create package in distributed database")));
#endif
    u_sess->plsql_cxt.debug_query_string = pstrdup(queryString);
    //Oid packageId;
    Oid namespaceId;
    char* pkgname = NULL;
    char* pkgBodySrc = NULL;
    Oid pkgOwner;

    /* Convert list of names to a name and namespace */
    namespaceId = QualifiedNameGetCreationNamespace(stmt->pkgname, &pkgname);

    /*
     * isAlter is true, change the owner of the objects as the owner of the
     * namespace, if the owner of the namespce has the same name as the namescpe
     */
    bool isAlter = false;
    bool anyResult = CheckCreatePrivilegeInNamespace(namespaceId, GetUserId(), CREATE_ANY_PACKAGE);

    //@Temp Table. Lock Cluster after determine whether is a temp object,
    // so we can decide if locking other coordinator
    pgxc_lock_for_utility_stmt((Node*)stmt, namespaceId == u_sess->catalog_cxt.myTempNamespace);

    pkgBodySrc = stmt->pkgbody;

    if (u_sess->attr.attr_sql.enforce_a_behavior) {
        pkgOwner = GetUserIdFromNspId(namespaceId, false, anyResult);

        if (!OidIsValid(pkgOwner))
            pkgOwner = GetUserId();
        else if (pkgOwner != GetUserId())
            isAlter = true;

        if (isAlter) {
            (void)CheckCreatePrivilegeInNamespace(namespaceId, pkgOwner, CREATE_ANY_PACKAGE);
        }
    } else {
        pkgOwner = GetUserId();
    }

    /* Create the package */
    Oid packageOid = InvalidOid;
    packageOid = PackageBodyCreate(namespaceId, pkgname, pkgOwner, pkgBodySrc , stmt->pkginit, stmt->replace);

    /* Advance cmd counter to make the package visible */
    if (OidIsValid(packageOid)) {
        CommandCounterIncrement();
    } else {
        ereport(ERROR,
            (errmodule(MOD_PLSQL), errcode(ERRCODE_INVALID_NAME),
                errmsg("package body has error"),
                errdetail("empty package body is not allowed"),
                errcause("package body is null"),
                erraction("check package body")));   
    }
    if (u_sess->plsql_cxt.debug_query_string) {
        pfree_ext(u_sess->plsql_cxt.debug_query_string);
    }
}

static inline void clear_plsql_ctx_line_info()
{
    u_sess->plsql_cxt.procedure_first_line = 0;
    u_sess->plsql_cxt.procedure_start_line = 0;
    u_sess->plsql_cxt.package_first_line = 0;
    u_sess->plsql_cxt.package_as_line = 0;
}

static void RecompileSinglePackage(Oid package_oid, bool is_spec)
{
    Oid* save_pseudo_current_user_id = u_sess->misc_cxt.Pseudo_CurrentUserId;
    _PG_init();
    PLpgSQL_compile_context* save_compile_context = u_sess->plsql_cxt.curr_compile_context;
    int save_compile_list_length = list_length(u_sess->plsql_cxt.compile_context_list);
    int save_compile_status = getCompileStatus();
    bool save_need_create_depend = u_sess->plsql_cxt.need_create_depend;
    u_sess->plsql_cxt.isCreateFunction = false;
    u_sess->plsql_cxt.compile_has_warning_info = false;
    int save_searchpath_stack = list_length(u_sess->catalog_cxt.overrideStack);
    PG_TRY();
    {
        u_sess->plsql_cxt.createPlsqlType = CREATE_PLSQL_TYPE_RECOMPILE;
        if (GetPgObjectValid(package_oid, is_spec ? OBJECT_TYPE_PKGSPEC : OBJECT_TYPE_PKGBODY)) {
            u_sess->plsql_cxt.need_create_depend = false;
        } else {
            u_sess->plsql_cxt.need_create_depend = true;
        }
        SetCurrCompilePgObjStatus(true);
        (void)plpgsql_pkg_compile(package_oid, true, is_spec, false, true);
        u_sess->misc_cxt.Pseudo_CurrentUserId = save_pseudo_current_user_id;
        u_sess->plsql_cxt.need_create_depend = save_need_create_depend;
        if (enable_plpgsql_gsdependency_guc()) {
            u_sess->plsql_cxt.createPlsqlType = CREATE_PLSQL_TYPE_END;
        }
        SetCurrCompilePgObjStatus(true);
    }
    PG_CATCH();
    {
        if (enable_plpgsql_gsdependency_guc()) {
            u_sess->plsql_cxt.createPlsqlType = CREATE_PLSQL_TYPE_END;
        }
        SetCurrCompilePgObjStatus(true);
        u_sess->plsql_cxt.curr_compile_context = save_compile_context;
        u_sess->plsql_cxt.compile_status = save_compile_status;
        u_sess->plsql_cxt.need_create_depend = save_need_create_depend;
        u_sess->plsql_cxt.during_compile = false;
        clearCompileContextList(save_compile_list_length);
        u_sess->plsql_cxt.running_pkg_oid = InvalidOid;
        clear_plsql_ctx_line_info();
        u_sess->plsql_cxt.in_package_function_compile = false;
        u_sess->misc_cxt.Pseudo_CurrentUserId = save_pseudo_current_user_id;
        plpgsql_free_override_stack(save_searchpath_stack);
        PG_RE_THROW();
    }
    PG_END_TRY();
}

static inline void RecompilePackageByOid(Oid pkg_oid)
{
    bool is_null;
    HeapTuple pkg_tup = SearchSysCache1(PACKAGEOID, ObjectIdGetDatum(pkg_oid));
    if (!HeapTupleIsValid(pkg_tup)) {
        ereport(ERROR,  (errcode(ERRCODE_CACHE_LOOKUP_FAILED),
                        errdetail("cache lookup failed for package %u, while recompile package.", pkg_oid)));
        return;
    }
    SysCacheGetAttr(PACKAGEOID, pkg_tup, Anum_gs_package_pkgbodydeclsrc, &is_null);
    if (is_null) {
        ReleaseSysCache(pkg_tup);
        RecompileSinglePackage(pkg_oid, true);
        return;
    }
    ReleaseSysCache(pkg_tup);
    RecompileSinglePackage(pkg_oid, false);
}

void RecompilePackage(CompileStmt* stmt)
{
    Oid pkg_oid = PackageNameListGetOid(stmt->objName, false, false);
    if (IsSystemObjOid(pkg_oid)) {
        return;
    }
    switch (stmt->compileItem) {
        case COMPILE_PKG_SPECIFICATION:
            RecompileSinglePackage(pkg_oid, true);
            break;
        case COMPILE_PKG_BODY:
            RecompileSinglePackage(pkg_oid, false);
            break;
        default:
            RecompilePackageByOid(pkg_oid);
            break;
    }
}

/*
 * Change package owner by name
 */
ObjectAddress AlterPackageOwner(List* name, Oid newOwnerId)
{
#ifdef ENABLE_MULTIPLE_NODES
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
        errmsg("package not supported in distributed database")));
#endif
    Oid pkgOid = PackageNameListGetOid(name, false, false);
    Relation rel;
    HeapTuple tup;
    ObjectAddress address;
    if (IsSystemObjOid(pkgOid)) {
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_PACKAGE_DEFINITION),
                errmsg("ownerId change failed for package %u, because it is a builtin package.", pkgOid)));
    }
    rel = heap_open(PackageRelationId, RowExclusiveLock);
    tup = SearchSysCache1(PACKAGEOID, ObjectIdGetDatum(pkgOid));
    /* should not happen */
    if (!HeapTupleIsValid(tup)) {
        ereport(
            ERROR, (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for package %u", pkgOid)));
    }

    TrForbidAccessRbObject(PACKAGEOID, pkgOid);

    Form_gs_package gs_package_tuple = (Form_gs_package)GETSTRUCT(tup);
    /*
     * If the new owner is the same as the existing owner, consider the
     * command to have succeeded.  This is for dump restoration purposes.
     */
    if (gs_package_tuple->pkgowner == newOwnerId) {
        ReleaseSysCache(tup);
        /* Recode time of change the funciton owner. */
        UpdatePgObjectMtime(pkgOid, OBJECT_TYPE_PKGSPEC);
        heap_close(rel, NoLock);
        ObjectAddressSet(address, PackageRelationId, pkgOid);
        return address;
    }

    Datum repl_val[Natts_gs_package];
    bool repl_null[Natts_gs_package];
    bool repl_repl[Natts_gs_package];
    Acl* newAcl = NULL;
    Datum aclDatum;
    bool isNull = false;
    HeapTuple newtuple;
    AclResult aclresult;

    /* Superusers can always do it */
    if (!superuser()) {
        /* Otherwise, must be owner of the existing object */
        if (!pg_package_ownercheck(pkgOid, GetUserId()))
            aclcheck_error(ACLCHECK_NOT_OWNER, ACL_KIND_PACKAGE, NameStr(gs_package_tuple->pkgname));

        /* Must be able to become new owner */
        check_is_member_of_role(GetUserId(), newOwnerId);

        /* New owner must have CREATE privilege on namespace */
        aclresult = pg_namespace_aclcheck(gs_package_tuple->pkgnamespace, newOwnerId, ACL_CREATE);
        if (aclresult != ACLCHECK_OK)
            aclcheck_error(aclresult, ACL_KIND_NAMESPACE, get_namespace_name(gs_package_tuple->pkgnamespace));
    }

    /* alter owner of procedure in packgae */
    AlterFunctionOwnerByPkg(pkgOid, newOwnerId);
    /* alter packgae type onwer */
    AlterTypeOwnerByPkg(pkgOid, newOwnerId);

    errno_t errorno = EOK;
    errorno = memset_s(repl_null, sizeof(repl_null), false, sizeof(repl_null));
    securec_check(errorno, "\0", "\0");
    errorno = memset_s(repl_repl, sizeof(repl_repl), false, sizeof(repl_repl));
    securec_check(errorno, "\0", "\0");

    repl_repl[Anum_gs_package_pkgowner - 1] = true;
    repl_val[Anum_gs_package_pkgowner - 1] = ObjectIdGetDatum(newOwnerId);

    /*
     * Determine the modified ACL for the new owner.  This is only
     * necessary when the ACL is non-null.
     */
    aclDatum = SysCacheGetAttr(PACKAGEOID, tup, Anum_gs_package_pkgacl, &isNull);
    if (!isNull) {
        newAcl = aclnewowner(DatumGetAclP(aclDatum), gs_package_tuple->pkgowner, newOwnerId);
        repl_repl[Anum_gs_package_pkgacl - 1] = true;
        repl_val[Anum_gs_package_pkgacl - 1] = PointerGetDatum(newAcl);
    }

    newtuple = (HeapTuple) tableam_tops_modify_tuple(tup, RelationGetDescr(rel), repl_val, repl_null, repl_repl);

    simple_heap_update(rel, &newtuple->t_self, newtuple);
    CatalogUpdateIndexes(rel, newtuple);

    tableam_tops_free_tuple(newtuple);

    /* Update owner dependency reference */
    changeDependencyOnOwner(PackageRelationId, pkgOid, newOwnerId);

    ReleaseSysCache(tup);
    /* Recode time of change the funciton owner. */
    UpdatePgObjectMtime(pkgOid, OBJECT_TYPE_PKGSPEC);
    heap_close(rel, NoLock);
    ObjectAddressSet(address, PackageRelationId, pkgOid);
    return address;
}
