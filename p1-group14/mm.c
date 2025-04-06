/*
 * mm.c
 *
 * Name: Skyler Hawkins and Tej Patel
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * Also, read the project PDF document carefully and in its entirety before beginning.
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include "mm.h"
#include "memlib.h"
/*
 * If you want to enable your debugging output and heap checker code,
 * uncomment the following line. Be sure not to have debugging enabled
 * in your final submission.
 */
// #define DEBUG
#define DEBUG

#ifdef DEBUG
/* When debugging is enabled, the underlying functions get called */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated */
#define dbg_printf(...)
#define dbg_assert(...)
#endif /* DEBUG */
/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* DRIVER */


#define ALIGNMENT 16
/* rounds up to the nearest multiple of ALIGNMENT */
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}
/*all typedefs and structs go here*/
struct header {
    size_t size;
    struct header *next_p;
    struct header *prev_p;
};
struct footer {
    size_t size;
};
struct allocHdr {
    size_t size;
};


/* GLOBAL VARS and structs here*/
typedef struct allocHdr alloc_hdr;
typedef struct header block_hdr;
typedef struct footer block_ftr;
static size_t block_hdr_size = sizeof(block_hdr);
static size_t block_ftr_size = sizeof(block_ftr);
static size_t alloc_block_hdr_size = sizeof(alloc_hdr);
size_t diffInHeap;


/* DEFINING ALL FUNCTION PROTOTYPES HERE*/
void *find_free(size_t size);
void coaleasce(block_hdr *currentBlock);
void split_block(alloc_hdr *blockToSplit, size_t requestedTotalSize);
void split_block_no_header(block_hdr *blockToSplit, size_t requestedTotalSize, size_t sizeOfPrevBlock);



/*
 * Initialize: returns false on error, true on success.
 */

//static char *heap_list_pointer; /*To first pointer in block*/
bool mm_init(void)
{
    
    // block_hdr_size = align(block_hdr_size);
    // block_ftr_size = align(block_ftr_size);
    alloc_block_hdr_size = align(alloc_block_hdr_size);

    /*In first block of heap, must allocate space for for list containing free blocks*/   
    block_hdr *heap_list_pointer = mem_sbrk(block_hdr_size + block_ftr_size); 
    heap_list_pointer->size = (block_hdr_size + block_ftr_size) | 3; // nonzero size to ensure occupied
    heap_list_pointer->next_p = heap_list_pointer;
    heap_list_pointer->prev_p = heap_list_pointer;
    block_ftr *heap_footer = (block_ftr *)((char *)heap_list_pointer + (heap_list_pointer->size & ~3) - block_ftr_size);
    heap_footer->size = heap_list_pointer->size;
    
    if (heap_list_pointer == NULL ){
        return false; //null pointer returned, error with mem_sbrk
    }
    //need to create way to pack bytes, need size and allocated flag
    //HEADER: 1 word, containing size of the payload in block
    //FOOTER: 1 word, containing size of payload in block
    /* The above definitions may change with new allocation algorithm*/
    return true;
}
/*
 * malloc
 */
