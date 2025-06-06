/* -------------------------------------------------------------------------
 *
 * gistbuildbuffers.cpp
 *	  node buffer management functions for GiST buffering build algorithm.
 *
 *
 * Portions Copyright (c) 2020 Huawei Technologies Co.,Ltd.
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/gausskernel/storage/access/gist/gistbuildbuffers.cpp
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"
#include "knl/knl_variable.h"

#include "access/genam.h"
#include "access/gist_private.h"
#include "catalog/index.h"
#include "miscadmin.h"
#include "storage/buf/buffile.h"
#include "storage/buf/bufmgr.h"
#include "utils/memutils.h"
#include "utils/rel.h"
#include "utils/rel_gs.h"

static GISTNodeBufferPage *gistAllocateNewPageBuffer(GISTBuildBuffers *gfbb);
static void gistAddLoadedBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer);
static void gistLoadNodeBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer);
static void gistUnloadNodeBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer);
static void gistPlaceItupToPage(GISTNodeBufferPage *pageBuffer, IndexTuple item);
static void gistGetItupFromPage(GISTNodeBufferPage *pageBuffer, IndexTuple *item);
static long gistBuffersGetFreeBlock(GISTBuildBuffers *gfbb);
static void gistBuffersReleaseBlock(GISTBuildBuffers *gfbb, long blocknum);

static void ReadTempFileBlock(BufFile *file, long blknum, void *ptr);
static void WriteTempFileBlock(BufFile *file, long blknum, void *ptr);

/*
 * Initialize GiST build buffers.
 */
GISTBuildBuffers *gistInitBuildBuffers(int pagesPerBuffer, int levelStep, int maxLevel)
{
    GISTBuildBuffers *gfbb = NULL;
    HASHCTL hashCtl;
    errno_t rc;
    /* Initialize the hash table. */
    rc = memset_s(&hashCtl, sizeof(hashCtl), 0, sizeof(hashCtl));
    securec_check(rc, "", "");

    gfbb = (GISTBuildBuffers *)palloc(sizeof(GISTBuildBuffers));
    gfbb->pagesPerBuffer = pagesPerBuffer;
    gfbb->levelStep = levelStep;

    /*
     * Create a temporary file to hold buffer pages that are swapped out of
     * memory.
     */
    gfbb->pfile = BufFileCreateTemp(false);
    gfbb->nFileBlocks = 0;

    /* Initialize free page management. */
    gfbb->nFreeBlocks = 0;
    gfbb->freeBlocksLen = 32;
    gfbb->freeBlocks = (long *)palloc((Size)(gfbb->freeBlocksLen) * sizeof(long));

    /*
     * Current memory context will be used for all in-memory data structures
     * of buffers which are persistent during buffering build.
     */
    gfbb->context = CurrentMemoryContext;

    /*
     * nodeBuffersTab hash is association between index blocks and it's
     * buffers.
     */
    hashCtl.keysize = sizeof(BlockNumber);
    hashCtl.entrysize = sizeof(GISTNodeBuffer);
    hashCtl.hcxt = CurrentMemoryContext;
    hashCtl.hash = tag_hash;
    hashCtl.match = memcmp;
    gfbb->nodeBuffersTab = hash_create("gistbuildbuffers", 1024, &hashCtl,
                                       HASH_ELEM | HASH_CONTEXT | HASH_FUNCTION | HASH_COMPARE);

    gfbb->bufferEmptyingQueue = NIL;

    /*
     * Per-level node buffers lists for final buffers emptying process. Node
     * buffers are inserted here when they are created.
     */
    gfbb->buffersOnLevelsLen = 1;
    gfbb->buffersOnLevels = (List **)palloc(sizeof(List *) * (Size)(gfbb->buffersOnLevelsLen));
    gfbb->buffersOnLevels[0] = NIL;

    /*
     * Block numbers of node buffers which last pages are currently loaded
     * into main memory.
     */
    gfbb->loadedBuffersLen = 32;
    gfbb->loadedBuffers = (GISTNodeBuffer **)palloc((Size)(gfbb->loadedBuffersLen) * sizeof(GISTNodeBuffer *));
    gfbb->loadedBuffersCount = 0;

    gfbb->rootlevel = maxLevel;

    return gfbb;
}

/*
 * Returns a node buffer for given block. The buffer is created if it
 * doesn't exist yet.
 */
