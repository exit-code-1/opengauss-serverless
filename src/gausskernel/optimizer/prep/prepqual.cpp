/* -------------------------------------------------------------------------
 *
 * prepqual.cpp
 *	  Routines for preprocessing qualification expressions
 *
 *
 * The parser regards AND and OR as purely binary operators, so a qual like
 *		(A = 1) OR (A = 2) OR (A = 3) ...
 * will produce a nested parsetree
 *		(OR (A = 1) (OR (A = 2) (OR (A = 3) ...)))
 * In reality, the optimizer and executor regard AND and OR as N-argument
 * operators, so this tree can be flattened to
 *		(OR (A = 1) (A = 2) (A = 3) ...)
 *
 * Formerly, this module was responsible for doing the initial flattening,
 * but now we leave it to eval_const_expressions to do that since it has to
 * make a complete pass over the expression tree anyway.  Instead, we just
 * have to ensure that our manipulations preserve AND/OR flatness.
 * pull_ands() and pull_ors() are used to maintain flatness of the AND/OR
 * tree after local transformations that might introduce nested AND/ORs.
 *
 *
 * Portions Copyright (c) 2020 Huawei Technologies Co.,Ltd.
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/gausskernel/optimizer/prep/prepqual.cpp
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"
#include "knl/knl_variable.h"

#include "nodes/makefuncs.h"
#include "optimizer/clauses.h"
#include "optimizer/prep.h"
#include "utils/lsyscache.h"

static Expr* find_duplicate_ors(Expr* qual, bool is_check);
static Expr* process_duplicate_ors(List* orlist);

/*
 * negate_clause
 *	  Negate a Boolean expression.
 *
 * Input is a clause to be negated (e.g., the argument of a NOT clause).
 * Returns a new clause equivalent to the negation of the given clause.
 *
 * Although this can be invoked on its own, it's mainly intended as a helper
 * for eval_const_expressions(), and that context drives several design
 * decisions.  In particular, if the input is already AND/OR flat, we must
 * preserve that property.	We also don't bother to recurse in situations
 * where we can assume that lower-level executions of eval_const_expressions
 * would already have simplified sub-clauses of the input.
 *
 * The difference between this and a simple make_notclause() is that this
 * tries to get rid of the NOT node by logical simplification.	It's clearly
 * always a win if the NOT node can be eliminated altogether.  However, our
 * use of DeMorgan's laws could result in having more NOT nodes rather than
 * fewer.  We do that unconditionally anyway, because in WHERE clauses it's
 * important to expose as much top-level AND/OR structure as possible.
 * Also, eliminating an intermediate NOT may allow us to flatten two levels
 * of AND or OR together that we couldn't have otherwise.  Finally, one of
 * the motivations for doing this is to ensure that logically equivalent
 * expressions will be seen as physically equal(), so we should always apply
 * the same transformations.
 */