void* malloc(size_t size)
{
    /* IMPLEMENT THIS */
    /*Need to allocate size equal to payload + header size */

    /*
        Note for malloc footer optimization: 
        at this point, when allocating a block, need to ensure that the size of the footer is not accounted for
            Just need a minumum size for the free blocks.., since the payload of a free block can overlap with its payload, as can its header... for a size of 32 bytes
    */
    if (size <= 0){
        printf("failing here 111");
        //printf("failing here 111");
        return NULL;
    }
    size_t alloc_size;
    //min size is block_hdr + block_ftr
    if (alloc_block_hdr_size  + align(size) < 32){
        alloc_size = 32;
    }
    else{
        /////// **Footer Optimization** Removing block ftr size from allocation, and changing to a minimum size of 32 bytes (down from 48 due to inefficient setup)
        alloc_size = align(alloc_block_hdr_size + size);
    }
    //attempting to find a free block given our algorithm(explicit list)           
    block_hdr *new_pointer = find_free(alloc_size);
    if (new_pointer == NULL){
        
        //if find_free fails, need to extend the heap by alloc_size bits
        new_pointer = mem_sbrk(alloc_size);
        if ((long)new_pointer == -1){
        //no space left in heap here, nothing can be done until free memory
            printf("failing here");
            //printf("failing here");
            return NULL;
        }
        else {
            //allocating to be size alloc size and allocater bit = 1
           // printf("Size in malloc (memsbrk'd this): %zu\n", alloc_size);
            
            alloc_hdr *new_alloc_block = (alloc_hdr *)new_pointer;
            new_alloc_block->size = alloc_size | 1;
            new_alloc_block->size = new_alloc_block->size | 2;

            /////// **Footer Optimization** Removing the footer from the allocated block
            // block_ftr *new_footer = (block_ftr *)((char *)new_alloc_block + (alloc_size & ~3) - block_ftr_size);
            // new_footer->size = new_alloc_block->size;
            return (char *)new_alloc_block + alloc_block_hdr_size;

        }
        //returns pointer + hdr_size to give new memory space
        //must typcase to byte pointer
    }
    else{
        //means new_pointer is not null, space in heap found for payload

        //printf("Size in malloc: %zu\n", new_pointer->size);
        //removing block from the free block list, linking next & prev nodes
        
       //printf("New_Pointer: %p\n", new_pointer);
        //size_t oldSize = new_pointer->size & ~3;
        new_pointer->prev_p->next_p = new_pointer->next_p;
        new_pointer->next_p->prev_p = new_pointer->prev_p;
        new_pointer -> prev_p = 0;
        new_pointer -> next_p = 0;
        
        
        alloc_hdr *new_alloc_block = (alloc_hdr *)new_pointer;
        new_alloc_block->size = new_alloc_block->size | 1; //meaning that this block is allocated
        
        /////// **Footer Optimization** Removing the footer from the allocated block
        // block_ftr *new_footer = (block_ftr *)((char *)new_alloc_block + (new_alloc_block->size & ~3) - block_ftr_size);
        // new_footer->size = new_alloc_block->size;        

        //////////////////// Getting the next block for footer optimization /////////////////////////////////
        block_hdr *nextBlock = ((block_hdr *)((char *)new_alloc_block + (new_alloc_block->size & ~3)));
        //block_ftr *nextBlockFtr = ((block_ftr *)((char *)new_alloc_block + (new_alloc_block->size & ~3) - block_ftr_size));
        size_t bitForNextBlock;        
        if ((new_alloc_block->size & ~3) > 64 + alloc_size){
            
            split_block(new_alloc_block, alloc_size);     
            bitForNextBlock = 0; // since if we split, we know the prev block is 'free'    
        }
        else{
            //Since the block was not split, we know that "next block" (aka the next block BEFORE the split occured)'s previousAllocatorBit = 0, since we freed a portion to its left
            bitForNextBlock = 1;
        }
        //bitForNextBlock = 1;

        if(mem_heap_hi() > (void *)nextBlock){
            //if there is a valid nextBlock, set its allocator bit to be the value of bitForNextBlock
            if(bitForNextBlock == 0){
                nextBlock->size = nextBlock->size & ~2; // 1111 0 1 
            }
            else {
                nextBlock->size = nextBlock->size | 2; // 0000 1 0 
            }

        }
            
    /////////////////////////////////////////////////////////////////////////// NOTE, particularly chose not to update footers allocator bits, as it is only used to get the size of the whole block

 
        return (char *)new_alloc_block + alloc_block_hdr_size;
    }
    return NULL;
}
/*
 * free
 */
