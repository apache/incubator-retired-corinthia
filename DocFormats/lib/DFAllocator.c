//
//  DFAllocator.c
//  DocFormats
//
//  Created by Peter Kelly on 2/03/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "DFAllocator.h"
#include "DFCommon.h"

typedef struct DFAllocatorBlock DFAllocatorBlock;

struct DFAllocatorBlock {
    DFAllocatorBlock *next;
    size_t used;
    size_t size;
    char mem[0] __attribute__ ((aligned (8)));
};

struct DFAllocator {
    DFAllocatorBlock *blocks;
    unsigned int blockCount;
};

DFAllocator *DFAllocatorNew(void)
{
    size_t initialSize = 1;
    DFAllocator *alc = (DFAllocator *)malloc(sizeof(DFAllocator));
    alc->blocks = (DFAllocatorBlock *)malloc(sizeof(DFAllocatorBlock)+initialSize);
    alc->blocks->next = NULL;
    alc->blocks->used = 0;
    alc->blocks->size = initialSize;
    alc->blockCount = 1;
    return alc;
}

void DFAllocatorFree(DFAllocator *alc)
{
    while (alc->blocks != NULL) {
        DFAllocatorBlock *next = alc->blocks->next;
        free(alc->blocks);
        alc->blocks = next;
    }
    free(alc);
}

void *DFAllocatorAlloc(DFAllocator *alc, size_t size)
{
    size_t remainder = size % 8;
    if (remainder != 0)
        size += (8 - remainder);
    DFAllocatorBlock *block = alc->blocks;
    if (size > block->size - block->used) {
        size_t newSize = block->size*2;
        while (size > newSize)
            newSize *= 2;
        block = (DFAllocatorBlock *)malloc(sizeof(DFAllocatorBlock)+newSize);
        block->used = 0;
        block->size = newSize;
        block->next = alc->blocks;
        alc->blocks = block;
        alc->blockCount++;
    }
    char *mem = block->mem + block->used;
    block->used += size;
    assert(block->used <= block->size);
    assert((((unsigned long long)mem) % 8) == 0);
    return mem;
}