GISTNodeBuffer *gistGetNodeBuffer(GISTBuildBuffers *gfbb, GISTSTATE *giststate, BlockNumber nodeBlocknum, int level)
{
    GISTNodeBuffer *nodeBuffer = NULL;
    bool found = false;

    /* Find node buffer in hash table */
    nodeBuffer = (GISTNodeBuffer *)hash_search(gfbb->nodeBuffersTab, (const void *)&nodeBlocknum, HASH_ENTER, &found);
    if (!found) {
        /*
         * Node buffer wasn't found. Initialize the new buffer as empty.
         */
        MemoryContext oldcxt = MemoryContextSwitchTo(gfbb->context);

        /* nodeBuffer->nodeBlocknum is the hash key and was filled in already */
        nodeBuffer->blocksCount = 0;
        nodeBuffer->pageBlocknum = InvalidBlockNumber;
        nodeBuffer->pageBuffer = NULL;
        nodeBuffer->queuedForEmptying = false;
        nodeBuffer->level = level;

        /*
         * Add this buffer to the list of buffers on this level. Enlarge
         * buffersOnLevels array if needed.
         */
        if (level >= gfbb->buffersOnLevelsLen) {
            int i;

            gfbb->buffersOnLevels = (List **)repalloc(gfbb->buffersOnLevels, (Size)(level + 1) * sizeof(List *));

            /* initialize the enlarged portion */
            for (i = gfbb->buffersOnLevelsLen; i <= level; i++)
                gfbb->buffersOnLevels[i] = NIL;
            gfbb->buffersOnLevelsLen = level + 1;
        }

        /*
         * Prepend the new buffer to the list of buffers on this level. It's
         * not arbitrary that the new buffer is put to the beginning of the
         * list: in the final emptying phase we loop through all buffers at
         * each level, and flush them. If a page is split during the emptying,
         * it's more efficient to flush the new splitted pages first, before
         * moving on to pre-existing pages on the level. The buffers just
         * created during the page split are likely still in cache, so
         * flushing them immediately is more efficient than putting them to
         * the end of the queue.
         */
        gfbb->buffersOnLevels[level] = lcons(nodeBuffer, gfbb->buffersOnLevels[level]);

        (void)MemoryContextSwitchTo(oldcxt);
    }

    return nodeBuffer;
}

/*
 * Allocate memory for a buffer page.
 */
static GISTNodeBufferPage *gistAllocateNewPageBuffer(GISTBuildBuffers *gfbb)
{
    GISTNodeBufferPage *pageBuffer = NULL;

    pageBuffer = (GISTNodeBufferPage *)MemoryContextAlloc(gfbb->context, BLCKSZ);
    pageBuffer->prev = InvalidBlockNumber;

    /* Set page free space */
    PAGE_FREE_SPACE(pageBuffer) = BLCKSZ - BUFFER_PAGE_DATA_OFFSET;
    return pageBuffer;
}

/*
 * Add specified buffer into loadedBuffers array.
 */
static void gistAddLoadedBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer)
{
    /* Never add a temporary buffer to the array */
    if (nodeBuffer->isTemp)
        return;

    /* Enlarge the array if needed */
    if (gfbb->loadedBuffersCount >= gfbb->loadedBuffersLen) {
        gfbb->loadedBuffersLen *= 2;
        gfbb->loadedBuffers = (GISTNodeBuffer **)repalloc(gfbb->loadedBuffers,
                                                          (Size)(gfbb->loadedBuffersLen) * sizeof(GISTNodeBuffer *));
    }

    gfbb->loadedBuffers[gfbb->loadedBuffersCount] = nodeBuffer;
    gfbb->loadedBuffersCount++;
}

/*
 * Load last page of node buffer into main memory.
 */
static void gistLoadNodeBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer)
{
    /* Check if we really should load something */
    if (!nodeBuffer->pageBuffer && nodeBuffer->blocksCount > 0) {
        /* Allocate memory for page */
        nodeBuffer->pageBuffer = gistAllocateNewPageBuffer(gfbb);

        /* Read block from temporary file */
        ReadTempFileBlock(gfbb->pfile, nodeBuffer->pageBlocknum, nodeBuffer->pageBuffer);

        /* Mark file block as free */
        gistBuffersReleaseBlock(gfbb, nodeBuffer->pageBlocknum);

        /* Mark node buffer as loaded */
        gistAddLoadedBuffer(gfbb, nodeBuffer);
        nodeBuffer->pageBlocknum = InvalidBlockNumber;
    }
}

