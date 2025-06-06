/* -------------------------------------------------------------------------
 *
 * pg_dump_sort.c
 *	  Sort the items of a dump into a safe order for dumping
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/bin/pg_dump/pg_dump_sort.c
 *
 * -------------------------------------------------------------------------
 */
#include "pg_backup_archiver.h"
#include "dumpmem.h"

#ifdef GAUSS_SFT_TEST
#include "gauss_sft.h"
#endif

extern bool include_extensions;
/* translator: this is a module name */
static const char* modulename = gettext_noop("sorter");

/*
 * Sort priority for object types when dumping a pre-7.3 database.
 * Objects are sorted by priority levels, and within an equal priority level
 * by OID.	(This is a relatively crude hack to provide semi-reasonable
 * behavior for old databases without full dependency info.)  Note: collations,
 * extensions, text search, foreign-data, and default ACL objects can't really
 * happen here, so the rather bogus priorities for them don't matter.
 *
 * NOTE: object-type priorities must match the section assignments made in
 * pg_dump.c; that is, PRE_DATA objects must sort before DO_PRE_DATA_BOUNDARY,
 * POST_DATA objects must sort after DO_POST_DATA_BOUNDARY, and DATA objects
 * must sort between them.
 */
static const int oldObjectTypePriority[] = {
    1,  /* DO_NAMESPACE */
    1,  /* DO_EXTENSION */
    2,  /* DO_TYPE */
    2,  /* DO_SHELL_TYPE */
    2,  /* DO_FUNC */
    3,  /* DO_AGG */
    3,  /* DO_OPERATOR */
    3,  /* DO_ACCESS_METHOD */
    4,  /* DO_OPCLASS */
    4,  /* DO_OPFAMILY */
    4,  /* DO_COLLATION */
    5,  /* DO_CONVERSION */
    6,  /* DO_TABLE */
    8,  /* DO_ATTRDEF */
    15, /* DO_INDEX */
    16, /* DO_RULE */
    17, /* DO_TRIGGER */
    14, /* DO_CONSTRAINT */
    18, /* DO_FK_CONSTRAINT */
    2,  /* DO_PROCLANG */
    2,  /* DO_CAST */
    11, /* DO_TABLE_DATA */
    7,  /* DO_DUMMY_TYPE */
    4,  /* DO_TSPARSER */
    4,  /* DO_TSDICT */
    4,  /* DO_TSTEMPLATE */
    4,  /* DO_TSCONFIG */
    4,  /* DO_FDW */
    4,  /* DO_FOREIGN_SERVER */
    19, /* DO_DEFAULT_ACL */
    9,  /* DO_BLOB */
    12, /* DO_BLOB_DATA */
    10, /* DO_PRE_DATA_BOUNDARY */
    13  /* DO_POST_DATA_BOUNDARY */
};

/*
 * Sort priority for object types when dumping newer databases.
 * Objects are sorted by type, and within a type by name.
 *
 * NOTE: object-type priorities must match the section assignments made in
 * pg_dump.c; that is, PRE_DATA objects must sort before DO_PRE_DATA_BOUNDARY,
 * POST_DATA objects must sort after DO_POST_DATA_BOUNDARY, and DATA objects
 * must sort between them.
 */
static const int newObjectTypePriority[] = {
    1,  /* DO_NAMESPACE */
    4,  /* DO_EXTENSION */
    5,  /* DO_TYPE */
    5,  /* DO_SHELL_TYPE */
    6,  /* DO_FUNC */
    7,  /* DO_AGG */
    8,  /* DO_OPERATOR */
    8,  /* DO_ACCESS_METHOD */
    9,  /* DO_OPCLASS */
    9,  /* DO_OPFAMILY */
    3,  /* DO_COLLATION */
    11, /* DO_CONVERSION */
    18, /* DO_TABLE */
    20, /* DO_ATTRDEF */
    27, /* DO_INDEX */
    28, /* DO_RULE */
    29, /* DO_TRIGGER */
    26, /* DO_CONSTRAINT */
    30, /* DO_FK_CONSTRAINT */
    2,  /* DO_PROCLANG */
    10, /* DO_CAST */
    23, /* DO_PACKAGE */
    23, /* DO_TABLE_DATA */
    19, /* DO_DUMMY_TYPE */
    12, /* DO_TSPARSER */
    14, /* DO_TSDICT */
    13, /* DO_TSTEMPLATE */
    15, /* DO_TSCONFIG */
    16, /* DO_FDW */
    17, /* DO_FOREIGN_SERVER */
    32, /* DO_DEFAULT_ACL */
    21, /* DO_BLOB */
    24, /* DO_BLOB_DATA */
    22, /* DO_PRE_DATA_BOUNDARY */
    25, /* DO_POST_DATA_BOUNDARY */
    31, /* DO_FTBL_CONSTRAINT */
    33, /* DO_RLSPOLICY */
    34, /* DO_PUBLICATION */
    35, /* DO_PUBLICATION_REL */
    36, /* DO_SUBSCRIPTION */
    18,  /* DO_EVENT */
    37  /* DO_EVENT_TRIGGER */
};

static DumpId postDataBoundId;