void free(void* ptr)
{
    /* IMPLEMENT THIS */
    //needs to point to first byte of allocated memory, pointer given has address to payload, subtract the header size to get true value 


    block_hdr *block_pointer = (block_hdr *)((char*)ptr - alloc_block_hdr_size);
    

    if((block_pointer->size & 1) == 0){
        ;
    }
    else {

    //  printf("\nCurrentBlockFreed: %p", (void *)block_pointer);
    //  printf("\nCurrentBlockSize: %zu", block_pointer->size);

    block_pointer->size = block_pointer->size & ~1;
    
    //////////////////////////////////////////// ADDING BLOCK TO THE FREE LIST
    block_hdr *head_pointer = mem_heap_lo();
    block_pointer->next_p = head_pointer->next_p;
    block_pointer->prev_p = head_pointer;
    head_pointer->next_p = block_pointer;
    block_pointer->next_p->prev_p = block_pointer;
    //////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////SETTING THE FOOTER OF THE NEWLY FREED BLOCK
    block_ftr *new_footer = (block_ftr *)((char *)block_pointer + (block_pointer->size & ~3) - block_ftr_size);
    new_footer->size = (block_pointer->size & ~3);   
    //////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////// FOOTER OPTIMIZATION ////////////////////

    block_hdr *nextBlock = ((block_hdr *)((char *)block_pointer + (block_pointer->size & ~3)));
    //block_ftr *nextBlockFtr = ((block_ftr *)((char *)block_pointer + (block_pointer->size & ~3) - block_ftr_size));


    if(mem_heap_hi() > (void *)nextBlock){
        //if there is a valid nextBlock, set its allocator bit to be the value of bitForNextBlock

            nextBlock->size = nextBlock->size & ~2; // 1111 0 1 
   

    }

    //CALLING COALESCE//
    coaleasce(block_pointer);
    }
}
/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{
    /* IMPLEMENT THIS */
    /*if new size is less than current block size, for initial implementation
    do not need to change*/
    /*if size is greater than old size free current block and then attempt to expand        
    current block
    find free block capable of holding new size
    FOR EFFICIENCY TESTS THE METHOD DESCRIBED ABOVE WILL NOT BE SUFFICIENT
    */
    // getting blockp

    alloc_hdr *blockp = oldptr - alloc_block_hdr_size;
    
    size_t requestedTotalSize;
    //size_t requestedTotalSize= align(size + alloc_block_hdr_size + block_ftr_size);

    if ( align(alloc_block_hdr_size + size) < 32){
        requestedTotalSize = 32;
    }
    else{
        /////// **Footer Optimization** Removing block ftr size from allocation
        requestedTotalSize = align(alloc_block_hdr_size + size);
    }


    // if size = 0 implementation
    if (size == 0){
        free(oldptr);
        return NULL;
    }
    // if ptr = NULL implementation
    if (oldptr == NULL){
        return malloc(size);
    }
    
    // if reducing block size with realloc
    //simple case: if the size of the block is currently greater than the allocated, 
    if((size_t)blockp->size - (size_t)alloc_block_hdr_size >= (size_t)size){
         if(blockp->size > 64 + requestedTotalSize){
            split_block(blockp, requestedTotalSize); 
     }        
        return oldptr;   
    }
    else{
        
        //here, we are going to be reallocating to a LARGER size... several options
        /*
            To Do: 
                Cases: 
                1)the next block is ALLOCATED, therefore cannot be split into
                2)the next block is FREE, then
                    a) the next block does NOT have enough space to be split into
                    b) the next block HAS enough space to be split into
        */
        
         block_hdr *nextBlock = (((block_hdr *)((char *)blockp + (blockp->size & ~3))));

         if(mem_heap_hi() > (void *)nextBlock){
            size_t nextAllocatorBit = nextBlock->size & 1;
            if (nextAllocatorBit == 1){
                //next block being allocated, must maloc memcpy and free
                void* newPtr = malloc(size);
                memcpy(newPtr, oldptr, blockp->size - alloc_block_hdr_size );
                free(oldptr);
                return newPtr; 
            }
            else{
                //next block is free, from here must determine if splitting or not
                size_t remainingSizeNeeded = requestedTotalSize - (blockp->size & ~3);
                if(remainingSizeNeeded > (nextBlock->size & ~3)){
                    //similar to previous case, must simply find new memory address
                    void* newPtr = malloc(size);
                    memcpy(newPtr, oldptr, blockp->size - alloc_block_hdr_size);
                    free(oldptr);
                    return newPtr; 
                }
                else{
                    // here, we can split if we choose to
                    // this split is different, since we need to remove nextBlock from the free list, and not set the left portion as an allocated block

                    if ( (nextBlock->size & ~3) > 64 + remainingSizeNeeded)
                    {
                        size_t prevBlockAllocatorBit = blockp->size & 2;
                        blockp->size = requestedTotalSize | 1;
                        blockp->size = blockp->size | prevBlockAllocatorBit;
                        
                        split_block_no_header(nextBlock, remainingSizeNeeded, (blockp->size & ~3));
            
                        return oldptr;
                        void* newPtr = malloc(size);
                        memcpy(newPtr, oldptr, blockp->size - alloc_block_hdr_size);
                        free(oldptr);
                        return newPtr;
                    }
                    else{
                        void* newPtr = malloc(size);
                        memcpy(newPtr, oldptr, blockp->size - alloc_block_hdr_size);
                        free(oldptr);
                        return newPtr;
                    }


                    }
              
                }

            }
       


        void* newPtr = malloc(size);
        memcpy(newPtr, oldptr, blockp->size - alloc_block_hdr_size );
        free(oldptr);
        return newPtr;
    }

    return NULL;
}