Node* negate_clause(Node* node)
{
    if (node == NULL) /* should not happen */
        ereport(ERROR,
            (errmodule(MOD_OPT),
                errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
                (errmsg("can't negate an empty subexpression"))));
    switch (nodeTag(node)) {
        case T_Const: {
            Const* c = (Const*)node;

            /* NOT NULL is still NULL */
            if (c->constisnull)
                return makeBoolConst(false, true);
            /* otherwise pretty easy */
            return makeBoolConst(!DatumGetBool(c->constvalue), false);
        } break;
        case T_OpExpr: {
            /*
             * Negate operator if possible: (NOT (< A B)) => (>= A B)
             */
            OpExpr* opexpr = (OpExpr*)node;
            Oid negator = get_negator(opexpr->opno);
            if (negator) {
                OpExpr* newopexpr = makeNode(OpExpr);
                newopexpr->opno = negator;
                newopexpr->opfuncid = InvalidOid;
                newopexpr->opresulttype = opexpr->opresulttype;
                newopexpr->opretset = opexpr->opretset;
                newopexpr->opcollid = opexpr->opcollid;
                newopexpr->inputcollid = opexpr->inputcollid;
                newopexpr->args = opexpr->args;
                newopexpr->location = opexpr->location;
                return (Node*)newopexpr;
            }
        } break;
        case T_ScalarArrayOpExpr: {
            /*
             * Negate a ScalarArrayOpExpr if its operator has a negator;
             * for example x = ANY (list) becomes x <> ALL (list)
             */
            ScalarArrayOpExpr* saopexpr = (ScalarArrayOpExpr*)node;
            Oid negator = get_negator(saopexpr->opno);
            if (negator) {
                ScalarArrayOpExpr* newopexpr = makeNode(ScalarArrayOpExpr);
                newopexpr->opno = negator;
                newopexpr->opfuncid = InvalidOid;
                newopexpr->useOr = !saopexpr->useOr;
                newopexpr->inputcollid = saopexpr->inputcollid;
                newopexpr->args = saopexpr->args;
                newopexpr->location = saopexpr->location;
                return (Node*)newopexpr;
            }
        } break;
        case T_BoolExpr: {
            BoolExpr* expr = (BoolExpr*)node;

            switch (expr->boolop) {
                    /* --------------------
                     * Apply DeMorgan's Laws:
                     *		(NOT (AND A B)) => (OR (NOT A) (NOT B))
                     *		(NOT (OR A B))	=> (AND (NOT A) (NOT B))
                     * i.e., swap AND for OR and negate each subclause.
                     *
                     * If the input is already AND/OR flat and has no NOT
                     * directly above AND or OR, this transformation preserves
                     * those properties.  For example, if no direct child of
                     * the given AND clause is an AND or a NOT-above-OR, then
                     * the recursive calls of negate_clause() can't return any
                     * OR clauses.	So we needn't call pull_ors() before
                     * building a new OR clause.  Similarly for the OR case.
                     * --------------------
                     */
                case AND_EXPR: {
                    List* nargs = NIL;
                    ListCell* lc = NULL;

                    foreach (lc, expr->args) {
                        nargs = lappend(nargs, negate_clause((Node*)lfirst(lc)));
                    }
                    return (Node*)make_orclause(nargs);
                } break;
                case OR_EXPR: {
                    List* nargs = NIL;
                    ListCell* lc = NULL;

                    foreach (lc, expr->args) {
                        nargs = lappend(nargs, negate_clause((Node*)lfirst(lc)));
                    }
                    return (Node*)make_andclause(nargs);
                } break;
                case NOT_EXPR:

                    /*
                     * NOT underneath NOT: they cancel.  We assume the
                     * input is already simplified, so no need to recurse.
                     */
                    return (Node*)linitial(expr->args);
                default:
                    ereport(ERROR,
                        (errmodule(MOD_OPT),
                            errcode(ERRCODE_OPTIMIZER_INCONSISTENT_STATE),
                            (errmsg("unrecognized boolop: %d", (int)expr->boolop))));
                    break;
            }
        } break;
        case T_NullTest: {
            NullTest* expr = (NullTest*)node;

            /*
             * In the rowtype case, the two flavors of NullTest are *not*
             * logical inverses, so we can't simplify.  But it does work
             * for scalar datatypes.
             */
            if (!expr->argisrow) {
                NullTest* newexpr = makeNode(NullTest);
                newexpr->arg = expr->arg;
                newexpr->nulltesttype = (expr->nulltesttype == IS_NULL ? IS_NOT_NULL : IS_NULL);
                newexpr->argisrow = expr->argisrow;
                return (Node*)newexpr;
            }
        } break;
        case T_NanTest: {
            NanTest* expr = (NanTest*)node;

            NanTest* newexpr = makeNode(NanTest);
            newexpr->arg = expr->arg;
            newexpr->nantesttype = (expr->nantesttype == IS_NAN ? IS_NOT_NAN : IS_NAN);
            return (Node*)newexpr;
        } break;
        case T_InfiniteTest: {
            InfiniteTest* expr = (InfiniteTest*)node;

            InfiniteTest* newexpr = makeNode(InfiniteTest);
            newexpr->arg = expr->arg;
            newexpr->infinitetesttype = (expr->infinitetesttype == IS_INFINITE ? IS_NOT_INFINITE : IS_INFINITE);
            return (Node*)newexpr;
        } break;
        case T_BooleanTest: {
            BooleanTest* expr = (BooleanTest*)node;
            BooleanTest* newexpr = makeNode(BooleanTest);

            newexpr->arg = expr->arg;
            switch (expr->booltesttype) {
                case IS_TRUE:
                    newexpr->booltesttype = IS_NOT_TRUE;
                    break;
                case IS_NOT_TRUE:
                    newexpr->booltesttype = IS_TRUE;
                    break;
                case IS_FALSE:
                    newexpr->booltesttype = IS_NOT_FALSE;
                    break;
                case IS_NOT_FALSE:
                    newexpr->booltesttype = IS_FALSE;
                    break;
                case IS_UNKNOWN:
                    newexpr->booltesttype = IS_NOT_UNKNOWN;
                    break;
                case IS_NOT_UNKNOWN:
                    newexpr->booltesttype = IS_UNKNOWN;
                    break;
                default: {
                    ereport(ERROR,
                        (errmodule(MOD_OPT),
                            errcode(ERRCODE_UNRECOGNIZED_NODE_TYPE),
                            errmsg("unrecognized bool test type: %d", (int)expr->booltesttype)));
                } break;
            }
            return (Node*)newexpr;
        } break;
        default:
            /* else fall through */
            break;
    }

    /*
     * Otherwise we don't know how to simplify this, so just tack on an
     * explicit NOT node.
     */
    return (Node*)make_notclause((Expr*)node);
}