/*
 * Write last page of node buffer to the disk.
 */
static void gistUnloadNodeBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer)
{
    /* Check if we have something to write */
    if (nodeBuffer->pageBuffer) {
        BlockNumber blkno;

        /* Get free file block */
        blkno = (BlockNumber)gistBuffersGetFreeBlock(gfbb);

        /* Write block to the temporary file */
        WriteTempFileBlock(gfbb->pfile, blkno, nodeBuffer->pageBuffer);

        /* Free memory of that page */
        pfree(nodeBuffer->pageBuffer);
        nodeBuffer->pageBuffer = NULL;

        /* Save block number */
        nodeBuffer->pageBlocknum = blkno;
    }
}

/*
 * Write last pages of all node buffers to the disk.
 */
void gistUnloadNodeBuffers(GISTBuildBuffers *gfbb)
{
    int i;

    /* Unload all the buffers that have a page loaded in memory. */
    for (i = 0; i < gfbb->loadedBuffersCount; i++)
        gistUnloadNodeBuffer(gfbb, gfbb->loadedBuffers[i]);

    /* Now there are no node buffers with loaded last page */
    gfbb->loadedBuffersCount = 0;
}

/*
 * Add index tuple to buffer page.
 */
static void gistPlaceItupToPage(GISTNodeBufferPage *pageBuffer, IndexTuple itup)
{
    Size itupsz = IndexTupleSize(itup);
    char *ptr = NULL;
    errno_t ret = EOK;

    /* There should be enough of space. */
    Assert(PAGE_FREE_SPACE(pageBuffer) >= MAXALIGN(itupsz));

    /* Reduce free space value of page to reserve a spot for the tuple. */
    PAGE_FREE_SPACE(pageBuffer) -= MAXALIGN(itupsz);

    /* Get pointer to the spot we reserved (ie. end of free space). */
    ptr = (char *)pageBuffer + BUFFER_PAGE_DATA_OFFSET + PAGE_FREE_SPACE(pageBuffer);

    /* Copy the index tuple there. */
    ret = memcpy_s(ptr, MAXALIGN(itupsz), itup, itupsz);
    securec_check(ret, "", "");
}

/*
 * Get last item from buffer page and remove it from page.
 */
static void gistGetItupFromPage(GISTNodeBufferPage *pageBuffer, IndexTuple *itup)
{
    IndexTuple ptr;
    Size itupsz;
    errno_t ret;

    Assert(!PAGE_IS_EMPTY(pageBuffer)); /* Page shouldn't be empty */

    /* Get pointer to last index tuple */
    ptr = (IndexTuple)((char *)pageBuffer + BUFFER_PAGE_DATA_OFFSET + PAGE_FREE_SPACE(pageBuffer));
    itupsz = IndexTupleSize(ptr);

    /* Make a copy of the tuple */
    *itup = (IndexTuple)palloc(itupsz);
    ret = memcpy_s(*itup, itupsz, ptr, itupsz);
    securec_check(ret, "", "");

    /* Mark the space used by the tuple as free */
    PAGE_FREE_SPACE(pageBuffer) += (uint32)(MAXALIGN(itupsz));
}

/*
 * Push an index tuple to node buffer.
 */
void gistPushItupToNodeBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer, IndexTuple itup)
{
    /*
     * Most part of memory operations will be in buffering build persistent
     * context. So, let's switch to it.
     */
    MemoryContext oldcxt = MemoryContextSwitchTo(gfbb->context);

    /*
     * If the buffer is currently empty, create the first page.
     */
    if (nodeBuffer->blocksCount == 0) {
        nodeBuffer->pageBuffer = gistAllocateNewPageBuffer(gfbb);
        nodeBuffer->blocksCount = 1;
        gistAddLoadedBuffer(gfbb, nodeBuffer);
    }

    /* Load last page of node buffer if it wasn't in memory already */
    if (!nodeBuffer->pageBuffer)
        gistLoadNodeBuffer(gfbb, nodeBuffer);

    /*
     * Check if there is enough space on the last page for the tuple.
     */
    if (PAGE_NO_SPACE(nodeBuffer->pageBuffer, itup)) {
        /*
         * Nope. Swap previous block to disk and allocate a new one.
         */
        BlockNumber blkno;

        /* Write filled page to the disk */
        blkno = (BlockNumber)gistBuffersGetFreeBlock(gfbb);
        WriteTempFileBlock(gfbb->pfile, blkno, nodeBuffer->pageBuffer);

        /*
         * Reset the in-memory page as empty, and link the previous block to
         * the new page by storing its block number in the prev-link.
         */
        PAGE_FREE_SPACE(nodeBuffer->pageBuffer) = BLCKSZ - MAXALIGN(offsetof(GISTNodeBufferPage, tupledata));
        nodeBuffer->pageBuffer->prev = blkno;

        /* We've just added one more page */
        nodeBuffer->blocksCount++;
    }

    gistPlaceItupToPage(nodeBuffer->pageBuffer, itup);

    /*
     * If the buffer just overflowed, add it to the emptying queue.
     */
    if (BUFFER_HALF_FILLED(nodeBuffer, gfbb) && !nodeBuffer->queuedForEmptying) {
        gfbb->bufferEmptyingQueue = lcons(nodeBuffer, gfbb->bufferEmptyingQueue);
        nodeBuffer->queuedForEmptying = true;
    }

    /* Restore memory context */
    (void)MemoryContextSwitchTo(oldcxt);
}

