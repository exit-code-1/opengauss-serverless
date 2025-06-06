/* -------------------------------------------------------------------------
 *
 * format_type.c
 *	  Display type names "nicely".
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/backend/utils/adt/format_type.c
 *
 * -------------------------------------------------------------------------
 */

#include "postgres.h"
#include "knl/knl_variable.h"

#include "catalog/namespace.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/numeric.h"
#include "utils/numeric_gs.h"
#include "utils/syscache.h"
#include "mb/pg_wchar.h"
#include "miscadmin.h"

#define MAX_INT32_LEN 11

static char* format_type_internal(
    Oid type_oid, int32 typemod, bool typemod_given, bool allow_invalid, bool include_nspname = false);
char* printTypmod(const char* typname, int32 typmod, Oid typmodout);
static char* psnprintf(size_t len, const char* fmt, ...)
    /* This lets gcc check the format string for consistency. */
    __attribute__((format(PG_PRINTF_ATTRIBUTE, 2, 3)));

/*
 * SQL function: format_type(type_oid, typemod)
 *
 * `type_oid' is from pg_type.oid, `typemod' is from
 * pg_attribute.atttypmod. This function will get the type name and
 * format it and the modifier to canonical SQL format, if the type is
 * a standard type. Otherwise you just get pg_type.typname back,
 * double quoted if it contains funny characters or matches a keyword.
 *
 * If typemod is NULL then we are formatting a type name in a context where
 * no typemod is available, eg a function argument or result type.	This
 * yields a slightly different result from specifying typemod = -1 in some
 * cases.  Given typemod = -1 we feel compelled to produce an output that
 * the parser will interpret as having typemod -1, so that pg_dump will
 * produce CREATE TABLE commands that recreate the original state.	But
 * given NULL typemod, we assume that the parser's interpretation of
 * typemod doesn't matter, and so we are willing to output a slightly
 * "prettier" representation of the same type.	For example, type = bpchar
 * and typemod = NULL gets you "character", whereas typemod = -1 gets you
 * "bpchar" --- the former will be interpreted as character(1) by the
 * parser, which does not yield typemod -1.
 *
 * XXX encoding a meaning in typemod = NULL is ugly; it'd have been
 * cleaner to make two functions of one and two arguments respectively.
 * Not worth changing it now, however.
 */
Datum format_type(PG_FUNCTION_ARGS)
{
    Oid type_oid;
    int32 typemod;
    char* result = NULL;

    /* Since this function is not strict, we must test for null args */
    if (PG_ARGISNULL(0))
        PG_RETURN_NULL();

    type_oid = PG_GETARG_OID(0);

    if (PG_ARGISNULL(1))
        result = format_type_internal(type_oid, -1, false, true);
    else {
        typemod = PG_GETARG_INT32(1);
        result = format_type_internal(type_oid, typemod, true, true);
    }

    PG_RETURN_TEXT_P(cstring_to_text(result));
}

/*
 * This version is for use within the backend in error messages, etc.
 * One difference is that it will fail for an invalid type.
 *
 * The result is always a palloc'd string.
 */
char* format_type_be(Oid type_oid)
{
    return format_type_internal(type_oid, -1, false, false);
}

char *
format_type_be_qualified(Oid type_oid)
{
   return format_type_internal(type_oid, -1, false, false, true);
}

/*
 * This version is for use within send PARSE message with specified
 * statement/plan down to the Datanode
 *
 * The result is always a palloc'd string.
 */
char* format_type_with_nspname(Oid type_oid)
{
    return format_type_internal(type_oid, -1, false, false, true);
}

/*
 * This version allows a nondefault typemod to be specified.
 */
char* format_type_with_typemod(Oid type_oid, int32 typemod)
{
    return format_type_internal(type_oid, typemod, true, false);
}