void split_block_no_header(block_hdr *blockToSplit, size_t requestedTotalSize, size_t sizeOfPrevBlock){
    //CASE OF BLOCK SPLITTING SPECIFIC TO SECOND PART OF REALLOC *******************
    /*
        To Implement: 
            Need to ensure that the block only gets split IF
                *** There is enough memory in the block to house the two split ones ***
                    AKA into block1->size = reqeustedTotalSize, and block2->size = Original Size - requestedTotalSize (at a minimum 48 bytes will go here)
            Need to set the blockToSplit->size to the accurate size parameter (requestedTotalSize)
            Need to 'free' the second block (call free)
    */


        size_t originalSize = (blockToSplit->size & ~3);
        size_t sizeOfBlock2 = originalSize - requestedTotalSize;



        printf("removing %p from the free list \n", blockToSplit);
        printf("\nOriginal size of the block: %zu\n", originalSize);
        printf("Splitting block into sizes: %zu,  %zu\n", requestedTotalSize, sizeOfBlock2);
        ///////////////////////// Removing blockToSplit from the free list
        blockToSplit->next_p->prev_p =  blockToSplit->prev_p;  
        blockToSplit->prev_p->next_p =  blockToSplit->next_p;  
        /////////////////////////

        //////////////////////////// Setting the allocated portion of the block, none at all in Footer Optimization
        //blockToSplit->size = requestedTotalSize | 1; 
        // block_ftr *block_footer = (block_ftr *)((char *)blockToSplit + requestedTotalSize - block_ftr_size);
        // block_footer->size = sizeOfPrevBlock | 1; 
        // ///////////////////////////////////////////////////////////////////////

        //////////////////////////// Setting the free portion of the block

        //** Note, reason for setting allocator bit of free block to 1 is so free knows to free it **//
        alloc_hdr *newBlock = (((alloc_hdr *)((char *)blockToSplit + (requestedTotalSize & ~3))));
        newBlock->size = sizeOfBlock2 | 1;
        newBlock->size = newBlock->size | 2; //since it is known that the previous block is allocated

        printf("sending to free: %p \n", newBlock);
        printf("sending has size: %zu \n", newBlock->size);
        block_ftr *new_footer = (block_ftr *)((char *)newBlock + (newBlock->size & ~3) - block_ftr_size);
        new_footer->size = newBlock->size;

        free((void*)((char *)newBlock + alloc_block_hdr_size));     

        ///////////////////////////////////////////////////////////////////////
    


}