static int DOTypeNameCompare(const void* p1, const void* p2);
static int DOTypeOidCompare(const void* p1, const void* p2);
static bool TopoSort(DumpableObject** objs, int numObjs, DumpableObject** ordering, int* nOrdering);
static void addHeapElement(int val, int* heap, int heapLength);
static int removeHeapElement(int* heap, int heapLength);
static void findDependencyLoops(DumpableObject** objs, int nObjs, int totObjs);
static int findLoop(DumpableObject* obj, DumpId startPoint, bool* processed, DumpableObject** workspace, int depth);
static void repairDependencyLoop(DumpableObject** loop, int nLoop);
static void describeDumpableObject(DumpableObject* obj, char* buf, int bufsize);

/*
 * Sort the given objects into a type/name-based ordering
 *
 * Normally this is just the starting point for the dependency-based
 * ordering.
 */
void sortDumpableObjectsByTypeName(DumpableObject** objs, int numObjs)
{
    if (numObjs > 1)
        qsort((void*)objs, numObjs, sizeof(DumpableObject*), DOTypeNameCompare);
}

static int DOTypeNameCompare(const void* p1, const void* p2)
{
    DumpableObject* obj1 = *(DumpableObject* const*)p1;
    DumpableObject* obj2 = *(DumpableObject* const*)p2;
    int cmpval;

    /* Sort by type */
    cmpval = newObjectTypePriority[obj1->objType] - newObjectTypePriority[obj2->objType];

    if (cmpval != 0)
        return cmpval;

    /*
     * Sort by namespace.  Typically, all objects of the same priority would
     * either have or not have a namespace link, but there are exceptions.
     * Sort NULL namespace after non-NULL in such cases.
     */
    if (obj1->nmspace != NULL) {
        if (obj2->nmspace != NULL) {
            cmpval = strcmp(obj1->nmspace->dobj.name, obj2->nmspace->dobj.name);
            if (cmpval != 0)
                return cmpval;
        } else
            return -1;
    } else if (obj2->nmspace != NULL)
        return 1;

    /* Sort by name */
    cmpval = strcmp(obj1->name, obj2->name);
    if (cmpval != 0)
        return cmpval;

    /* To have a stable sort order, break ties for some object types */
    if (obj1->objType == DO_FUNC || obj1->objType == DO_AGG) {
        FuncInfo* fobj1 = *(FuncInfo* const*)p1;
        FuncInfo* fobj2 = *(FuncInfo* const*)p2;

        cmpval = fobj1->nargs - fobj2->nargs;
        if (cmpval != 0)
            return cmpval;
    } else if (obj1->objType == DO_OPERATOR) {
        OprInfo* oobj1 = *(OprInfo* const*)p1;
        OprInfo* oobj2 = *(OprInfo* const*)p2;

        /* oprkind is 'l', 'r', or 'b'; this sorts prefix, postfix, infix */
        cmpval = (oobj2->oprkind - oobj1->oprkind);
        if (cmpval != 0)
            return cmpval;
    } else if (obj1->objType == DO_ATTRDEF) {
        AttrDefInfo* adobj1 = *(AttrDefInfo* const*)p1;
        AttrDefInfo* adobj2 = *(AttrDefInfo* const*)p2;

        cmpval = (adobj1->adnum - adobj2->adnum);
        if (cmpval != 0)
            return cmpval;
    }

    /* Usually shouldn't get here, but if we do, sort by OID */
    return oidcmp(obj1->catId.oid, obj2->catId.oid);
}

/*
 * Sort the given objects into a type/OID-based ordering
 *
 * This is used with pre-7.3 source databases as a crude substitute for the
 * lack of dependency information.
 */
void sortDumpableObjectsByTypeOid(DumpableObject** objs, int numObjs)
{
    if (numObjs > 1)
        qsort((void*)objs, numObjs, sizeof(DumpableObject*), DOTypeOidCompare);
}

static int DOTypeOidCompare(const void* p1, const void* p2)
{
    DumpableObject* obj1 = *(DumpableObject* const*)p1;
    DumpableObject* obj2 = *(DumpableObject* const*)p2;
    int cmpval;

    cmpval = oldObjectTypePriority[obj1->objType] - oldObjectTypePriority[obj2->objType];

    if (cmpval != 0)
        return cmpval;

    return oidcmp(obj1->catId.oid, obj2->catId.oid);
}

/*
 * Sort the given objects into a safe dump order using dependency
 * information (to the extent we have it available).
 *
 * The DumpIds of the PRE_DATA_BOUNDARY and POST_DATA_BOUNDARY objects are
 * passed in separately, in case we need them during dependency loop repair.
 */
void sortDumpableObjects(DumpableObject** objs, int numObjs, DumpId preBoundaryId, DumpId postBoundaryId)
{
    DumpableObject** ordering;
    int nOrdering;
    errno_t rc = 0;

    if (numObjs <= 0) /* can't happen anymore ... */
        return;

    /*
     * Saving the boundary IDs in static variables is a bit grotty, but seems
     * better than adding them to parameter lists of subsidiary functions.
     */
    postDataBoundId = postBoundaryId;

    ordering = (DumpableObject**)pg_malloc(numObjs * sizeof(DumpableObject*));
    while (!TopoSort(objs, numObjs, ordering, &nOrdering))
        findDependencyLoops(ordering, nOrdering, numObjs);

    rc = memcpy_s(objs, (numObjs * sizeof(DumpableObject*)), ordering, numObjs * sizeof(DumpableObject*));
    securec_check_c(rc, "\0", "\0");

    free(ordering);
    ordering = NULL;
}

