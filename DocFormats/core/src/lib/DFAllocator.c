// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "DFAllocator.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>

typedef struct DFAllocatorBlock DFAllocatorBlock;

struct DFAllocatorBlock {
    DFAllocatorBlock *next;
    size_t used;
    size_t size;
    char ATTRIBUTE_ALIGNED(8) mem[0];
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
    struct DFAllocatorBlock *block = alc->blocks;
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