/*
 * canonicalize_qual
 *	  Convert a qualification expression to the most useful form.
 *
 * The name of this routine is a holdover from a time when it would try to
 * force the expression into canonical AND-of-ORs or OR-of-ANDs form.
 * Eventually, we recognized that that had more theoretical purity than
 * actual usefulness, and so now the transformation doesn't involve any
 * notion of reaching a canonical form.
 *
 * NOTE: we assume the input has already been through eval_const_expressions
 * and therefore possesses AND/OR flatness.  Formerly this function included
 * its own flattening logic, but that requires a useless extra pass over the
 * tree.
 *
 * Returns the modified qualification.
 */
Expr* canonicalize_qual(Expr* qual, bool is_check)
{
    Expr* newqual = NULL;
    /* Quick exit for empty qual */
    if (qual == NULL)
        return NULL;

    /*
     * Pull up redundant subclauses in OR-of-AND trees.  We do this only
     * within the top-level AND/OR structure; there's no point in looking
     * deeper.
     */
    newqual = find_duplicate_ors(qual, is_check);

    return newqual;
}

/*
 * pull_ands
 *	  Recursively flatten nested AND clauses into a single and-clause list.
 *
 * Input is the arglist of an AND clause.
 * Returns the rebuilt arglist (note original list structure is not touched).
 */
List* pull_ands(List* andlist)
{
    List* out_list = NIL;
    ListCell* arg = NULL;

    foreach (arg, andlist) {
        Node* subexpr = (Node*)lfirst(arg);

        /*
         * Note: we can destructively concat the subexpression's arglist
         * because we know the recursive invocation of pull_ands will have
         * built a new arglist not shared with any other expr. Otherwise we'd
         * need a list_copy here.
         */
        if (and_clause(subexpr))
            out_list = list_concat(out_list, pull_ands(((BoolExpr*)subexpr)->args));
        else
            out_list = lappend(out_list, subexpr);
    }
    return out_list;
}

/*
 * pull_ors
 *	  Recursively flatten nested OR clauses into a single or-clause list.
 *
 * Input is the arglist of an OR clause.
 * Returns the rebuilt arglist (note original list structure is not touched).
 */
List* pull_ors(List* orlist)
{
    List* out_list = NIL;
    ListCell* arg = NULL;

    foreach (arg, orlist) {
        Node* subexpr = (Node*)lfirst(arg);

        /*
         * Note: we can destructively concat the subexpression's arglist
         * because we know the recursive invocation of pull_ors will have
         * built a new arglist not shared with any other expr. Otherwise we'd
         * need a list_copy here.
         */
        if (or_clause(subexpr))
            out_list = list_concat(out_list, pull_ors(((BoolExpr*)subexpr)->args));
        else
            out_list = lappend(out_list, subexpr);
    }
    return out_list;
}
/**
 * call it on in Recurse find_duplicate_ors 
 * @return NULL or true,
 * NULL means continue the loop, true means finish and return the value
 */
static Expr *ReduceConstWithinOr(Const *var, bool is_check)
{
    Assert(var != NULL && IsA(var, Const));
    if (var->constisnull) {
        if (is_check) {
            /* within OR in CHECK, Constant NULL, reduces to TRUE */
            return (Expr *) makeBoolConst(true, false);
        } else {
            /* Within OR in WHERE, drop constant NULL */
            return NULL;
        }
    }
    
    if (!DatumGetBool(var->constvalue)) {
        /* Within OR, drop constant FALSE */
        return NULL;
    }
    /* Within OR, save Constant TRUE */
    return (Expr *) var;
}

/**
 * call it on in Recurse find_duplicate_ors 
 * @return NULL or false,
 * NULL means continue the loop, false means finish and return the value
 */