/*
 * TopoSort -- topological sort of a dump list
 *
 * Generate a re-ordering of the dump list that satisfies all the dependency
 * constraints shown in the dump list.	(Each such constraint is a fact of a
 * partial ordering.)  Minimize rearrangement of the list not needed to
 * achieve the partial ordering.
 *
 * The input is the list of numObjs objects in objs[].	This list is not
 * modified.
 *
 * Returns TRUE if able to build an ordering that satisfies all the
 * constraints, FALSE if not (there are contradictory constraints).
 *
 * On success (TRUE result), ordering[] is filled with a sorted array of
 * DumpableObject pointers, of length equal to the input list length.
 *
 * On failure (FALSE result), ordering[] is filled with an unsorted array of
 * DumpableObject pointers of length *nOrdering, listing the objects that
 * prevented the sort from being completed.  In general, these objects either
 * participate directly in a dependency cycle, or are depended on by objects
 * that are in a cycle.  (The latter objects are not actually problematic,
 * but it takes further analysis to identify which are which.)
 *
 * The caller is responsible for allocating sufficient space at *ordering.
 */
static bool TopoSort(DumpableObject** objs, int numObjs, DumpableObject** ordering, /* output argument */
    int* nOrdering)                                                                 /* output argument */
{
    DumpId maxDumpId = getMaxDumpId();
    int* pendingHeap = NULL;
    int* beforeConstraints = NULL;
    int* idMap = NULL;
    DumpableObject* obj = NULL;
    int heapLength;
    int i = 0;
    int j = 0;
    int k = 0;
    errno_t rc = 0;

    /*
     * This is basically the same algorithm shown for topological sorting in
     * Knuth's Volume 1.  However, we would like to minimize unnecessary
     * rearrangement of the input ordering; that is, when we have a choice of
     * which item to output next, we always want to take the one highest in
     * the original list.  Therefore, instead of maintaining an unordered
     * linked list of items-ready-to-output as Knuth does, we maintain a heap
     * of their item numbers, which we can use as a priority queue.  This
     * turns the algorithm from O(N) to O(N log N) because each insertion or
     * removal of a heap item takes O(log N) time.	However, that's still
     * plenty fast enough for this application.
     */

    *nOrdering = numObjs; /* for success return */

    /* Eliminate the null case */
    if (numObjs <= 0)
        return true;

    /* Create workspace for the above-described heap */
    pendingHeap = (int*)pg_malloc(numObjs * sizeof(int));

    /*
     * Scan the constraints, and for each item in the input, generate a count
     * of the number of constraints that say it must be before something else.
     * The count for the item with dumpId j is stored in beforeConstraints[j].
     * We also make a map showing the input-order index of the item with
     * dumpId j.
     */
    beforeConstraints = (int*)pg_malloc((maxDumpId + 1) * sizeof(int));
    rc = memset_s(beforeConstraints, ((maxDumpId + 1) * sizeof(int)), 0, (maxDumpId + 1) * sizeof(int));
    securec_check_c(rc, "\0", "\0");
    idMap = (int*)pg_malloc((maxDumpId + 1) * sizeof(int));
    for (i = 0; i < numObjs; i++) {
        obj = objs[i];
        j = obj->dumpId;
        if (j <= 0 || j > maxDumpId)
            exit_horribly(modulename, "invalid dumpId %d\n", j);
        idMap[j] = i;
        for (j = 0; j < obj->nDeps; j++) {
            k = obj->dependencies[j];
            if (k <= 0 || k > maxDumpId)
                exit_horribly(modulename, "invalid dependency %d\n", k);
            beforeConstraints[k]++;
        }
    }

    /*
     * Now initialize the heap of items-ready-to-output by filling it with the
     * indexes of items that already have beforeConstraints[id] == 0.
     *
     * The essential property of a heap is heap[(j-1)/2] >= heap[j] for each j
     * in the range 1..heapLength-1 (note we are using 0-based subscripts
     * here, while the discussion in Knuth assumes 1-based subscripts). So, if
     * we simply enter the indexes into pendingHeap[] in decreasing order, we
     * a-fortiori have the heap invariant satisfied at completion of this
     * loop, and don't need to do any sift-up comparisons.
     */
    heapLength = 0;
    for (i = numObjs; --i >= 0;) {
        if (beforeConstraints[objs[i]->dumpId] == 0)
            pendingHeap[heapLength++] = i;
    }

    /* --------------------
     * Now emit objects, working backwards in the output list.	At each step,
     * we use the priority heap to select the last item that has no remaining
     * before-constraints.	We remove that item from the heap, output it to
     * ordering[], and decrease the beforeConstraints count of each of the
     * items it was constrained against.  Whenever an item's beforeConstraints
     * count is thereby decreased to zero, we insert it into the priority heap
     * to show that it is a candidate to output.  We are done when the heap
     * becomes empty; if we have output every element then we succeeded,
     * otherwise we failed.
     * i = number of ordering[] entries left to output
     * j = objs[] index of item we are outputting
     * k = temp for scanning constraint list for item j
     * --------------------
     */
    i = numObjs;
    while (heapLength > 0) {
        /* Select object to output by removing largest heap member */
        j = removeHeapElement(pendingHeap, heapLength--);
        obj = objs[j];
        /* Output candidate to ordering[] */
        ordering[--i] = obj;
        /* Update beforeConstraints counts of its predecessors */
        for (k = 0; k < obj->nDeps; k++) {
            int id = obj->dependencies[k];

            if ((--beforeConstraints[id]) == 0)
                addHeapElement(idMap[id], pendingHeap, heapLength++);
        }
    }

    /*
     * If we failed, report the objects that couldn't be output; these are the
     * ones with beforeConstraints[] still nonzero.
     */
    if (i != 0) {
        k = 0;
        for (j = 1; j <= maxDumpId; j++) {
            if (beforeConstraints[j] != 0)
                ordering[k++] = objs[idMap[j]];
        }
        *nOrdering = k;
    }

    /* Done */
    free(pendingHeap);
    pendingHeap = NULL;
    free(beforeConstraints);
    beforeConstraints = NULL;
    free(idMap);
    idMap = NULL;

    return (i == 0);
}