/*
 * Removes one index tuple from node buffer. Returns true if success and false
 * if node buffer is empty.
 */
bool gistPopItupFromNodeBuffer(GISTBuildBuffers *gfbb, GISTNodeBuffer *nodeBuffer, IndexTuple *itup)
{
    /*
     * If node buffer is empty then return false.
     */
    if (nodeBuffer->blocksCount <= 0)
        return false;

    /* Load last page of node buffer if needed */
    if (!nodeBuffer->pageBuffer)
        gistLoadNodeBuffer(gfbb, nodeBuffer);

    /*
     * Get index tuple from last non-empty page.
     */
    gistGetItupFromPage(nodeBuffer->pageBuffer, itup);

    /*
     * If we just removed the last tuple from the page, fetch previous page on
     * this node buffer (if any).
     */
    if (PAGE_IS_EMPTY(nodeBuffer->pageBuffer)) {
        BlockNumber prevblkno;

        /*
         * blocksCount includes the page in pageBuffer, so decrease it now.
         */
        nodeBuffer->blocksCount--;

        /*
         * If there's more pages, fetch previous one.
         */
        prevblkno = nodeBuffer->pageBuffer->prev;
        if (prevblkno != InvalidBlockNumber) {
            /* There is a previous page. Fetch it. */
            Assert(nodeBuffer->blocksCount > 0);
            ReadTempFileBlock(gfbb->pfile, prevblkno, nodeBuffer->pageBuffer);

            /*
             * Now that we've read the block in memory, we can release its
             * on-disk block for reuse.
             */
            gistBuffersReleaseBlock(gfbb, prevblkno);
        } else {
            /* No more pages. Free memory. */
            Assert(nodeBuffer->blocksCount == 0);
            pfree(nodeBuffer->pageBuffer);
            nodeBuffer->pageBuffer = NULL;
        }
    }
    return true;
}

/*
 * Select a currently unused block for writing to.
 */
static long gistBuffersGetFreeBlock(GISTBuildBuffers *gfbb)
{
    /*
     * If there are multiple free blocks, we select the one appearing last in
     * freeBlocks[].  If there are none, assign the next block at the end of
     * the file (causing the file to be extended).
     */
    if (gfbb->nFreeBlocks > 0)
        return gfbb->freeBlocks[--gfbb->nFreeBlocks];
    else
        return gfbb->nFileBlocks++;
}

/*
 * Return a block# to the freelist.
 */
static void gistBuffersReleaseBlock(GISTBuildBuffers *gfbb, long blocknum)
{
    int ndx;

    /* Enlarge freeBlocks array if full. */
    if (gfbb->nFreeBlocks >= gfbb->freeBlocksLen) {
        gfbb->freeBlocksLen *= 2;
        gfbb->freeBlocks = (long *)repalloc(gfbb->freeBlocks, (Size)(gfbb->freeBlocksLen) * sizeof(long));
    }

    /* Add blocknum to array */
    ndx = gfbb->nFreeBlocks++;
    gfbb->freeBlocks[ndx] = blocknum;
}

/*
 * Free buffering build data structure.
 */
void gistFreeBuildBuffers(GISTBuildBuffers *gfbb)
{
    /* Close buffers file. */
    BufFileClose(gfbb->pfile);

    /* All other things will be freed on memory context release */
}

/*
 * Data structure representing information about node buffer for index tuples
 * relocation from splitted node buffer.
 */