void split_block(alloc_hdr *blockToSplit, size_t requestedTotalSize){
    //Case of splitting a block where the block is shrinking...
    /*
        To Implement: 
            Need to ensure that the block only gets split IF
                *** There is enough memory in the block to house the two split ones ***
                    AKA into block1->size = reqeustedTotalSize, and block2->size = Original Size - requestedTotalSize (at a minimum 48 bytes will go here)
            Need to set the blockToSplit->size to the accurate size parameter (requestedTotalSize)
            Need to 'free' the second block (call free)
    */

    //////////////////////////////// Getting original size and alloc Bit so the information is not lost

    size_t originalSize = (blockToSplit->size & ~3);
    size_t originalPrevAllocatorBit = blockToSplit->size & 2;

    size_t sizeOfBlock2 = originalSize - requestedTotalSize;
    // printf("\nOriginal size of the block: %zu\n", originalSize);
    // printf("Splitting block into sizes: %zu,  %zu\n", requestedTotalSize, sizeOfBlock2);

    if (sizeOfBlock2 >= 48){
        //  Checked size, will now begin the block split
        //////////////////////////// Setting the allocated portion of the block to have correct headers and allocation bits
        blockToSplit->size = requestedTotalSize | 1;
        blockToSplit->size = blockToSplit->size | originalPrevAllocatorBit; 

        ////////////////////////////////////////////// **Footer Optimization ** Do not need footer
        // block_ftr *block_footer = (block_ftr *)((char *)blockToSplit + (blockToSplit->size & ~3) - block_ftr_size);
        // block_footer->size = blockToSplit->size; 
        ///////////////////////////////////////////////////////////////////////

        //////////////////////////// Setting the free portion of the block

        //** Note, reason for setting allocator bit of free block to 1 is so free knows to free it **//
        alloc_hdr *newBlock = (((alloc_hdr *)((char *)blockToSplit + requestedTotalSize)));
        newBlock->size = sizeOfBlock2 | 1;
        newBlock->size = newBlock->size | 2;

        block_ftr *new_footer = (block_ftr *)((char *)newBlock + (newBlock->size & ~3) - block_ftr_size);
        new_footer->size = (newBlock->size & ~3);

        free((void*)((char *)newBlock + alloc_block_hdr_size));     

        ///////////////////////////////////////////////////////////////////////
    }


}