/*
 * Add an item to a heap (priority queue)
 *
 * heapLength is the current heap size; caller is responsible for increasing
 * its value after the call.  There must be sufficient storage at *heap.
 */
static void addHeapElement(int val, int* heap, int heapLength)
{
    int j;

    /*
     * Sift-up the new entry, per Knuth 5.2.3 exercise 16. Note that Knuth is
     * using 1-based array indexes, not 0-based.
     */
    j = heapLength;
    while (j > 0) {
        int i = (uint32)(j - 1) >> 1;

        if (val <= heap[i])
            break;
        heap[j] = heap[i];
        j = i;
    }
    heap[j] = val;
}

/*
 * Remove the largest item present in a heap (priority queue)
 *
 * heapLength is the current heap size; caller is responsible for decreasing
 * its value after the call.
 *
 * We remove and return heap[0], which is always the largest element of
 * the heap, and then "sift up" to maintain the heap invariant.
 */
static int removeHeapElement(int* heap, int heapLength)
{
    int result = heap[0];
    int val;
    int i;

    if (--heapLength <= 0)
        return result;
    val = heap[heapLength]; /* value that must be reinserted */
    i = 0;                  /* i is where the "hole" is */
    for (;;) {
        int j = 2 * i + 1;

        if (j >= heapLength)
            break;
        if (j + 1 < heapLength && heap[j] < heap[(int)(j + 1)])
            j++;
        if (val >= heap[j])
            break;
        heap[i] = heap[j];
        i = j;
    }
    heap[i] = val;
    return result;
}

/*
 * findDependencyLoops - identify loops in TopoSort's failure output,
 *		and pass each such loop to repairDependencyLoop() for action
 *
 * In general there may be many loops in the set of objects returned by
 * TopoSort; for speed we should try to repair as many loops as we can
 * before trying TopoSort again.  We can safely repair loops that are
 * disjoint (have no members in common); if we find overlapping loops
 * then we repair only the first one found, because the action taken to
 * repair the first might have repaired the other as well.	(If not,
 * we'll fix it on the next go-round.)
 *
 * objs[] lists the objects TopoSort couldn't sort
 * nObjs is the number of such objects
 * totObjs is the total number of objects in the universe
 */
static void findDependencyLoops(DumpableObject** objs, int nObjs, int totObjs)
{
    /*
     * We use two data structures here.  One is a bool array processed[],
     * which is indexed by dump ID and marks the objects already processed
     * during this invocation of findDependencyLoops().  The other is a
     * workspace[] array of DumpableObject pointers, in which we try to build
     * lists of objects constituting loops.  We make workspace[] large enough
     * to hold all the objects, which is huge overkill in most cases but could
     * theoretically be necessary if there is a single dependency chain
     * linking all the objects.
     */
    bool* processed = NULL;
    DumpableObject** workspace = NULL;
    ;
    bool fixedloop = false;
    int i;

    processed = (bool*)pg_calloc(getMaxDumpId() + 1, sizeof(bool));
    workspace = (DumpableObject**)pg_malloc(totObjs * sizeof(DumpableObject*));
    fixedloop = false;

    for (i = 0; i < nObjs; i++) {
        DumpableObject* obj = objs[i];
        int looplen;
        int j;

        looplen = findLoop(obj, obj->dumpId, processed, workspace, 0);

        if (looplen > 0) {
            /* Found a loop, repair it */
            repairDependencyLoop(workspace, looplen);
            fixedloop = true;
            /* Mark loop members as processed */
            for (j = 0; j < looplen; j++)
                processed[workspace[j]->dumpId] = true;
        } else {
            /*
             * There's no loop starting at this object, but mark it processed
             * anyway.	This is not necessary for correctness, but saves later
             * invocations of findLoop() from uselessly chasing references to
             * such an object.
             */
            processed[obj->dumpId] = true;
        }
    }

    /* We'd better have fixed at least one loop */
    if (!fixedloop)
        exit_horribly(modulename, "could not identify dependency loop\n");

    free(workspace);
    workspace = NULL;
    free(processed);
    processed = NULL;
}