typedef struct {
    GISTENTRY entry[INDEX_MAX_KEYS];
    bool isnull[INDEX_MAX_KEYS];
    GISTPageSplitInfo *splitinfo;
    GISTNodeBuffer *nodeBuffer;
} RelocationBufferInfo;

/*
 * At page split, distribute tuples from the buffer of the split page to
 * new buffers for the created page halves. This also adjusts the downlinks
 * in 'splitinfo' to include the tuples in the buffers.
 */
void gistRelocateBuildBuffersOnSplit(GISTBuildBuffers *gfbb, GISTSTATE *giststate, Relation r, int level, Buffer buffer,
                                     List *splitinfo)
{
    RelocationBufferInfo *relocationBuffersInfos = NULL;
    bool found = false;
    GISTNodeBuffer *nodeBuffer = NULL;
    BlockNumber blocknum;
    IndexTuple itup;
    int splitPagesCount = 0;
    int i = 0;
    GISTENTRY entry[INDEX_MAX_KEYS];
    bool isnull[INDEX_MAX_KEYS];
    GISTNodeBuffer oldBuf;
    ListCell *lc = NULL;
    errno_t ret = EOK;

    /* If the splitted page doesn't have buffers, we have nothing to do. */
    if (!LEVEL_HAS_BUFFERS(level, gfbb))
        return;

    /*
     * Get the node buffer of the splitted page.
     */
    blocknum = BufferGetBlockNumber(buffer);
    nodeBuffer = (GISTNodeBuffer *)hash_search(gfbb->nodeBuffersTab, &blocknum, HASH_FIND, &found);
    if (!found) {
        /* The page has no buffer, so we have nothing to do. */
        return;
    }

    /*
     * Make a copy of the old buffer, as we're going reuse it as the buffer
     * for the new left page, which is on the same block as the old page.
     * That's not true for the root page, but that's fine because we never
     * have a buffer on the root page anyway. The original algorithm as
     * described by Arge et al did, but it's of no use, as you might as well
     * read the tuples straight from the heap instead of the root buffer.
     */
    Assert(blocknum != GIST_ROOT_BLKNO);
    ret = memcpy_s(&oldBuf, sizeof(GISTNodeBuffer), nodeBuffer, sizeof(GISTNodeBuffer));
    securec_check(ret, "", "");
    oldBuf.isTemp = true;

    /* Reset the old buffer, used for the new left page from now on */
    nodeBuffer->blocksCount = 0;
    nodeBuffer->pageBuffer = NULL;
    nodeBuffer->pageBlocknum = InvalidBlockNumber;

    /*
     * Allocate memory for information about relocation buffers.
     */
    splitPagesCount = list_length(splitinfo);
    relocationBuffersInfos = (RelocationBufferInfo *)palloc(sizeof(RelocationBufferInfo) * (Size)splitPagesCount);

    /*
     * Fill relocation buffers information for node buffers of pages produced
     * by split.
     */
    i = 0;
    foreach (lc, splitinfo) {
        GISTPageSplitInfo *si = (GISTPageSplitInfo *)lfirst(lc);
        GISTNodeBuffer *newNodeBuffer = NULL;

        /* Decompress parent index tuple of node buffer page. */
        gistDeCompressAtt(giststate, r, si->downlink, NULL, (OffsetNumber)0, relocationBuffersInfos[i].entry,
                          relocationBuffersInfos[i].isnull);

        /*
         * Create a node buffer for the page. The leftmost half is on the same
         * block as the old page before split, so for the leftmost half this
         * will return the original buffer. The tuples on the original buffer
         * were relinked to the temporary buffer, so the original one is now
         * empty.
         */
        newNodeBuffer = gistGetNodeBuffer(gfbb, giststate, BufferGetBlockNumber(si->buf), level);

        relocationBuffersInfos[i].nodeBuffer = newNodeBuffer;
        relocationBuffersInfos[i].splitinfo = si;

        i++;
    }

    /*
     * Loop through all index tuples in the buffer of the page being split,
     * moving them to buffers for the new pages.  We try to move each tuple to
     * the page that will result in the lowest penalty for the leading column
     * or, in the case of a tie, the lowest penalty for the earliest column
     * that is not tied.
     *
     * The page searching logic is very similar to gistchoose().
     */
    while (gistPopItupFromNodeBuffer(gfbb, &oldBuf, &itup)) {
        float best_penalty[INDEX_MAX_KEYS];
        int which;
        IndexTuple newtup;
        RelocationBufferInfo *targetBufferInfo = NULL;

        gistDeCompressAtt(giststate, r, itup, NULL, (OffsetNumber)0, entry, isnull);

        /* default to using first page (shouldn't matter) */
        which = 0;

        /*
         * best_penalty[j] is the best penalty we have seen so far for column
         * j, or -1 when we haven't yet examined column j.  Array entries to
         * the right of the first -1 are undefined.
         */
        best_penalty[0] = -1;

        /*
         * Loop over possible target pages, looking for one to move this tuple
         * to.
         */
        for (int x = 0; x < splitPagesCount; x++) {
            RelocationBufferInfo *splitPageInfo = &relocationBuffersInfos[x];
            bool zero_penalty = false;
            int j;

            zero_penalty = true;

            /* Loop over index attributes. */
            for (j = 0; j < r->rd_att->natts; j++) {
                float usize;

                /* Compute penalty for this column. */
                usize = gistpenalty(giststate, j, &splitPageInfo->entry[j], splitPageInfo->isnull[j], &entry[j],
                                    isnull[j]);
                if (usize > 0)
                    zero_penalty = false;

                if (best_penalty[j] < 0 || usize < best_penalty[j]) {
                    /*
                     * New best penalty for column.  Tentatively select this
                     * page as the target, and record the best penalty.  Then
                     * reset the next column's penalty to "unknown" (and
                     * indirectly, the same for all the ones to its right).
                     * This will force us to adopt this page's penalty values
                     * as the best for all the remaining columns during
                     * subsequent loop iterations.
                     */
                    which = x;
                    best_penalty[j] = usize;

                    if (j < r->rd_att->natts - 1)
                        best_penalty[j + 1] = -1;
                } else if (usize > best_penalty[j]) {
                    /*
                     * The current page is worse for this column than the best
                     * page seen so far.  Skip the remaining columns and move
                     * on to the next page, if any.
                     */
                    zero_penalty = false; /* so outer loop won't exit */
                    break;
                }
                /*
                 * if best_penalty[j] == usize, The current page is exactly as
                 * good for this column as the best page seen so far. The next
                 * iteration of this loop will compare the next column.
                 */
            }

            /*
             * If we find a page with zero penalty for all columns, there's no
             * need to examine remaining pages; just break out of the loop and
             * return it.
             */
            if (zero_penalty)
                break;
        }

        /* OK, "which" is the page index to push the tuple to */
        targetBufferInfo = &relocationBuffersInfos[which];

        /* Push item to selected node buffer */
        gistPushItupToNodeBuffer(gfbb, targetBufferInfo->nodeBuffer, itup);

        /* Adjust the downlink for this page, if needed. */
        newtup = gistgetadjusted(r, targetBufferInfo->splitinfo->downlink, itup, giststate);
        if (newtup) {
            gistDeCompressAtt(giststate, r, newtup, NULL, (OffsetNumber)0, targetBufferInfo->entry,
                              targetBufferInfo->isnull);

            targetBufferInfo->splitinfo->downlink = newtup;
        }
    }

    pfree(relocationBuffersInfos);
}

