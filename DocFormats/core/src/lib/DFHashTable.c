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

#include "DFPlatform.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DFHashEntry DFHashEntry;

struct DFHashEntry {
    DFHashEntry *next;
    void *value;
    DFHashCode hash;
    char key[0];
};

struct DFHashTable {
    size_t retainCount;
    size_t binsCount;
    DFHashEntry **bins;
    DFCopyFunction copy;
    DFFreeFunction free;
};

DFHashTable *DFHashTableNew(DFCopyFunction copy, DFFreeFunction free)
{
    return DFHashTableNew2(copy,free,53);
}

DFHashTable *DFHashTableNew2(DFCopyFunction copy, DFFreeFunction free, int binsCount)
{
    DFHashTable *table = (DFHashTable *)calloc(1,sizeof(DFHashTable));
    table->retainCount = 1;
    table->binsCount = binsCount;
    table->bins = (DFHashEntry **)calloc(1,table->binsCount*sizeof(DFHashEntry *));
    table->copy = copy;
    table->free = free;
    return table;
}

DFHashTable *DFHashTableRetain(DFHashTable *table)
{
    if (table != NULL)
        table->retainCount++;
    return table;
}

void DFHashTableRelease(DFHashTable *table)
{
    if (table == NULL)
        return;

    table->retainCount--;
    if (table->retainCount > 0)
        return;

    for (DFHashCode bin = 0; bin < table->binsCount; bin++) {
        DFHashEntry *entry = table->bins[bin];
        while (entry != NULL) {
            DFHashEntry *next = entry->next;
            if (table->free != NULL)
                table->free(entry->value);
            free(entry);
            entry = next;
        }
    }
    free(table->bins);
    free(table);
}

DFHashTable *DFHashTableCopy(DFHashTable *src)
{
    DFHashTable *result = DFHashTableNew(src->copy,src->free);
    for (DFHashCode bin = 0; bin < src->binsCount; bin++) {
        for (DFHashEntry *entry = src->bins[bin]; entry != NULL; entry = entry->next)
            DFHashTableAdd(result,entry->key,entry->value);
    }
    return result;
}

int DFHashTableCount(DFHashTable *table)
{
    int count = 0;
    for (DFHashCode bin = 0; bin < table->binsCount; bin++) {
        for (DFHashEntry *entry = table->bins[bin]; entry != NULL; entry = entry->next)
            count++;
    }
    return count;
}

const char **DFHashTableCopyKeys(DFHashTable *table)
{
    // This function returns a block of memory divided into two parts. The first part contains an array of string
    // pointers (terminated by a NULL pointer), while the second part contains the actual string data. All pointers
    // in the first part of the memory block refer to locations in the second part of the memory block.
    //
    // By placing all pointers and values in a single block of memory, the caller has to make only one call to
    // free after using this function; they do not (and cannot) free the individual string pointers. Thus a typical
    // usage of the function is as follows:
    //
    // const char **keys = DFHashTableCopyKeys(table);
    // for (int i = 0; keys[i]; i++) {
    //     ... do stuff with the current key ...
    // }
    // free(keys);
    //
    // Because the returned array is a const char ** (i.e. an array pointing to char * values that cannot be changed),
    // the compiler will issue a warning if an attempt is made to free individual keys. Ignoring this warning will
    // result in a crash when calling free.

    int count = DFHashTableCount(table);
    size_t numBytes = (count+1)*sizeof(char *);
    for (DFHashCode bin = 0; bin < table->binsCount; bin++) {
        for (DFHashEntry *entry = table->bins[bin]; entry != NULL; entry = entry->next)
            numBytes += strlen(entry->key)+1;
    }

    void *mem = malloc(numBytes);
    char **pointers = mem;
    char *storage = (char *)mem + (count+1)*sizeof(char *);

    size_t index = 0;
    for (DFHashCode bin = 0; bin < table->binsCount; bin++) {
        for (DFHashEntry *entry = table->bins[bin]; entry != NULL; entry = entry->next) {
            pointers[index++] = storage;
            size_t len = strlen(entry->key);
            memcpy(storage,entry->key,len);
            storage += len;
            *storage = '\0';
            storage += 1;
        }
    }
    assert(index == count);
    assert(storage == (char *)mem + numBytes);
    pointers[index++] = NULL;
    return (const char **)pointers;
}

static DFHashEntry **DFHashTableLookupEntry(DFHashTable *table, const char *key, DFHashCode hash)
{
    DFHashCode bin = hash % table->binsCount;
    for (DFHashEntry **ptr = &table->bins[bin]; *ptr != NULL; ptr = &(*ptr)->next) {
        DFHashEntry *entry = *ptr;
        if (entry->hash != hash)
            continue;
        if (strcmp(entry->key,key))
            continue;
        return ptr;
    }
    return NULL;
}

static DFHashCode DFHashString(const char *str)
{
    DFHashCode hash = 0;
    DFHashBegin(hash);
    while (*str != '\0') {
        DFHashUpdate(hash,*str);
        str++;
    }
    DFHashEnd(hash);
    return hash;
}

void *DFHashTableLookup(DFHashTable *table, const char *key)
{
    DFHashCode hash = DFHashString(key);
    DFHashEntry **ptr = DFHashTableLookupEntry(table,key,hash);
    if (ptr != NULL)
        return (*ptr)->value;
    else
        return NULL;
}

void DFHashTableAdd(DFHashTable *table, const char *key, const void *constValue)
{
    void *value = (void *)constValue;
    if (table->copy != NULL) {
        value = table->copy(value);
    }

    DFHashCode hash = DFHashString(key);
    DFHashEntry **ptr = DFHashTableLookupEntry(table,key,hash);
    if (ptr != NULL) {
        DFHashEntry *entry = *ptr;
        void *oldValue = entry->value;
        entry->value = value;
        if (table->free != NULL)
            table->free(oldValue);
    }
    else {
        size_t len = strlen(key);
        DFHashEntry *entry = (DFHashEntry *)malloc(sizeof(DFHashEntry)+len*sizeof(char)+1);
        entry->value = value;
        entry->hash = hash;
        memcpy(&entry->key[0],key,len);
        entry->key[len] = '\0';

        DFHashCode bin = hash % table->binsCount;
        entry->next = table->bins[bin];
        table->bins[bin] = entry;
    }
}

void DFHashTableRemove(DFHashTable *table, const char *key)
{
    DFHashCode hash = DFHashString(key);
    DFHashEntry **ptr = DFHashTableLookupEntry(table,key,hash);
    if (ptr != NULL) {
        DFHashEntry *entry = *ptr;
        *ptr = entry->next;
        if (table->free)
            table->free(entry->value);
        free(entry);
    }
}

void *DFHashTableLookupInt(DFHashTable *table, int key)
{
    char strkey[40];
    snprintf(strkey,40,"%d",key);
    return DFHashTableLookup(table,strkey);
}

void DFHashTableAddInt(DFHashTable *table, int key, void *value)
{
    char strkey[40];
    snprintf(strkey,40,"%d",key);
    DFHashTableAdd(table,strkey,value);
}

void DFHashTableRemoveInt(DFHashTable *table, int key)
{
    char strkey[40];
    snprintf(strkey,40,"%d",key);
    DFHashTableRemove(table,strkey);
}
