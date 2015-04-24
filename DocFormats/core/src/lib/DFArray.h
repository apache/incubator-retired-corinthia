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