/*
 * Wrappers around BufFile operations. The main difference is that these
 * wrappers report errors with ereport(), so that the callers don't need
 * to check the return code.
 */
static void ReadTempFileBlock(BufFile *file, long blknum, void *ptr)
{
    if (BufFileSeekBlock(file, blknum) != 0)
        ereport(ERROR, (errcode_for_file_access(), errmsg("could not seek temporary file: %ld", blknum)));
    if (BufFileRead(file, ptr, BLCKSZ) != BLCKSZ)
        ereport(ERROR, (errcode_for_file_access(), errmsg("could not read temporary file: %ld", blknum)));
}

static void WriteTempFileBlock(BufFile *file, long blknum, void *ptr)
{
    if (BufFileSeekBlock(file, blknum) != 0)
        ereport(ERROR, (errcode_for_file_access(), errmsg("could not seek temporary file: %ld", blknum)));
    if (BufFileWrite(file, ptr, BLCKSZ) != BLCKSZ) {
        /*
         * the other errors in Read/WriteTempFileBlock shouldn't happen, but
         * an error at write can easily happen if you run out of disk space.
         */
        ereport(ERROR, (errcode_for_file_access(), errmsg("could not write block %ld of temporary file: %s", blknum, TRANSLATE_ERRNO)));
    }
}