void coaleasce(block_hdr *currentBlock){
    /*
        General Approach of Coalescing
        Need to inspect both the next and previous blocks in order to see if they are free
            IF they are free, they are combined with the currentBlock into one large contiguous block of memory
            4 Main cases:
                1)  Allocated   Free     Allocated 
                2)  Allocated   Free     Free
                3)  Free        Free     Allocated
                4)  Free        Free     Free
    */
    
    ///////////////////////FOOTER OPTIMIZATION** No grabbing footer preemtpively
    //block_ftr *previousBlockFooter = ((block_ftr *)((char *)currentBlock - block_ftr_size));
    //size_t previousAllocatorBit = previousBlockFooter->size & 1;
    size_t previousAllocatorBit = currentBlock->size & 2;
    block_hdr *nextBlock = ((block_hdr *)((char *)currentBlock + (currentBlock->size & ~3)));
    size_t nextAllocatorBit = nextBlock->size & 1;

    if(mem_heap_hi() <= (void *)nextBlock){
        //If the next block isnt in the heap, treated it as 'allocated' from a coalescing perspective... meaning do not coalesce
        nextAllocatorBit = 1;
    }
    

    if(previousAllocatorBit == 2 && nextAllocatorBit == 1){
        ; 
    }
    else if(previousAllocatorBit == 2 && nextAllocatorBit == 0){
        //Forward coalescing here, want to merge next block and current one
        /*521
            To Implement: 
                remove nextBlock from the free list
                update size element of currentBlock to include itself AND nextBlock
                update the footer of currentBlock to reflect actual size
        */

        /////////////////////////////////// Removing From Free List 
        nextBlock->next_p->prev_p = nextBlock->prev_p;
        nextBlock->prev_p->next_p = nextBlock->next_p;
        ////////////////////////////////////

        ///////////////////////////////////////// Updating the size of currentBlocks header and footer
        currentBlock->size = (((currentBlock->size & ~3) + (nextBlock->size & ~3)) & ~1);
        currentBlock->size = currentBlock->size & ~1;
        currentBlock->size = currentBlock->size | 2;
        block_ftr *newCurrentFooter = ((block_ftr *)((char *)currentBlock + (currentBlock->size & ~3) - block_ftr_size));
        newCurrentFooter->size = currentBlock->size;
    }
    else if(previousAllocatorBit == 0 && nextAllocatorBit == 1){
        //Backward coalescing here, want to merge previous block and current one
        /*
            To Implement: 
                remove currentBlock from the free list
                update size element of previousBlock to include itself AND currentBlock
                update the footer of previousBlock to reflect actual size
        */     
        //////////////////////////////////// Establishing previousBlock
        


        block_ftr *previousBlockFooter = ((block_ftr *)((char *)currentBlock - block_ftr_size));
        block_hdr *previousBlock = (((block_hdr *)((char*)currentBlock - (previousBlockFooter->size & ~3))));
        
        size_t tempAllocatorBit = previousBlock->size & 2; //to maintain knowledge of the allocator bit for previousBlocks previous block 




        // printf("previous and current block: %p  %p \n",(void *)previousBlock, (void *)currentBlock);
        /////////////////////////////////// Removing From Free List 
        currentBlock->next_p->prev_p = currentBlock->prev_p;
        currentBlock->prev_p->next_p = currentBlock->next_p;
        ////////////////////////////////////
        previousBlock->size = (((currentBlock->size & ~3) + (previousBlock->size & ~3)) & ~1);
        previousBlock->size = previousBlock->size & ~1;
        previousBlock->size = previousBlock->size | tempAllocatorBit;
        block_ftr *newPreviousFooter = ((block_ftr *)((char *)previousBlock + (previousBlock->size & ~3) - block_ftr_size));
        newPreviousFooter->size = previousBlock->size;

            // if(mem_heap_hi() <= (void *)nextBlock){
            // //If the next block isnt in the heap, treated it as 'allocated' from a coalescing perspective... meaning do not coalesce
            //     newBlock->size = newBlock->size & ~2;
            // }

    }
    else if(previousAllocatorBit == 0 && nextAllocatorBit == 0){
        //Bidirectional coalescing want to merge all 3
        /*
            To implement: 
                want previousBlock to hold the size of previousBlock + currentBlock + nextBlock
                want to free both currentBlock and nextBlock
                want to update footer of newPreviousBlock
        */
        block_ftr *previousBlockFooter = ((block_ftr *)((char *)currentBlock - block_ftr_size));

        block_hdr *previousBlock = (((block_hdr *)((char*)currentBlock - (previousBlockFooter->size & ~3))));

        //////////////////////////////////// Removing current and next from the free list
        nextBlock->next_p->prev_p = nextBlock->prev_p;
        nextBlock->prev_p->next_p = nextBlock->next_p;   
    
        currentBlock->next_p->prev_p = currentBlock->prev_p;
        currentBlock->prev_p->next_p = currentBlock->next_p;
        ////////////////////////////////////    
        size_t tempAllocatorBit = previousBlock->size & 2; //to maintain knowledge of the allocator bit for previousBlocks previous block 
        previousBlock->size = (((currentBlock->size & ~3) + (previousBlock->size & ~3) + (nextBlock->size & ~3)) & ~ 1);
        block_ftr *newPreviousFooter = ((block_ftr *)((char *)previousBlock + (previousBlock->size & ~3) - block_ftr_size));
        newPreviousFooter->size = previousBlock->size;


        previousBlock->size = previousBlock->size & ~1; //to set own allocator bit to 0
        previousBlock->size = previousBlock->size | tempAllocatorBit;

        // mm_checkheap(1);
        // if (diffInHeap != 1){
        //     printf("\n\n\n\n\n\n\n\n\n\nWHAT THE HECK IS GOIN ON HERE:  %zu????\n\n\n\n\n\n\n", diffInHeap);

        // }

        //////////Lastly, need to s


    }

}




/*
 * calloc
 * This function is not tested by mdriver, and has been implemented for you.
 */
void* calloc(size_t nmemb, size_t size)
{
    void* ptr;
    size *= nmemb;
    ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}
/*
 * Returns whether the pointer is in the heap.
 * May be useful for debugging.
 */
static bool in_heap(const void* p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}
/*
 * Returns whether the pointer is aligned.
 * May be useful for debugging.
 */