/*
 * Recursively search for a circular dependency loop that doesn't include
 * any already-processed objects.
 *
 *	obj: object we are examining now
 *	startPoint: dumpId of starting object for the hoped-for circular loop
 *	processed[]: flag array marking already-processed objects
 *	workspace[]: work array in which we are building list of loop members
 *	depth: number of valid entries in workspace[] at call
 *
 * On success, the length of the loop is returned, and workspace[] is filled
 * with pointers to the members of the loop.  On failure, we return 0.
 *
 * Note: it is possible that the given starting object is a member of more
 * than one cycle; if so, we will find an arbitrary one of the cycles.
 */
static int findLoop(DumpableObject* obj, DumpId startPoint, bool* processed, DumpableObject** workspace, int depth)
{
    int i;

    /*
     * Reject if obj is already processed.	This test prevents us from finding
     * loops that overlap previously-processed loops.
     */
    if (processed[obj->dumpId])
        return 0;

    /*
     * Reject if obj is already present in workspace.  This test prevents us
     * from going into infinite recursion if we are given a startPoint object
     * that links to a cycle it's not a member of, and it guarantees that we
     * can't overflow the allocated size of workspace[].
     */
    for (i = 0; i < depth; i++) {
        if (workspace[i] == obj)
            return 0;
    }

    /*
     * Okay, tentatively add obj to workspace
     */
    workspace[depth++] = obj;

    /*
     * See if we've found a loop back to the desired startPoint; if so, done
     */
    for (i = 0; i < obj->nDeps; i++) {
        if (obj->dependencies[i] == startPoint)
            return depth;
    }

    /*
     * Recurse down each outgoing branch
     */
    for (i = 0; i < obj->nDeps; i++) {
        DumpableObject* nextobj = findObjectByDumpId(obj->dependencies[i]);
        int newDepth;

        if (nextobj == NULL)
            continue; /* ignore dependencies on undumped objects */
        newDepth = findLoop(nextobj, startPoint, processed, workspace, depth);
        if (newDepth > 0)
            return newDepth;
    }

    return 0;
}

/*
 * A user-defined datatype will have a dependency loop with each of its
 * I/O functions (since those have the datatype as input or output).
 * Similarly, a range type will have a loop with its canonicalize function,
 * if any.	Break the loop by making the function depend on the associated
 * shell type, instead.
 */
static void repairTypeFuncLoop(DumpableObject* typeobj, DumpableObject* funcobj)
{
    TypeInfo* typeInfo = (TypeInfo*)typeobj;

    /* remove function's dependency on type */
    removeObjectDependency(funcobj, typeobj->dumpId);

    /* add function's dependency on shell type, instead */
    if (typeInfo->shellType != NULL) {
        addObjectDependency(funcobj, typeInfo->shellType->dobj.dumpId);
        /* Mark shell type as to be dumped if any such function is */
        if (funcobj->dump)
            typeInfo->shellType->dobj.dump = true;
    }
}

/*
 * Because we force a view to depend on its ON SELECT rule, while there
 * will be an implicit dependency in the other direction, we need to break
 * the loop.  If there are no other objects in the loop then we can remove
 * the implicit dependency and leave the ON SELECT rule non-separate.
 */
static void repairViewRuleLoop(DumpableObject* viewobj, DumpableObject* ruleobj)
{
    /* remove rule's dependency on view */
    removeObjectDependency(ruleobj, viewobj->dumpId);
}

/*
 * However, if there are other objects in the loop, we must break the loop
 * by making the ON SELECT rule a separately-dumped object.
 *
 * Because findLoop() finds shorter cycles before longer ones, it's likely
 * that we will have previously fired repairViewRuleLoop() and removed the
 * rule's dependency on the view.  Put it back to ensure the rule won't be
 * emitted before the view...
 */
static void repairViewRuleMultiLoop(DumpableObject* viewobj, DumpableObject* ruleobj)
{
    TableInfo* viewinfo = (TableInfo*)viewobj;
    RuleInfo* ruleinfo = (RuleInfo*)ruleobj;

    /* remove view's dependency on rule */
    removeObjectDependency(viewobj, ruleobj->dumpId);
    /* pretend view is a plain table and dump it that way */
    viewinfo->relkind = 'r'; /* RELKIND_RELATION */
    /* mark rule as needing its own dump */
    ruleinfo->separate = true;
    /* move any reloptions from view to rule */
    if (viewinfo->reloptions != NULL) {
        ruleinfo->reloptions = viewinfo->reloptions;
        viewinfo->reloptions = NULL;
    }
    /* put back rule's dependency on view */
    addObjectDependency(ruleobj, viewobj->dumpId);
    /* now that rule is separate, it must be post-data */
    addObjectDependency(ruleobj, postDataBoundId);
}

/*
 * Because we make tables depend on their CHECK constraints, while there
 * will be an automatic dependency in the other direction, we need to break
 * the loop.  If there are no other objects in the loop then we can remove
 * the automatic dependency and leave the CHECK constraint non-separate.
 */
static void repairTableConstraintLoop(DumpableObject* tableobj, DumpableObject* constraintobj)
{
    /* remove constraint's dependency on table */
    removeObjectDependency(constraintobj, tableobj->dumpId);
}

/*
 * However, if there are other objects in the loop, we must break the loop
 * by making the CHECK constraint a separately-dumped object.
 *
 * Because findLoop() finds shorter cycles before longer ones, it's likely
 * that we will have previously fired repairTableConstraintLoop() and
 * removed the constraint's dependency on the table.  Put it back to ensure
 * the constraint won't be emitted before the table...
 */
