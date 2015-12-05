/* this implementation of malloc, realloc and calloc are based on segregated
 * list of free blocks, built upon the version of implicit list from
 * csapp.cs.cmu.edu. The heap is maintained with a prologue block,
 * series of blocks, and then a epilogue block at the end of the heap. 
 * Each time malloc can't find a free block that is big enough to accomodate
 * the user's request, it extends the heap by calling extend_heap. Finding
 * an appropriate free block is done by first fit algorithm.
 * The functions add_node and delete_node are implemented to maintain 
 * the free list. The function coalesce is implemented to merge adjacent
 * free blocks.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mm.h"
#include "memlib.h"

/*
 * If NEXT_FIT defined use next fit search, else use first fit search
 */
#define NEXT_FITx

#define ALIGNMENT 8
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT - 1)) & ~0x7)
/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ //line:vm:mm:beginconst
#define DSIZE       8       /* Doubleword size (bytes) */
#define CHUNKSIZE  (1<<8)  /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) //line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            //line:vm:mm:get
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    //line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   //line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1)                    //line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
/* $end mallocmacros */

/*macros written by me*/
#define NEXT_FREE(bp) (*(void **)(bp))
#define PREV_FREE(bp) (*(void **)((void *)(bp) + DSIZE))
#define SET_NEXT(bp, val) (NEXT_FREE(bp) = val)
#define SET_PREV(bp, val) (PREV_FREE(bp) = val) 

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */
static char *fl_0 = NULL;
static char *fl_1 = NULL;
static char *fl_2 = NULL;
static char *fl_3 = NULL;
static char *fl_4 = NULL;    
static char *fl_5 = NULL;
static char *fl_6 = NULL;
static char *fl_7 = NULL;
static char *fl_8 = NULL;
static char *fl_9 = NULL;          
static char *fl_10 = NULL;
/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void add_node(void *bp);
static void delete_node(void *bp);
static void checkblock(void *bp, int lineno);
static void check_free(int heap_free, int lineno);
static char **get_list(size_t size);
static char **next_list(char **list);
/*
 * mm_init - Initialize the memory manager
 */
/* $begin mminit */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) //line:vm:mm:begininit
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);                     //line:vm:mm:endinit

    //free lists should be initially null
    fl_0 = NULL;
    fl_1 = NULL;
    fl_2 = NULL;
    fl_3 = NULL;
    fl_4 = NULL;
    fl_5 = NULL;
    fl_6 = NULL;
    fl_7 = NULL;
    fl_8 = NULL;
    fl_9 = NULL;
    fl_10 = NULL;

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    if (heap_listp == 0){
        mm_init();
    }

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    //3*DSIZE is the minimum block size. 2 DSIZE for next_free and prev_free
    //pointers, and 1 WSIZE each for a header and a footer
    if (size <= DSIZE)
            asize = 3*DSIZE;
    else
            asize = ALIGN(size +DSIZE);
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) 
    {  
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);                 
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
            return NULL;                                  
    place(bp, asize);                                 
    return bp;
}
/* $end mmmalloc */

/*
 * mm_free - Free a block
 */
void mm_free(void *bp)
{
    //not a workable request
    if(bp == 0)
        return;

    if (heap_listp == 0){
        mm_init();
    }

    //get the size of the block to set its header and footer's
    //allocated bit to zero
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    //adding to the free list is done in coalesce
    coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    //no free blocks in the front or back
    if (prev_alloc && next_alloc) {            /* Case 1 */
        add_node(bp);
        return bp;
    }
    //free block in the back. merge current block with next one
    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
        
        add_node(bp);
    }
    //previous block is free. merge previous block with current one
    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        delete_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        
        add_node(bp);
    }
    //both previous block and next block are free. merge all three
    else {                                     /* Case 4 */
        delete_node(PREV_BLKP(bp));
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(HDRP(NEXT_BLKP(bp)));
        
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        add_node(bp);
    }
    return bp;
}
void *mm_calloc (size_t nmemb, size_t size)
{
    size_t bytes = nmemb * size;
    void *newptr;
    newptr = mm_malloc(bytes);
    memset(newptr, 0, bytes);
    return newptr;
}
/*
 * mm_realloc - Naive implementation of realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

//given size, returns the address of the appropriate list
static char **get_list(size_t size)
{
    if(size <= (1 << 4) + 8) return &fl_0;
    if(size <= (1 << 5) + 8) return &fl_1;
    if(size <= (1 << 6) + 8) return &fl_2;
    if(size <= (1 << 7) + 8) return &fl_3;
    if(size <= (1 << 8) + 8) return &fl_4;
    if(size <= (1 << 9) + 8) return &fl_5;
    if(size <= (1 << 10) + 8) return &fl_6;
    if(size <= (1 << 11) + 8) return &fl_7;
    if(size <= (1 << 12) + 8) return &fl_8;
    if(size <= (1 << 13) + 8) return &fl_9;
    else return &fl_10;
}
/*
 * The remaining routines are internal helper routines
 */
//the structure of the list maintenance is LIFO.
static void add_node(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    char **free_list = get_list(size);
    //the list is none empty.
    //put bp in front of the list
    if(*free_list != NULL)
    {
        SET_PREV(*free_list, bp);
        SET_NEXT(bp, *free_list);
        SET_PREV(bp, NULL);
        *free_list = bp;
    }
    //the list is empty. bp is the only element
    else
    {
        SET_PREV(bp, NULL);
        SET_NEXT(bp, NULL);
        *free_list = bp;
    }
}