static char* format_type_internal(
    Oid type_oid, int32 typemod, bool typemod_given, bool allow_invalid, bool include_nspname)
{
    bool with_typemod = typemod_given && (typemod >= 0);
    HeapTuple tuple;
    Form_pg_type typeform;
    Oid array_base_type;
    bool is_array = false;
    char* buf = NULL;

    if (type_oid == InvalidOid && allow_invalid)
        return pstrdup("-");

    tuple = SearchSysCache1(TYPEOID, ObjectIdGetDatum(type_oid));
    if (!HeapTupleIsValid(tuple)) {
        if (allow_invalid)
            return pstrdup("???");
        else
            ereport(ERROR, (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for type %u", type_oid)));
    }
    typeform = (Form_pg_type)GETSTRUCT(tuple);

    /*
     * Check if it's a regular (variable length) array type.  Fixed-length
     * array types such as "name" shouldn't get deconstructed.  As of openGauss
     * 8.1, rather than checking typlen we check the toast property, and don't
     * deconstruct "plain storage" array types --- this is because we don't
     * want to show oidvector as oid[].
     */
    array_base_type = typeform->typelem;

    if (array_base_type != InvalidOid && typeform->typstorage != 'p' && type_oid != OIDVECTOREXTENDOID &&
        type_oid != INT2VECTOREXTENDOID && typeform->typtype != TYPTYPE_TABLEOF) {
        /* Switch our attention to the array element type */
        ReleaseSysCache(tuple);
        tuple = SearchSysCache1(TYPEOID, ObjectIdGetDatum(array_base_type));
        if (!HeapTupleIsValid(tuple)) {
            if (allow_invalid)
                return pstrdup("???[]");
            else
                ereport(
                    ERROR, (errcode(ERRCODE_CACHE_LOOKUP_FAILED), errmsg("cache lookup failed for type %u", type_oid)));
        }
        typeform = (Form_pg_type)GETSTRUCT(tuple);
        type_oid = array_base_type;
        is_array = true;
    } else
        is_array = false;

    /*
     * See if we want to special-case the output for certain built-in types.
     * Note that these special cases should all correspond to special
     * productions in gram.y, to ensure that the type name will be taken as a
     * system type, not a user type of the same name.
     *
     * If we do not provide a special-case output here, the type name will be
     * handled the same way as a user type name --- in particular, it will be
     * double-quoted if it matches any lexer keyword.  This behavior is
     * essential for some cases, such as types "bit" and "char".
     */
    buf = NULL; /* flag for no special case */
// if the type is encrypted column we should replace the type oid to the original oid
// the original oid should be saved in typemod so we sanity check that typemod can be type id
    if ((type_oid == BYTEAWITHOUTORDERWITHEQUALCOLOID || type_oid == BYTEAWITHOUTORDERCOLOID) && typemod > 0) {
        type_oid = typemod;
        with_typemod = false;
    }

    switch (type_oid) {
        case BITOID:
            if (with_typemod)
                buf = printTypmod("bit", typemod, typeform->typmodout);
            else if (typemod_given) {
                /*
                 * bit with typmod -1 is not the same as BIT, which means
                 * BIT(1) per SQL spec.  Report it as the quoted typename so
                 * that parser will not assign a bogus typmod.
                 */
            } else
                buf = pstrdup("bit");
            break;

        case BOOLOID:
            buf = pstrdup("boolean");
            break;

        case BPCHAROID:
            if (with_typemod)
                buf = printTypmod("character", typemod, typeform->typmodout);
            else if (typemod_given) {
                /*
                 * bpchar with typmod -1 is not the same as CHARACTER, which
                 * means CHARACTER(1) per SQL spec.  Report it as bpchar so
                 * that parser will not assign a bogus typmod.
                 */
            } else
                buf = pstrdup("character");
            break;

        case DATEOID:
            buf = pstrdup("date");
            break;

        case FLOAT4OID:
            buf = pstrdup("real");
            break;

        case FLOAT8OID:
            buf = pstrdup("double precision");
            break;
        case INT1OID:
            buf = pstrdup("tinyint");
            break;
        case INT2OID:
            buf = pstrdup("smallint");
            break;

        case INT4OID:
            buf = pstrdup("integer");
            break;

        case INT8OID:
            buf = pstrdup("bigint");
            break;

        case NUMERICOID:
            if (with_typemod)
                buf = printTypmod("numeric", typemod, typeform->typmodout);
            else
                buf = pstrdup("numeric");
            break;

        case INTERVALOID:
            if (with_typemod)
                buf = printTypmod("interval", typemod, typeform->typmodout);
            else
                buf = pstrdup("interval");
            break;

        case TIMEOID:
            if (with_typemod)
                buf = printTypmod("time", typemod, typeform->typmodout);
            else
                buf = pstrdup("time without time zone");
            break;

        case TIMETZOID:
            if (with_typemod)
                buf = printTypmod("time", typemod, typeform->typmodout);
            else
                buf = pstrdup("time with time zone");
            break;

        case TIMESTAMPOID:
            if (with_typemod)
                buf = printTypmod("timestamp", typemod, typeform->typmodout);
            else
                buf = pstrdup("timestamp without time zone");
            break;

        case TIMESTAMPTZOID:
            if (with_typemod)
                buf = printTypmod("timestamp", typemod, typeform->typmodout);
            else
                buf = pstrdup("timestamp with time zone");
            break;
        case SMALLDATETIMEOID:
            buf = pstrdup("smalldatetime");
            break;
        case VARBITOID:
            if (with_typemod)
                buf = printTypmod("bit varying", typemod, typeform->typmodout);
            else
                buf = pstrdup("bit varying");
            break;

        case VARCHAROID:
            if (with_typemod)
                buf = printTypmod("character varying", typemod, typeform->typmodout);
            else
                buf = pstrdup("character varying");
            break;

        case NVARCHAR2OID:
            if (with_typemod)
                buf = printTypmod("nvarchar2", typemod, typeform->typmodout);
            else
                buf = pstrdup("nvarchar2");
            break;
        case TEXTOID:
            buf = pstrdup("text");
            break;
        case BYTEAOID:
            buf = pstrdup("bytea");
            break;
        case BYTEAWITHOUTORDERWITHEQUALCOLOID:
            buf = pstrdup("byteawithoutorderwithequalcol");
            break;
        default:
            break;
    }

    if (buf == NULL) {
        /*
         * Default handling: report the name as it appears in the catalog.
         * Here, we must qualify the name if it is not visible in the search
         * path, and we must double-quote it if it's not a standard identifier
         * or if it matches any keyword.
         */
        char* nspname = NULL;
        char* typname = NULL;

        if (TypeIsVisible(type_oid)) {
            nspname = NULL;

            /* get namespace string if we foce to deparse namespace name */
            if (include_nspname && PG_CATALOG_NAMESPACE != typeform->typnamespace)
                nspname = get_namespace_name_or_temp(typeform->typnamespace);
        } else {
            nspname = get_namespace_name_or_temp(typeform->typnamespace);
        }

        typname = NameStr(typeform->typname);

        buf = quote_qualified_identifier(nspname, typname);

        if (with_typemod && !(type_oid == BYTEAWITHOUTORDERWITHEQUALCOLOID || type_oid == BYTEAWITHOUTORDERCOLOID))
            buf = printTypmod(buf, typemod, typeform->typmodout);
    }

    if (is_array)
        buf = psnprintf(strlen(buf) + 3, "%s[]", buf);

    ReleaseSysCache(tuple);

    return buf;
}

/*
 * Add typmod decoration to the basic type name
 */
char* printTypmod(const char* typname, int32 typmod, Oid typmodout)
{
    char* res = NULL;

    /* Shouldn't be called if typmod is -1 */
    Assert(typmod >= 0);

    if (typmodout == InvalidOid) {
        /* Default behavior: just print the integer typmod with parens */
        res = psnprintf(strlen(typname) + MAX_INT32_LEN + 3, "%s(%d)", typname, (int)typmod);
    } else {
        /* Use the type-specific typmodout procedure */
        char* tmstr = NULL;
        if (strcmp(typname, "numeric") == 0 && ((int16)(((uint32)(typmod - VARHDRSZ)) & 0XFFFF)) == PG_INT16_MIN) {
            typname = "float";
        }

        tmstr = DatumGetCString(OidFunctionCall1(typmodout, Int32GetDatum(typmod)));
        res = psnprintf(strlen(typname) + strlen(tmstr) + 1, "%s%s", typname, tmstr);
    }

    return res;
}

/*
 * type_maximum_size --- determine maximum width of a variable-width column
 *
 * If the max width is indeterminate, return -1.  In particular, we return
 * -1 for any type not known to this routine.  We assume the caller has
 * already determined that the type is a variable-width type, so it's not
 * necessary to look up the type's pg_type tuple here.
 *
 * This may appear unrelated to format_type(), but in fact the two routines
 * share knowledge of the encoding of typmod for different types, so it's
 * convenient to keep them together.  (XXX now that most of this knowledge
 * has been pushed out of format_type into the typmodout functions, it's
 * interesting to wonder if it's worth trying to factor this code too...)
 */
int32 type_maximum_size(Oid type_oid, int32 typemod)
{
    if (typemod < 0)
        return -1;

    switch (type_oid) {
        case BPCHAROID:
        case VARCHAROID:
            /* typemod includes varlena header */

            /* typemod is in characters not bytes */
            return (typemod - VARHDRSZ) * pg_encoding_max_length(GetDatabaseEncoding()) + VARHDRSZ;

        case NUMERICOID:
            return numeric_maximum_size(typemod);

        case VARBITOID:
        case BITOID:
            /* typemod is the (max) number of bits */
            return (typemod + (BITS_PER_BYTE - 1)) / BITS_PER_BYTE + 2 * sizeof(int32);
        default:
            break;
    }

    /* Unknown type, or unlimited-width type such as 'text' */
    return -1;
}

/*
 * oidvectortypes			- converts a vector of type OIDs to "typname" list
 */
Datum oidvectortypes(PG_FUNCTION_ARGS)
{
    oidvector* oidArray = (oidvector*)PG_GETARG_POINTER(0);
    char* result = NULL;
    int numargs = oidArray->dim1;
    int num;
    size_t total;
    size_t left;

    total = 20 * numargs + 1;
    result = (char*)palloc(total);
    result[0] = '\0';
    left = total - 1;
    errno_t rc = EOK;

    for (num = 0; num < numargs; num++) {
        char* typname = format_type_internal(oidArray->values[num], -1, false, true);
        size_t slen = strlen(typname);

        if (left < (slen + 2)) {
            total += slen + 2;
            result = (char*)repalloc(result, total);
            left += slen + 2;
        }

        if (num > 0) {
            rc = strcat_s(result, total, ", ");
            securec_check(rc, "\0", "\0");
            left -= 2;
        }
        rc = strcat_s(result, total, typname);
        securec_check(rc, "\0", "\0");
        left -= slen;
    }

    if ((!strcmp(result, "")) && u_sess->attr.attr_sql.sql_compatibility == A_FORMAT &&
        !ACCEPT_EMPTY_STR && !RETURN_NS) {
        pfree_ext(result);
        PG_RETURN_NULL();
    }

    PG_RETURN_TEXT_P(cstring_to_text(result));
}

/* snprintf into a palloc'd string */
static char* psnprintf(size_t len, const char* fmt, ...)
{
    va_list ap;
    char* buf = NULL;

    buf = (char*)palloc0(len);

    va_start(ap, fmt);
    errno_t rc = vsnprintf_s(buf, len, len - 1, fmt, ap);
    securec_check_ss(rc, "\0", "\0");
    va_end(ap);

    return buf;
}
