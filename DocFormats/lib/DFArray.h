//
//  DFArray.h
//  DocFormats
//
//  Created by Peter Kelly on 29/12/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_DFArray_h
#define DocFormats_DFArray_h

#include "DFHashTable.h"
#include "DFTypes.h"

typedef struct DFArray DFArray;

DFArray *DFArrayNew(DFCopyFunction copy, DFFreeFunction free);
DFArray *DFArrayRetain(DFArray *array);
void DFArrayRelease(DFArray *array);

size_t DFArrayCount(DFArray *array);
void *DFArrayItemAt(DFArray *array, size_t index);
void DFArrayRemove(DFArray *array, size_t index);
void DFArrayAppend(DFArray *array, void *item);
void DFArraySort(DFArray *array, void *thunk, int (*compar)(void *, const void *, const void *));

void DFSort(void *base, size_t nel, size_t width, void *thunk, int (*compar)(void *, const void *, const void *));

#endif