static bool aligned(const void* p)
{
    size_t ip = (size_t) p;
    return align(ip) == ip;
}
/*
Find_Free finds a free block for the payload given using 
the explicit list algorithm
*/
void *find_free(size_t size){
    //pointer to return for finding a free
    block_hdr *p;
    
    //as long as the size of the free block is less than the size
    //of the requested block, keep looking
    for (p = ((block_hdr *)mem_heap_lo())->next_p;
        p != mem_heap_lo() && p->size < size;
        p = p->next_p);
    //print(p, size of block, size );
    if (p!=mem_heap_lo()) //meaning as long as block is found (cannot use first block)
    {

        return p;
    }   
    else
   { return NULL;}
}
/*
 * mm_checkheap
 */
bool mm_checkheap(int lineno)
{
#ifdef DEBUG
    /* Write code to check heap invariants here */
    /* IMPLEMENT THIS */
    //prints every free block in the heap when called, not using lineno right now
    //not needed to check contiguous free blocks as coalescing not implemented yet

    size_t numFreeInHeap = 0;
    size_t numFreeInFreeList = 0;
    // prints block by block, from start to end, irrelavant of it being free or allocated and shows following data about the same

    block_hdr *block_pointer = mem_heap_lo();
    printf("************BEGINNING HEAP CHECKER********");
    while (block_pointer < (block_hdr* )mem_heap_hi()){
        // prints allocated if 1, free if 0. rest is understood...
       printf("%s block at %p, size %zu\n", block_pointer->size&1?"allocated":"free", block_pointer, block_pointer->size & ~1);
        if((block_pointer->size & 1) == 0){
            numFreeInHeap++;
        }
        block_pointer = (block_hdr *)((char *)block_pointer + (block_pointer->size & ~3));

    }

   printf("\n\n\n\n\n");


    // Below code does the following
    // Is every block in the free list marked as free?
    // memheaplo.next all the way to the end should be free, 0 (free) for 'allocated bit' for all, error if '1' found

    block_pointer = mem_heap_lo();
    block_pointer = block_pointer->next_p;

    while (block_pointer != mem_heap_lo()){
        // exaggerates error if allocated if 1, free if 0. rest is understood...

       printf("%s block at %p, size %zu\n", block_pointer->size&1?"\nERROR ERROR ERROR, ALLOCATED WHEN SUPPOSED TO BE FREE\n\n\n\n":"free", block_pointer, block_pointer->size & ~1);
        block_pointer = block_pointer->next_p;        
        if((block_pointer->size & 1) == 0){
            numFreeInFreeList++;
        }        
    }
    diffInHeap = numFreeInHeap - numFreeInFreeList;
   printf(" *******DIFFERENCE IN NUMBER IN HEAP VS FREE LIST: %zu \n\n", (numFreeInHeap - numFreeInFreeList) - 1);
   printf("\n\n\n\n\n\n");





    // Below code does the following
    // 
    // Are there any contiguous free blocks that somehow escaped coalescing?
    // if (blockp + blockp.size) block marked as free and found in freelist, it escaped coalescing, so need to fix
    // 
    // 
    // Is every free block actually in the free list?
    // go from memheaplo to every next block by incrementing pointer by size, and checking if that pointer is 
    // marked free (allocation bit), itterate through the whole free list to see if we find that pointer in it, 
    // if not found, we missed adding it into the free list and needs to be fixed
    // 
    // Do the pointers in the free list point to valid free blocks?
    // at least size align(header), allocation bit 0, 
    
    // block_pointer = mem_heap_lo();

    // bool found1 = false;
    // bool found2 = false;

    // bool foundalone = false;

    
    // // going through all blocks
    // while ((void *)block_pointer < mem_heap_hi()){
        
    //     // if current block is marked free and next block is marked as free, and is within the heap (lesser than mem_heap_hi), it is a free block in bounds
    //     if ( ((block_pointer->size & 1) == 0) && (( (void *)block_pointer + (block_pointer->size & ~1))->size & 1) == 0 && (( (void *)block_pointer + (block_pointer->size & ~1)) < (void* )mem_heap_hi())){
                
    //             printf("adjacent free blocks found, need to do coalescing correctly\n");
    //             block_hdr *block_pointer2 = mem_heap_lo();

    //         // check free list if both blocks are in it 
    //         while (block_pointer2 < (block_hdr* )mem_heap_hi()){
                
    //             // block 1 found in free list
    //             if(block_pointer == block_pointer2){
    //                 printf("block 1 in free list and found at %p, size %zu ", block_pointer, block_pointer->size & ~1);
    //                 found1 = true;

    //                 if(block_pointer->size >= block_hdr_size){
    //                     printf("AND is a Valid free block\n");
    //                 }
    //                 else{
    //                     printf("\n\n\n\n\n\n\n\nINVALID FREE BLOCK\nINVALID FREE BLOCK\nINVALID FREE BLOCK\nINVALID FREE BLOCK\n\n\n\n\n\n\n\n\n");

    //                 }

    //             }

    //             // block 2 found in free list
    //             if((( block_hdr * ) block_pointer + (block_pointer->size & ~1)) == block_pointer2){
    //                 printf("block 2 in free list and found at %p, size %zu ", ( block_hdr * ) block_pointer + (block_pointer->size & ~1), (block_pointer + ((block_pointer->size) & ~1))->size & ~1);
    //                 found2 = true;

    //                 if((( block_hdr * ) block_pointer + (block_pointer->size & ~1))->size >= block_hdr_size){
    //                     printf("AND is a Valid free block\n");
    //                 }
    //                 else{
    //                     printf("\n\n\n\n\n\n\n\nINVALID FREE BLOCK\nINVALID FREE BLOCK\nINVALID FREE BLOCK\nINVALID FREE BLOCK\n\n\n\n\n\n\n\n\n");

    //                 }

    //             }

    //             block_pointer2 = block_pointer2->next_p;

    //         }
            
    //         if(found1 == false){
    //             printf("free block 1 not found in free list");
    //         }

    //         if(found2 == false){
    //             printf("free block 2 not found in free list");
    //         }

    //     }
    //     // check if all free blocks found in full traversal are a part of free list
    //     else if(((block_pointer->size & 1) == 0)){

    //         printf("stand alone free block found");
    //         block_hdr *block_pointer2 = mem_heap_lo();

    //         // checking through the free list
    //         while (block_pointer2 < (block_hdr* )mem_heap_hi()){
                
    //             // block found in free list
    //             if(block_pointer == block_pointer2){
    //                 printf("stand alone free block in free list and found at %p, size %zu \n", block_pointer, block_pointer->size & ~1);
    //                 foundalone = true;
    //             }
                
    //             block_pointer2 = block_pointer2->next_p;

    //         }

    //         if(foundalone == false){
    //             printf("stand alone free block not found in free list");
    //         }

    //     }

    //     printf("%s block at %p, size %zu\n", block_pointer->size&1?"ALLOCATED BLOCK":"\n\n\n\n\n\n\n\n\n\n\n\nSHOULD NEVER BE HERE\nSHOULD NEVER BE HERE\nSHOULD NEVER BE HERE\nSHOULD NEVER BE HERE\nSHOULD NEVER BE HERE\nSHOULD NEVER BE HERE\nSHOULD NEVER BE HERE\n\n\n\n\n\n\n\n\n\n\n", block_pointer, block_pointer->size & ~1);
    //     block_pointer = (block_pointer + (block_pointer->size & ~1));
    // }






 #endif /* DEBUG */
     return true;




//     /*
//     Is every block in the free list marked as free?
//     memheaplo.next all the way to the end should be free, 0 for allocated bit for all, error if 1 found
    

//     Are there any contiguous free blocks that somehow escaped coalescing?
//     if blockp + blockp.size = blockp.next, it escaped coalescing, so need to fix
    

//     Is every free block actually in the free list?
//     go from memheaplo to every next block by incrementing pointer by size, and checking if that pointer is 
//     marked free (allocation bit), itterate through the whole free list to see if we find that pointer in it, 
//     if not found, we missed adding it into the free list and needs to be fixed
    

//     Do the pointers in the free list point to valid free blocks?
//     at least size align(header), allocation bit 0, 


//     Do any allocated blocks overlap?

    
    
 //   Do the pointers in a heap block point to valid heap addresses*/
}