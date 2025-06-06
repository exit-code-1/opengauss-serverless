/* -------------------------------------------------------------------------
 *
 * scanner.h
 *		API for the core scanner (flex machine)
 *
 * The core scanner is also used by PL/pgsql, so we provide a public API
 * for it.	However, the rest of the backend is only expected to use the
 * higher-level API provided by parser.h.
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/parser/scanner.h
 *
 * -------------------------------------------------------------------------
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "nodes/parsenodes_common.h"
#include "parser/keywords.h"

#ifdef FRONTEND_PARSER
#include <setjmp.h>
#undef ereport
#define ereport(a, b) fe_rethrow(yyscanner)
#endif

/*
 * The scanner returns extra data about scanned tokens in this union type.
 * Note that this is a subset of the fields used in YYSTYPE of the bison
 * parsers built atop the scanner.
 */
typedef union core_YYSTYPE {
    int ival;            /* for integer literals */
    char* str;           /* for identifiers and non-integer literals */
    const char* keyword; /* canonical spelling of keywords */
} core_YYSTYPE;

/*
 * We track token locations in terms of byte offsets from the start of the
 * source string, not the column number/line number representation that
 * bison uses by default.  Also, to minimize overhead we track only one
 * location (usually the first token location) for each construct, not
 * the beginning and ending locations as bison does by default.  It's
 * therefore sufficient to make YYLTYPE an int.
 */
#define YYLTYPE int

#define DELIMITER_LENGTH 16

/*
 * Another important component of the scanner's API is the token code numbers.
 * However, those are not defined in this file, because bison insists on
 * defining them for itself.  The token codes used by the core scanner are
 * the ASCII characters plus these:
 *	%token <str>	IDENT FCONST SCONST BCONST XCONST Op
 *	%token <ival>	ICONST PARAM
 *	%token			TYPECAST DOT_DOT COLON_EQUALS PARA_EQUALS
 * The above token definitions *must* be the first ones declared in any
 * bison parser built atop this scanner, so that they will have consistent
 * numbers assigned to them (specifically, IDENT = 258 and so on).
 */

/*
 * The YY_EXTRA data that a flex scanner allows us to pass around.
 * Private state needed by the core scanner goes here.	Note that the actual
 * yy_extra struct may be larger and have this as its first component, thus
 * allowing the calling parser to keep some fields of its own in YY_EXTRA.
 */
typedef struct core_yy_extra_type {
    /*
     * The string the scanner is physically scanning.  We keep this mainly so
     * that we can cheaply compute the offset of the current token (yytext).
     */
    char* scanbuf;
    Size scanbuflen;

    /* 
     * The keyword list to use.
     */
	const ScanKeywordList *keywordlist;
	const uint16 *keyword_tokens;

    /*
     * literalbuf is used to accumulate literal values when multiple rules are
     * needed to parse a single literal.  Call startlit() to reset buffer to
     * empty, addlit() to add text.  NOTE: the string in literalbuf is NOT
     * necessarily null-terminated, but there always IS room to add a trailing
     * null at offset literallen.  We store a null only when we need it.
     */
    char* literalbuf; /* palloc'd expandable buffer */
    int literallen;   /* actual current string length */
    int literalalloc; /* current allocated buffer size */

    int xcdepth;     /* depth of nesting in slash-star comments */
    char* dolqstart; /* current $foo$ quote start string */

    /* first part of UTF16 surrogate pair for Unicode escapes */
    int32 utf16_first_part;

    /* state variables for literal-lexing warnings */
    bool warn_on_first_escape;
    bool saw_non_ascii;
    bool ident_quoted;
    bool warnOnTruncateIdent;

    /* record the message need by multi-query. */
    List* query_string_locationlist; /* record the end location of each single query */
    bool in_slash_proc_body;         /* check whether it's in a slash proc body */
    int paren_depth;                 /* record the current depth in  the '(' and ')' */
    bool is_createstmt;              /* check whether it's a create statement. */
    bool is_hint_str;                /* current identifier is in hint comment string */
    List* parameter_list;            /* placeholder parameter list */
    bool include_ora_comment;        /* dont igore comment when ture */
    int func_param_begin;            /* function and procedure param string start pos,exclude left parenthesis */
    int func_param_end;              /* function and procedure param string end pos,exclude right parenthesis */
    int return_pos_end;
    bool isPlpgsqlKeyWord;
    const PlpgsqlKeywordValue* plKeywordValue;
    bool is_delimiter_name;
    bool is_last_colon;
    bool is_proc_end;
} core_yy_extra_type;

#ifdef FRONTEND_PARSER

class PGClientLogic;

void fe_rethrow(void *yyscanner);
struct fe_core_yy_extra_type : public core_yy_extra_type {
public:
    jmp_buf jump_buffer;
    PGClientLogic *m_clientLogic;
};
#endif /* FRONTEND_PARSER */

/*
 * The type of yyscanner is opaque outside scan.l.
 */
typedef void* core_yyscan_t;
typedef void* yyscan_t;

/* Constant data exported from parser/scan.l */
extern PGDLLIMPORT const uint16 ScanKeywordTokens[];

/* Entry points in parser/scan.l */
extern core_yyscan_t scanner_init(const char* str, 
                                  core_yy_extra_type* yyext, 
                                  const ScanKeywordList *keywordlist,
	                              const uint16 *keyword_tokens);

#ifdef FRONTEND_PARSER
extern core_yyscan_t fe_scanner_init(const char *str, fe_core_yy_extra_type *yyext, 
    const ScanKeywordList *keywordlist, const uint16 *keyword_tokens);
#endif /* FRONTEND_PARSER */

extern void scanner_finish(core_yyscan_t yyscanner);
extern int core_yylex(core_YYSTYPE* lvalp, YYLTYPE* llocp, core_yyscan_t yyscanner);
extern int scanner_errposition(int location, core_yyscan_t yyscanner);
extern void scanner_yyerror(const char* message, core_yyscan_t yyscanner);
extern void addErrorList(const char* message, int lines);

typedef int (*coreYYlexFunc)(core_YYSTYPE* lvalp, YYLTYPE* llocp, core_yyscan_t yyscanner);

#endif /* SCANNER_H */