static void repairTableConstraintMultiLoop(DumpableObject* tableobj, DumpableObject* constraintobj)
{
    /* remove table's dependency on constraint */
    removeObjectDependency(tableobj, constraintobj->dumpId);
    /* mark constraint as needing its own dump */
    ((ConstraintInfo*)constraintobj)->separate = true;
    /* put back constraint's dependency on table */
    addObjectDependency(constraintobj, tableobj->dumpId);
    /* now that constraint is separate, it must be post-data */
    addObjectDependency(constraintobj, postDataBoundId);
}

/*
 * Attribute defaults behave exactly the same as CHECK constraints...
 */
static void repairTableAttrDefLoop(DumpableObject* tableobj, DumpableObject* attrdefobj)
{
    /* remove attrdef's dependency on table */
    removeObjectDependency(attrdefobj, tableobj->dumpId);
}

static void repairTableAttrDefMultiLoop(DumpableObject* tableobj, DumpableObject* attrdefobj)
{
    /* remove table's dependency on attrdef */
    removeObjectDependency(tableobj, attrdefobj->dumpId);
    /* mark attrdef as needing its own dump */
    ((AttrDefInfo*)attrdefobj)->separate = true;
    /* put back attrdef's dependency on table */
    addObjectDependency(attrdefobj, tableobj->dumpId);
}

/*
 * CHECK constraints on domains work just like those on tables ...
 */
static void repairDomainConstraintLoop(DumpableObject* domainobj, DumpableObject* constraintobj)
{
    /* remove constraint's dependency on domain */
    removeObjectDependency(constraintobj, domainobj->dumpId);
}

static void repairDomainConstraintMultiLoop(DumpableObject* domainobj, DumpableObject* constraintobj)
{
    /* remove domain's dependency on constraint */
    removeObjectDependency(domainobj, constraintobj->dumpId);
    /* mark constraint as needing its own dump */
    ((ConstraintInfo*)constraintobj)->separate = true;
    /* put back constraint's dependency on domain */
    addObjectDependency(constraintobj, domainobj->dumpId);
    /* now that constraint is separate, it must be post-data */
    addObjectDependency(constraintobj, postDataBoundId);
}

/*
 * Fix a dependency loop, or die trying ...
 *
 * This routine is mainly concerned with reducing the multiple ways that
 * a loop might appear to common cases, which it passes off to the
 * "fixer" routines above.
 */