static Expr *ReduceConstWithinAnd(Const *var, bool is_check)
{
    if (var->constisnull) {
        if (is_check) {
            /* Within AND in CHECK, drop constant NULL */
            return NULL;
        } else {
            /* Within AND in WHERE, constant NULL, reduces to FALSE */
            return (Expr *) makeBoolConst(false, false);
        }
    }
    
    if (DatumGetBool(var->constvalue)) {
        /* Within AND, drop constant TRUE */
        return NULL;
    }
    /* Constant FALSE, save it */
    return (Expr *) var;
}

/**
 * call it on in Recurse find_duplicate_ors 
 * @return true, false, or the @param var,
 * constant NULL:Bool reduces to true iff is_check else false, others return itself
 */
static Expr *ReduceConstWithinOther(Const *var, bool is_check)
{
    if (var == NULL || !IsA(var, Const)) {
        return (Expr *)var;
    }
    if (var->constisnull) {
        if (is_check) {
            /* in CHECK, constant NULL, reduces to TRUE */
            return (Expr *) makeBoolConst(true, false);
        } else {
            /* in WHERE, constant NULL, reduces to FALSE */
            return (Expr *) makeBoolConst(false, false);
        }
    }
    /* Constant FALSE or TRUE, save it */
    return (Expr *)var;
}

/* --------------------
 * The following code attempts to apply the inverse OR distributive law:
 *		((A AND B) OR (A AND C))  =>  (A AND (B OR C))
 * That is, locate OR clauses in which every subclause contains an
 * identical term, and pull out the duplicated terms.
 *
 * This may seem like a fairly useless activity, but it turns out to be
 * applicable to many machine-generated queries, and there are also queries
 * in some of the TPC benchmarks that need it.	This was in fact almost the
 * sole useful side-effect of the old prepqual code that tried to force
 * the query into canonical AND-of-ORs form: the canonical equivalent of
 *		((A AND B) OR (A AND C))
 * is
 *		((A OR A) AND (A OR C) AND (B OR A) AND (B OR C))
 * which the code was able to simplify to
 *		(A AND (A OR C) AND (B OR A) AND (B OR C))
 * thus successfully extracting the common condition A --- but at the cost
 * of cluttering the qual with many redundant clauses.
 * --------------------
 */
/*
 * find_duplicate_ors
 *	  Given a qualification tree with the NOTs pushed down, search for
 *	  OR clauses to which the inverse OR distributive law might apply.
 *	  Only the top-level AND/OR structure is searched.
 *
 * Returns the modified qualification.	AND/OR flatness is preserved.
 */
static Expr* find_duplicate_ors(Expr* qual, bool is_check)
{
    if (or_clause((Node*)qual)) {
        List* orlist = NIL;
        ListCell* temp = NULL;

        /* Recurse */
        foreach (temp, ((BoolExpr*)qual)->args) {
            Expr *arg = find_duplicate_ors((Expr*)lfirst(temp), is_check);
            if (arg && IsA(arg, Const)) {
                Expr *res = ReduceConstWithinOr((Const *)arg, is_check);
                /* NULL means continue */
                if (res == NULL) {
                    continue;
                } else {
                    return res;
                }
            }
            orlist = lappend(orlist, arg);
        }

        /* Flatten any ORs pulled up to just below here */
        List* temp_orlist = pull_ors(orlist);
        list_free(orlist);
        orlist = temp_orlist;
        /* OR of no inputs reduces to FALSE */
        if (orlist == NIL) {
            return (Expr *) makeBoolConst(false, false);
        }
        /* Single-expression OR just reduces to that expression */
        if (list_length(orlist) == 1) {
            return (Expr*)linitial(orlist);
        }
        
        /* Now we can look for duplicate ORs */
        return process_duplicate_ors(orlist);
    } else if (and_clause((Node*)qual)) {
        List* andlist = NIL;
        ListCell* temp = NULL;

        /* Recurse */
        foreach (temp, ((BoolExpr*)qual)->args) {
            Expr *arg = find_duplicate_ors((Expr*)lfirst(temp), is_check);
            if (arg && IsA(arg, Const)) {
                Expr *res = ReduceConstWithinAnd((Const *)arg, is_check);
                /* NULL means continue */
                if (res == NULL) {
                    continue;
                } else {
                    return res;
                }
            }
            andlist = lappend(andlist, arg);
        }
        /* Flatten any ANDs introduced just below here */
        List* temp_andlist = pull_ands(andlist);
        list_free(andlist);
        andlist = temp_andlist;
        /* AND of no inputs reduces to TRUE */
        if (andlist == NIL) {
            return (Expr *) makeBoolConst(true, false);
        }
        /* Single-expression AND just reduces to that expression */
        if (list_length(andlist) == 1) {
            return (Expr *) linitial(andlist);
        }
        /* The AND list can't get shorter, so result is always an AND */
        return make_andclause(andlist);
    } else {
        return ReduceConstWithinOther((Const *)qual, is_check);
    }
}