static void delete_node(void *bp)
{
    void *prev_free = PREV_FREE(bp);
    void *next_free = NEXT_FREE(bp);
    size_t size = GET_SIZE(HDRP(bp));
    char **free_list = get_list(size);
    //deleting the only element in the free list
    if(prev_free == NULL && next_free == NULL)
    {
        *free_list = NULL;
    }
    //deleting the last element of the list
    else if(prev_free != NULL && next_free == NULL)
    {
        SET_NEXT(prev_free, NULL);
    }
    //there is more than one element in the list
    //deleting the first element
    else if(prev_free == NULL && next_free != NULL)
    {
        SET_PREV(next_free, NULL);
        *free_list = next_free;
    }
    //deleting an element that has both none null
    //prev_free and next_free
    else
    {
        SET_PREV(next_free, prev_free);
        SET_NEXT(prev_free, next_free);
    }
}
/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words)
{
    void *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; 
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;                                        

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ 
    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    //the left over block is big enough to become a seperate block
    //need to split
    if ((csize - asize) >= (3*DSIZE)) {
        
        delete_node(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        
        bp = NEXT_BLKP(bp);
        
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        add_node(bp);
    }
    //no split
    else {
        delete_node(bp);
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
//based on pointer to current list, returns the pointer
//to the next list
static char **next_list(char **list)
{
    if(list == &fl_0) return &fl_1;
    if(list == &fl_1) return &fl_2;
    if(list == &fl_2) return &fl_3;
    if(list == &fl_3) return &fl_4;
    if(list == &fl_4) return &fl_5;
    if(list == &fl_5) return &fl_6;
    if(list == &fl_6) return &fl_7;
    if(list == &fl_7) return &fl_8;
    if(list == &fl_8) return &fl_9;
    if(list == &fl_9) return &fl_10;
    else return NULL;
}
/*
 * find_fit - Find a fit for a block with asize bytes
 */
//find_fit strategy: best first 10 fit
static void *find_fit(size_t asize)
{
    void *fit = NULL;
    char **free_list;
    for(free_list = get_list(asize); free_list != NULL; free_list = next_list(free_list))
    {
        for(fit = *free_list; (fit != NULL) && GET_SIZE(HDRP(fit)) > 0; fit = NEXT_FREE(fit))
        {
            if(asize <= GET_SIZE(HDRP(fit))) return fit;
        }
    }
    return fit;
}

static int aligned(const void *p)
{
    return (size_t)ALIGN(p) == (size_t)p;
}
static int in_heap(const void *p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

//basic structure of checkheap and checkblock are from csapp.cs.cmu.edu
void mm_checkheap(int lineno) {
    char *hp = heap_listp;
    int heap_free = 0;
    
    //checking prologue correctness
    if((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
    {
           printf("prologue corrupted up at line %d\n", lineno);
           exit(1);
    }

    //traversing through every block in the heap
    for(hp = heap_listp; GET_SIZE(HDRP(hp)) > 0; hp = NEXT_BLKP(hp))
    {
        //count the number of free blocks
        if(!GET_ALLOC(HDRP(hp))) heap_free++;
        checkblock(hp, lineno);
    }

    //checking epilogue correctness
    if((GET_SIZE(HDRP(hp)) != 0) || !GET_ALLOC(HDRP(hp)))
    {
        printf("epilogue corrupted up at line %d\n", lineno);
        exit(1);
    }
    check_free(heap_free, lineno);
}

static void check_free(int heap_free, int lineno)
{
    char *check;
    char **list;
    int list_free = 0;
    //traversing through the free list
    for(list = &fl_0; list != NULL; list = next_list(list))
    {
        for(check = *list; check != NULL; check = NEXT_FREE(check))
        {
            //check if the blocks point to each other
            if(NEXT_FREE(check) != NULL && PREV_FREE(NEXT_FREE(check)) != check)
            {
                printf("free node doesn't point to each other at line %d\n"
                        , lineno);
                exit(1);
            }
            //heap boundary check
            if(!in_heap(check))
            {
                printf("this free block %p is not within heap. line: %d\n"
                        , check, lineno);
                exit(1);
            }
            checkblock(check, lineno);
            //counting free blocks in the list
            list_free++;
        }
    }
    //checking the number of free blocks
    if(list_free != heap_free)
    {
        printf("number of free blocks in heap: %d list: %d at line: %d\n"
                , heap_free, list_free, lineno);
        exit(1);
    }
}
static void checkblock(void *bp, int lineno)
{
    //check alignment
    if(!aligned(bp))
    {
        printf("%p is not double word aligned: line %d\n", bp, lineno);
        exit(1);
    }
    //check header and footer
    if(GET(HDRP(bp)) != GET(FTRP(bp)))
    {
        printf("header != footer at %p: line %d\n", bp, lineno);
        exit(1);
    }
    //check if the size is greater than or equal to the minimum size
    if(bp != heap_listp && GET_SIZE(HDRP(bp)) < 3*DSIZE)
    {
        printf("size of block %p is smaller than the minimum: line %d\n",
                bp, lineno);
        exit(1);
    }
    //check coalesce correctness
    if(GET_SIZE(HDRP(NEXT_BLKP(bp))) != 0 && !GET_ALLOC(HDRP(bp)) && 
            !GET_ALLOC(HDRP(NEXT_BLKP(bp))))
    {
        printf("coalescing didn't quite work out with %p: line %d\n"
                , bp, lineno);
        exit(1);
    }
    //check if the block is within heap boundary
    if(!in_heap(bp))
    {
        printf("%p not in heap: line %d\n", bp, lineno);
        exit(1);
    }
}


