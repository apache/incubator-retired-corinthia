//
//  DFHashTable.h
//  DocFormats
//
//  Created by Peter Kelly on 28/02/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFHashTable_h
#define DocFormats_DFHashTable_h

#include "DFTypes.h"

// http://en.wikipedia.org/wiki/Jenkins_hash_function

#define DFHashBegin(__hc) ({ (__hc) = 0; })

#define DFHashUpdate(__hc,__value) ({ \
    (__hc) += (__value);              \
    (__hc) += ((__hc) << 10);         \
    (__hc) ^= ((__hc) >> 6);          \
})

#define DFHashEnd(__hc) ({            \
  (__hc) += ((__hc) << 3);            \
  (__hc) ^= ((__hc) >> 11);           \
  (__hc) += ((__hc) << 15);           \
})

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