/*
 * process_duplicate_ors
 *	  Given a list of exprs which are ORed together, try to apply
 *	  the inverse OR distributive law.
 *
 * Returns the resulting expression (could be an AND clause, an OR
 * clause, or maybe even a single subexpression).
 */
static Expr* process_duplicate_ors(List* orlist)
{
    List* reference = NIL;
    int num_subclauses = 0;
    List* winners = NIL;
    List* neworlist = NIL;
    ListCell* temp = NULL;
    
    Assert(orlist != NULL && list_length(orlist) > 1);
    /*
     * Choose the shortest AND clause as the reference list --- obviously, any
     * subclause not in this clause isn't in all the clauses. If we find a
     * clause that's not an AND, we can treat it as a one-element AND clause,
     * which necessarily wins as shortest.
     */
    foreach (temp, orlist) {
        Expr* clause = (Expr*)lfirst(temp);

        if (and_clause((Node*)clause)) {
            List* subclauses = ((BoolExpr*)clause)->args;
            int nclauses = list_length(subclauses);
            if (reference == NIL || nclauses < num_subclauses) {
                reference = subclauses;
                num_subclauses = nclauses;
            }
        } else {
            reference = list_make1(clause);
            break;
        }
    }

    /*
     * Just in case, eliminate any duplicates in the reference list.
     */
    reference = list_union(NIL, reference);

    /*
     * Check each element of the reference list to see if it's in all the OR
     * clauses.  Build a new list of winning clauses.
     */
    winners = NIL;
    foreach (temp, reference) {
        Expr* refclause = (Expr*)lfirst(temp);
        bool win = true;
        ListCell* temp2 = NULL;

        foreach (temp2, orlist) {
            Expr* clause = (Expr*)lfirst(temp2);

            if (and_clause((Node*)clause)) {
                if (!list_member(((BoolExpr*)clause)->args, refclause)) {
                    win = false;
                    break;
                }
            } else {
                if (!equal(refclause, clause)) {
                    win = false;
                    break;
                }
            }
        }

        if (win)
            winners = lappend(winners, refclause);
    }

    /*
     * If no winners, we can't transform the OR
     */
    if (winners == NIL)
        return make_orclause(orlist);

    /*
     * Generate new OR list consisting of the remaining sub-clauses.
     *
     * If any clause degenerates to empty, then we have a situation like (A
     * AND B) OR (A), which can be reduced to just A --- that is, the
     * additional conditions in other arms of the OR are irrelevant.
     *
     * Note that because we use list_difference, any multiple occurrences of a
     * winning clause in an AND sub-clause will be removed automatically.
     */
    neworlist = NIL;
    foreach (temp, orlist) {
        Expr* clause = (Expr*)lfirst(temp);

        if (and_clause((Node*)clause)) {
            List* subclauses = ((BoolExpr*)clause)->args;

            subclauses = list_difference(subclauses, winners);
            if (subclauses != NIL) {
                if (list_length(subclauses) == 1)
                    neworlist = lappend(neworlist, linitial(subclauses));
                else
                    neworlist = lappend(neworlist, make_andclause(subclauses));
            } else {
                neworlist = NIL; /* degenerate case, see above */
                break;
            }
        } else {
            if (!list_member(winners, clause))
                neworlist = lappend(neworlist, clause);
            else {
                neworlist = NIL; /* degenerate case, see above */
                break;
            }
        }
    }

    /*
     * Append reduced OR to the winners list, if it's not degenerate, handling
     * the special case of one element correctly (can that really happen?).
     * Also be careful to maintain AND/OR flatness in case we pulled up a
     * sub-sub-OR-clause.
     */
    if (neworlist != NIL) {
        if (list_length(neworlist) == 1)
            winners = lappend(winners, linitial(neworlist));
        else
            winners = lappend(winners, make_orclause(pull_ors(neworlist)));
    }

    /*
     * And return the constructed AND clause, again being wary of a single
     * element and AND/OR flatness.
     */
    if (list_length(winners) == 1)
        return (Expr*)linitial(winners);
    else
        return make_andclause(pull_ands(winners));
}