static bool repairDependencyFuncLoops(DumpableObject** loop, int nLoop)
{

    /* Datatype and one of its I/O or canonicalize functions */
    if (nLoop == 2 && loop[0]->objType == DO_TYPE && loop[1]->objType == DO_FUNC) {
        repairTypeFuncLoop(loop[0], loop[1]);
        return true;
    }
    if (nLoop == 2 && loop[1]->objType == DO_TYPE && loop[0]->objType == DO_FUNC) {
        repairTypeFuncLoop(loop[1], loop[0]);
        return true;
    }
    return false;
}
static bool repairDependencyViewLoops(DumpableObject** loop, int nLoop)
{
    int i = 0;
    int j = 0;

    /* View and its ON SELECT rule */
    if (nLoop == 2 && loop[0]->objType == DO_TABLE && loop[1]->objType == DO_RULE &&
        ((RuleInfo*)loop[1])->ev_type == '1' && ((RuleInfo*)loop[1])->is_instead &&
        ((RuleInfo*)loop[1])->ruletable == (TableInfo*)loop[0]) {
        repairViewRuleLoop(loop[0], loop[1]);
        return true;
    }
    if (nLoop == 2 && loop[1]->objType == DO_TABLE && loop[0]->objType == DO_RULE &&
        ((RuleInfo*)loop[0])->ev_type == '1' && ((RuleInfo*)loop[0])->is_instead &&
        ((RuleInfo*)loop[0])->ruletable == (TableInfo*)loop[1]) {
        repairViewRuleLoop(loop[1], loop[0]);
        return true;
    }

    /* Indirect loop involving view and ON SELECT rule */
    if (nLoop > 2) {
        for (i = 0; i < nLoop; i++) {
            if (loop[i]->objType == DO_TABLE) {
                for (j = 0; j < nLoop; j++) {
                    if (loop[j]->objType == DO_RULE && ((RuleInfo*)loop[j])->ev_type == '1' &&
                        ((RuleInfo*)loop[j])->is_instead && ((RuleInfo*)loop[j])->ruletable == (TableInfo*)loop[i]) {
                        repairViewRuleMultiLoop(loop[i], loop[j]);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
static bool repairDependencyTblChkConstraintLoops(DumpableObject** loop, int nLoop)
{
    int i = 0;
    int j = 0;

    /* Table and CHECK constraint */
    if (nLoop == 2 && loop[0]->objType == DO_TABLE && loop[1]->objType == DO_CONSTRAINT &&
        ((ConstraintInfo*)loop[1])->contype == 'c' && ((ConstraintInfo*)loop[1])->contable == (TableInfo*)loop[0]) {
        repairTableConstraintLoop(loop[0], loop[1]);
        return true;
    }
    if (nLoop == 2 && loop[1]->objType == DO_TABLE && loop[0]->objType == DO_CONSTRAINT &&
        ((ConstraintInfo*)loop[0])->contype == 'c' && ((ConstraintInfo*)loop[0])->contable == (TableInfo*)loop[1]) {
        repairTableConstraintLoop(loop[1], loop[0]);
        return true;
    }

    /* Indirect loop involving table and CHECK constraint */
    if (nLoop > 2) {
        for (i = 0; i < nLoop; i++) {
            if (loop[i]->objType == DO_TABLE) {
                for (j = 0; j < nLoop; j++) {
                    if (loop[j]->objType == DO_CONSTRAINT && ((ConstraintInfo*)loop[j])->contype == 'c' &&
                        ((ConstraintInfo*)loop[j])->contable == (TableInfo*)loop[i]) {
                        repairTableConstraintMultiLoop(loop[i], loop[j]);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
static bool repairDependencyTblAttrLoops(DumpableObject** loop, int nLoop)
{
    int i = 0;
    int j = 0;
    /* Table and attribute default */
    if (nLoop == 2 && loop[0]->objType == DO_TABLE && loop[1]->objType == DO_ATTRDEF &&
        ((AttrDefInfo*)loop[1])->adtable == (TableInfo*)loop[0]) {
        repairTableAttrDefLoop(loop[0], loop[1]);
        return true;
    }
    if (nLoop == 2 && loop[1]->objType == DO_TABLE && loop[0]->objType == DO_ATTRDEF &&
        ((AttrDefInfo*)loop[0])->adtable == (TableInfo*)loop[1]) {
        repairTableAttrDefLoop(loop[1], loop[0]);
        return true;
    }

    /* Indirect loop involving table and attribute default */
    if (nLoop > 2) {
        for (i = 0; i < nLoop; i++) {
            if (loop[i]->objType == DO_TABLE) {
                for (j = 0; j < nLoop; j++) {
                    if (loop[j]->objType == DO_ATTRDEF && ((AttrDefInfo*)loop[j])->adtable == (TableInfo*)loop[i]) {
                        repairTableAttrDefMultiLoop(loop[i], loop[j]);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
static bool repairDependencyDomainConstraintLoops(DumpableObject** loop, int nLoop)
{
    int i = 0;
    int j = 0;

    /* Domain and CHECK constraint */
    if (nLoop == 2 && loop[0]->objType == DO_TYPE && loop[1]->objType == DO_CONSTRAINT &&
        ((ConstraintInfo*)loop[1])->contype == 'c' && ((ConstraintInfo*)loop[1])->condomain == (TypeInfo*)loop[0]) {
        repairDomainConstraintLoop(loop[0], loop[1]);
        return true;
    }
    if (nLoop == 2 && loop[1]->objType == DO_TYPE && loop[0]->objType == DO_CONSTRAINT &&
        ((ConstraintInfo*)loop[0])->contype == 'c' && ((ConstraintInfo*)loop[0])->condomain == (TypeInfo*)loop[1]) {
        repairDomainConstraintLoop(loop[1], loop[0]);
        return true;
    }

    /* Indirect loop involving domain and CHECK constraint */
    if (nLoop > 2) {
        for (i = 0; i < nLoop; i++) {
            if (loop[i]->objType == DO_TYPE) {
                for (j = 0; j < nLoop; j++) {
                    if (loop[j]->objType == DO_CONSTRAINT && ((ConstraintInfo*)loop[j])->contype == 'c' &&
                        ((ConstraintInfo*)loop[j])->condomain == (TypeInfo*)loop[i]) {
                        repairDomainConstraintMultiLoop(loop[i], loop[j]);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static void repairDependencyLoop(DumpableObject** loop, int nLoop)
{
    int i;

    if (true == repairDependencyFuncLoops(loop, nLoop))
        return;
    if (true == repairDependencyViewLoops(loop, nLoop))
        return;
    if (true == repairDependencyTblChkConstraintLoops(loop, nLoop))
        return;
    if (true == repairDependencyTblAttrLoops(loop, nLoop))
        return;
    if (true == repairDependencyDomainConstraintLoops(loop, nLoop))
        return;

    /*
     * Loop of table with itself --- just ignore it.
     *
     * (Actually, what this arises from is a dependency of a table column on
     * another column, which happens with generated columns; or a dependency
     * of a table column on the whole table, which happens with partitioning.
     * But we didn't pay attention to sub-object IDs while collecting the
     * dependency data, so we can't see that here.)
     */
    if (nLoop == 1) {
        if (loop[0]->objType == DO_TABLE || loop[0]->objType == DO_FUNC) {
            removeObjectDependency(loop[0], loop[0]->dumpId);
            return;
        }
    }

    /*
     * If all the objects are TABLE_DATA items, what we must have is a
     * circular set of foreign key constraints (or a single self-referential
     * table).	Print an appropriate complaint and break the loop arbitrarily.
     */
    for (i = 0; i < nLoop; i++) {
        if (loop[i]->objType != DO_TABLE_DATA)
            break;
    }
    if (i >= nLoop) {
        write_msg(NULL, "NOTICE: there are circular foreign-key constraints among these table(s):\n");
        for (i = 0; i < nLoop; i++)
            write_msg(NULL, "  %s\n", loop[i]->name);
        write_msg(NULL,
            "You might not be able to restore the dump without using --disable-triggers or temporarily dropping the "
            "constraints.\n");
        write_msg(NULL, "Consider using a full dump instead of a --data-only dump to avoid this problem.\n");
        if (nLoop > 1)
            removeObjectDependency(loop[0], loop[1]->dumpId);
        else /* must be a self-dependency */
            removeObjectDependency(loop[0], loop[0]->dumpId);
        return;
    }

    /*
     * If we can't find a principled way to break the loop, complain and break
     * it in an arbitrary fashion.
     */
    write_msg(modulename, "WARNING: could not resolve dependency loop among these items:\n");
    for (i = 0; i < nLoop; i++) {
        char buf[1024] = {0};

        describeDumpableObject(loop[i], buf, sizeof(buf));
        if (strlen(buf) > 0)
            write_msg(modulename, "  %s\n", buf);
    }

    if (nLoop > 1)
        removeObjectDependency(loop[0], loop[1]->dumpId);
    else /* must be a self-dependency */
        removeObjectDependency(loop[0], loop[0]->dumpId);
}

/*
 * Describe a dumpable object usefully for errors
 *
 * This should probably go somewhere else...
 */
static void describeDumpableObject(DumpableObject* obj, char* buf, int bufsize)
{
    int nRet = 0;
    switch (obj->objType) {
        case DO_NAMESPACE:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "SCHEMA %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;

        case DO_EVENT_TRIGGER:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "EVENT TRIGGER %s (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
            
        case DO_EXTENSION:
            if (true == include_extensions) {
                nRet = snprintf_s(
                    buf, bufsize, bufsize - 1, "EXTENSION %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
                securec_check_ss_c(nRet, "\0", "\0");
            }

            return;
        case DO_TYPE:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "TYPE %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_SHELL_TYPE:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "SHELL TYPE %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_FUNC:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "FUNCTION %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_AGG:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "AGGREGATE %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_OPERATOR:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "OPERATOR %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_ACCESS_METHOD:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "ACCESS METHOD %s  (ID %d OID %u)",obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_OPCLASS:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "OPERATOR CLASS %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_OPFAMILY:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "OPERATOR FAMILY %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_COLLATION:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "COLLATION %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_CONVERSION:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "CONVERSION %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TABLE:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "TABLE %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_ATTRDEF:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "ATTRDEF %s.%s  (ID %d OID %u)",
                ((AttrDefInfo*)obj)->adtable->dobj.name,
                ((AttrDefInfo*)obj)->adtable->attnames[((AttrDefInfo*)obj)->adnum - 1],
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_INDEX:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "INDEX %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_RULE:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "RULE %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TRIGGER:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "TRIGGER %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_EVENT:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "EVENT %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_CONSTRAINT:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "CONSTRAINT %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_FK_CONSTRAINT:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "FK CONSTRAINT %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_PROCLANG:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "PROCEDURAL LANGUAGE %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_CAST:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "CAST %u to %u  (ID %d OID %u)",
                ((CastInfo*)obj)->castsource,
                ((CastInfo*)obj)->casttarget,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TABLE_DATA:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "TABLE DATA %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_DUMMY_TYPE:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "DUMMY TYPE %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TSPARSER:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "TEXT SEARCH PARSER %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TSDICT:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "TEXT SEARCH DICTIONARY %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TSTEMPLATE:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "TEXT SEARCH TEMPLATE %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_TSCONFIG:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "TEXT SEARCH CONFIGURATION %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_FDW:
            nRet = snprintf_s(buf,
                bufsize,
                bufsize - 1,
                "FOREIGN DATA WRAPPER %s  (ID %d OID %u)",
                obj->name,
                obj->dumpId,
                obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_FOREIGN_SERVER:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "FOREIGN SERVER %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_DEFAULT_ACL:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "DEFAULT ACL %s  (ID %d OID %u)", obj->name, obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_BLOB:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "BLOB  (ID %d OID %u)", obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_BLOB_DATA:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "BLOB DATA  (ID %d)", obj->dumpId);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_PRE_DATA_BOUNDARY:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "PRE-DATA BOUNDARY  (ID %d)", obj->dumpId);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_POST_DATA_BOUNDARY:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "POST-DATA BOUNDARY  (ID %d)", obj->dumpId);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_RLSPOLICY:
            nRet = snprintf_s(
                buf, bufsize, bufsize - 1, "ROW LEVEL SECURITY POLICY (ID %d OID %u)", obj->dumpId, obj->catId.oid);
            return;
        case DO_PUBLICATION:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "PUBLICATION (ID %d OID %u)", obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_PUBLICATION_REL:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "PUBLICATION TABLE (ID %d OID %u)",
                obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        case DO_SUBSCRIPTION:
            nRet = snprintf_s(buf, bufsize, bufsize - 1, "SUBSCRIPTION (ID %d OID %u)", obj->dumpId, obj->catId.oid);
            securec_check_ss_c(nRet, "\0", "\0");
            return;
        default:
            break;
    }
    /* shouldn't get here */
    nRet = snprintf_s(
        buf, bufsize, bufsize - 1, "object type %d  (ID %d OID %u)", (int)obj->objType, obj->dumpId, obj->catId.oid);
    securec_check_ss_c(nRet, "\0", "\0");
}
