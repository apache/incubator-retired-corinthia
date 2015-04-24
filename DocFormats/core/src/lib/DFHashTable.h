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

#ifndef DocFormats_DFHashTable_h
#define DocFormats_DFHashTable_h

#include "DFTypes.h"

// http://en.wikipedia.org/wiki/Jenkins_hash_function

#define DFHashBegin(__hc) { (__hc) = 0; }

#define DFHashUpdate(__hc,__value) { \
    (__hc) += (__value);              \
    (__hc) += ((__hc) << 10);         \
    (__hc) ^= ((__hc) >> 6);          \
}

#define DFHashEnd(__hc) {            \
  (__hc) += ((__hc) << 3);            \
  (__hc) ^= ((__hc) >> 11);           \
  (__hc) += ((__hc) << 15);           \
}

typedef uint32_t DFHashCode;
typedef struct DFHashTable DFHashTable;
typedef void* (*DFCopyFunction)(const void *object);
typedef void (*DFFreeFunction)(void *object);

DFHashTable *DFHashTableNew(DFCopyFunction copy, DFFreeFunction free);
DFHashTable *DFHashTableNew2(DFCopyFunction copy, DFFreeFunction free, int binsCount);
DFHashTable *DFHashTableRetain(DFHashTable *table);
void DFHashTableRelease(DFHashTable *table);

DFHashTable *DFHashTableCopy(DFHashTable *src);
int DFHashTableCount(DFHashTable *table);
const char **DFHashTableCopyKeys(DFHashTable *table);
void *DFHashTableLookup(DFHashTable *table, const char *key);
void DFHashTableAdd(DFHashTable *table, const char *key, const void *value);
void DFHashTableRemove(DFHashTable *table, const char *key);

void *DFHashTableLookupInt(DFHashTable *table, int key);
void DFHashTableAddInt(DFHashTable *table, int key, void *value);
void DFHashTableRemoveInt(DFHashTable *table, int key);

#endif
