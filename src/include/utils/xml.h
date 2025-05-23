/* -------------------------------------------------------------------------
 *
 * xml.h
 *	  Declarations for XML data type support.
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/xml.h
 *
 * -------------------------------------------------------------------------
 */

#ifndef XML_H
#define XML_H

#include "fmgr.h"
#include "nodes/execnodes.h"
#include "nodes/primnodes.h"
#include "utils/xmltypes.h"

#define DatumGetXmlP(X) ((xmltype*)PG_DETOAST_DATUM(X))
#define XmlPGetDatum(X) PointerGetDatum(X)

#define PG_GETARG_XML_P(n) DatumGetXmlP(PG_GETARG_DATUM(n))
#define PG_RETURN_XML_P(x) PG_RETURN_POINTER(x)

extern Datum xml_in(PG_FUNCTION_ARGS);
extern Datum xml_out(PG_FUNCTION_ARGS);
extern Datum xml_recv(PG_FUNCTION_ARGS);
extern Datum xml_send(PG_FUNCTION_ARGS);
extern Datum xmlcomment(PG_FUNCTION_ARGS);
extern Datum xmlconcat2(PG_FUNCTION_ARGS);
extern Datum texttoxml(PG_FUNCTION_ARGS);
extern Datum xmltotext(PG_FUNCTION_ARGS);
extern Datum xmlvalidate(PG_FUNCTION_ARGS);
extern Datum xpath(PG_FUNCTION_ARGS);
extern Datum xpath_exists(PG_FUNCTION_ARGS);
extern Datum xmlexists(PG_FUNCTION_ARGS);
extern Datum xml_is_well_formed(PG_FUNCTION_ARGS);
extern Datum xml_is_well_formed_document(PG_FUNCTION_ARGS);
extern Datum xml_is_well_formed_content(PG_FUNCTION_ARGS);

extern Datum table_to_xml(PG_FUNCTION_ARGS);
extern Datum query_to_xml(PG_FUNCTION_ARGS);
extern Datum cursor_to_xml(PG_FUNCTION_ARGS);
extern Datum table_to_xmlschema(PG_FUNCTION_ARGS);
extern Datum query_to_xmlschema(PG_FUNCTION_ARGS);
extern Datum cursor_to_xmlschema(PG_FUNCTION_ARGS);
extern Datum table_to_xml_and_xmlschema(PG_FUNCTION_ARGS);
extern Datum query_to_xml_and_xmlschema(PG_FUNCTION_ARGS);

extern Datum schema_to_xml(PG_FUNCTION_ARGS);
extern Datum schema_to_xmlschema(PG_FUNCTION_ARGS);
extern Datum schema_to_xml_and_xmlschema(PG_FUNCTION_ARGS);

extern Datum database_to_xml(PG_FUNCTION_ARGS);
extern Datum database_to_xmlschema(PG_FUNCTION_ARGS);
extern Datum database_to_xml_and_xmlschema(PG_FUNCTION_ARGS);

extern void pg_xml_init_library(void);
extern PgXmlErrorContext* pg_xml_init(PgXmlStrictness strictness);
extern void pg_xml_done(PgXmlErrorContext* errcxt, bool isError);
extern bool pg_xml_error_occurred(PgXmlErrorContext* errcxt);
extern void xml_ereport(PgXmlErrorContext* errcxt, int level, int sqlcode, const char* msg);

extern xmltype* xmlconcat(List* args);
extern xmltype* xmlelement(XmlExprState *xexpr, ExprContext* econtext);
extern xmltype* xmlelementByFlatten(XmlExpr *xexpr,
		   Datum *named_argvalue, bool *named_argnull,
		   Datum *argvalue, bool *argnull);
extern xmltype* xmlparse(text* data, XmlOptionType xmloption, bool preserve_whitespace);
extern xmltype* xmlpi(char* target, text* arg, bool arg_is_null, bool* result_is_null);
extern xmltype* xmlroot(xmltype* data, text* version, int standalone);
extern bool xml_is_document(xmltype* arg);
extern text* xmltotext_with_xmloption(xmltype* data, XmlOptionType xmloption_arg);
extern char* escape_xml(const char* str);

extern char* map_sql_identifier_to_xml_name(char* ident, bool fully_escaped, bool escape_period);
extern char* map_xml_name_to_sql_identifier(char* name);
extern char* map_sql_value_to_xml_value(Datum value, Oid type, bool xml_escape_strings);

/* xmltype functions */
extern Datum xmltype_extract(PG_FUNCTION_ARGS);
extern Datum xmltype_extractvalue(PG_FUNCTION_ARGS);
extern Datum xmltype_existsnode(PG_FUNCTION_ARGS);
extern Datum xmltype_getstringval(PG_FUNCTION_ARGS);
extern Datum xmltype_getstringval_array(PG_FUNCTION_ARGS);
extern Datum xmltype_xmlsequence(PG_FUNCTION_ARGS);
extern Datum xmltype_xmlsequence_array(PG_FUNCTION_ARGS);
extern Datum xmltype_appendchildxml(PG_FUNCTION_ARGS);

extern THR_LOCAL int xmloption; /* XmlOptionType, but int for guc enum */

#endif /* XML_H */
