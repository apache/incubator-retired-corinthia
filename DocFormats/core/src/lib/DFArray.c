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

#include "DFArray.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct DFArray {
    size_t retainCount;
    DFCopyFunction copyFun;
    DFFreeFunction freeFun;
    void **items;
    size_t count;
    size_t alloc;
};

DFArray *DFArrayNew(DFCopyFunction copy, DFFreeFunction free)
{
    DFArray *array = (DFArray *)calloc(1,sizeof(DFArray));
    array->retainCount = 1;
    array->copyFun = copy;
    array->freeFun = free;
    array->items = NULL;
    array->count = 0;
    array->alloc = 0;
    return array;
}

DFArray *DFArrayRetain(DFArray *array)
{
    if (array != NULL)
        array->retainCount++;
    return array;
}

void DFArrayRelease(DFArray *array)
{
    if ((array == NULL) || (--array->retainCount > 0))
        return;

    if (array->freeFun != NULL) {
        for (size_t i = 0; i < array->count; i++)
            array->freeFun(array->items[i]);
    }
    free(array->items);
    free(array);
}

size_t DFArrayCount(DFArray *array)
{
    return array->count;
}

void *DFArrayItemAt(DFArray *array, size_t index)
{
    assert(index < array->count);
    return array->items[index];
}

void DFArrayRemove(DFArray *array, size_t index)
{
    assert(index < array->count);
    memmove(&array->items[index],&array->items[index+1],(array->count-index-1)*sizeof(void *));
    array->count--;
}

void DFArrayAppend(DFArray *array, void *item)
{
    if (array->copyFun != NULL)
        item = array->copyFun(item);
    if (array->alloc == array->count) {
        if (array->alloc == 0)
            array->alloc = 1;
        else
            array->alloc *= 2;
        array->items = (void **)realloc(array->items,array->alloc*sizeof(void *));
    }
    array->items[array->count++] = item;
}

void DFArraySort(DFArray *array, void *thunk, int (*compar)(void *, const void *, const void *))
{
    DFSort(array->items,array->count,sizeof(void *),thunk,compar);
}

static void DFSortInternal(char *base, size_t nel, size_t width,
                           void *thunk, int (*compar)(void *, const void *, const void *),
                           char *work)
{
    if (nel < 2) {
        return;
    }

    size_t mid = nel/2;
    char *base1 = base;
    size_t nel1 = mid;
    char *base2 = base + nel1*width;
    size_t nel2 = nel - mid;

    DFSortInternal(base1,nel1,width,thunk,compar,work);
    DFSortInternal(base2,nel2,width,thunk,compar,work + width * nel1);

    size_t pos1 = 0;
    size_t pos2 = 0;

    for (size_t outpos = 0; outpos < nel; outpos++) {
        if ((pos2 >= nel2) || ((pos1 < nel1) && (compar(thunk,base1+pos1*width,base2+pos2*width) <= 0)))
            memcpy(work+outpos*width,base1+width*(pos1++),width);
        else
            memcpy(work+outpos*width,base2+width*(pos2++),width);
    }
    memcpy(base,work,nel*width);
}

void DFSort(void *base, size_t nel, size_t width, void *thunk, int (*compar)(void *, const void *, const void *))
{
    void *work = malloc(nel*width);
    DFSortInternal(base,nel,width,thunk,compar,work);
    free(work);
}
