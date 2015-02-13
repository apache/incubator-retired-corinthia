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
#include "DFNameMap.h"
#include "DFDOM.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include "DFPlatform.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void NameMap_staticInit();

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         DFNameHashTable                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define HASH_TABLE_SIZE 983

typedef struct DFNameEntry {
    char *name;
    char *URI;
    Tag tag;
    unsigned int namespaceID;
    TagDecl tagDecl;
    struct DFNameEntry *next;
} DFNameEntry;

typedef struct DFNameHashTable {
    DFNameEntry *bins[HASH_TABLE_SIZE];
} DFNameHashTable;

static uint32_t DFNameHashTableHash(const char *name, const char *URI)
{
    DFHashCode hash;
    DFHashBegin(hash);
    for(DFHashCode i = 0; name[i]; ++i)
        DFHashUpdate(hash,name[i]);
    for(DFHashCode i = 0; URI[i]; ++i)
        DFHashUpdate(hash,URI[i]);
    DFHashEnd(hash);

    return hash;
}

static const DFNameEntry *DFNameHashTableGet(DFNameHashTable *table, const char *name, const char *URI)
{
    if (URI == NULL)
        URI = "";;
    uint32_t hash = DFNameHashTableHash(name,URI)%HASH_TABLE_SIZE;
    for (DFNameEntry *entry = table->bins[hash]; entry != NULL; entry = entry->next) {
        if (!strcmp(name,entry->name) && !strcmp(URI,entry->URI))
            return entry;
    }
    return 0;
}

static void DFNameHashTableAdd(DFNameHashTable *table, const char *name, const char *URI,
                               Tag tag, unsigned int namespaceID)
{
    if (URI == NULL)
        URI = "";;
    uint32_t hash = DFNameHashTableHash(name,URI)%HASH_TABLE_SIZE;
    DFNameEntry *entry = (DFNameEntry *)malloc(sizeof(DFNameEntry));
    entry->name = strdup(name);
    entry->URI = strdup(URI);
    entry->tag = tag;
    entry->tagDecl.namespaceID = namespaceID;
    entry->tagDecl.localName = (const char *)entry->name;
    entry->next = table->bins[hash];
    table->bins[hash] = entry;
}

static DFNameHashTable *DFNameHashTableNew()
{
    return (DFNameHashTable*)calloc(1,sizeof(DFNameHashTable));
}

static void DFNameHashTableFree(DFNameHashTable *table)
{
    for (uint32_t hash = 0; hash < HASH_TABLE_SIZE; hash++) {
        DFNameEntry *entry = table->bins[hash];
        while (entry != NULL) {
            DFNameEntry *next = entry->next;
            free(entry->name);
            free(entry->URI);
            free(entry);
            entry = next;
        }
    }
    free(table);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         DFNamespaceInfo                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DFNamespaceInfo DFNamespaceInfo;

struct DFNamespaceInfo {
    NamespaceID nsId;
    NamespaceDecl *decl;
};

DFNamespaceInfo *DFNamespaceInfoNew(NamespaceID nsId, const char *URI, const char *prefix)
{
    DFNamespaceInfo *info = (DFNamespaceInfo *)calloc(1,sizeof(DFNamespaceInfo));
    info->nsId = nsId;
    info->decl = (NamespaceDecl *)malloc(sizeof(NamespaceDecl));
    info->decl->namespaceURI = strdup(URI);
    info->decl->prefix = strdup(prefix);
    return info;
}

void DFNamespaceInfoFree(DFNamespaceInfo *info)
{
    free((char *)info->decl->namespaceURI);
    free((char *)info->decl->prefix);
    free(info->decl);
    free(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFTagInfo                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DFTagInfo DFTagInfo;

struct DFTagInfo {
    Tag tag;
    TagDecl *decl;
};

DFTagInfo *DFTagInfoNew(Tag tag, NamespaceID nsId, const char *localName)
{
    DFTagInfo *info = (DFTagInfo *)calloc(1,sizeof(DFTagInfo));
    info->tag = tag;
    info->decl = (TagDecl *)malloc(sizeof(TagDecl));
    info->decl->namespaceID = nsId;
    info->decl->localName = strdup(localName);
    return info;
}

void DFTagInfoFree(DFTagInfo *info)
{
    free((char *)info->decl->localName);
    free(info->decl);
    free(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFNameMap                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFHashTable *defaultNamespacesByURI = NULL;
DFNameHashTable *defaultTagsByNameURI = NULL;

static void DFNameMapAddNamespace(DFNameMap *map, NamespaceID nsId, const char *URI, const char *prefix);


struct DFNameMap {
    DFHashTable *namespacesByID;  // NSNumber -> NamespaceInfo
    DFHashTable *namespacesByURI; // NSString -> NamespaceInfo
    DFHashTable *tagsByID;        // NSNumber -> TagInfo


    DFNameHashTable *localTagsByNameURI;
    NamespaceID nextNamespaceId;
    Tag nextTag;
};

DFNameMap *DFNameMapNew(void)
{
    DFNameMap *map = (DFNameMap *)calloc(1,sizeof(DFNameMap));
    map->namespacesByID = DFHashTableNew2(NULL,NULL,997);
    map->namespacesByURI = DFHashTableNew2(NULL,(DFFreeFunction)DFNamespaceInfoFree,997);
    map->tagsByID = DFHashTableNew2(NULL,(DFFreeFunction)DFTagInfoFree,997);
    map->nextNamespaceId = PREDEFINED_NAMESPACE_COUNT;
    map->nextTag = PREDEFINED_TAG_COUNT;
    map->localTagsByNameURI = DFNameHashTableNew();
    NameMap_staticInit();
    return map;
}

void DFNameMapFree(DFNameMap *map)
{
    DFHashTableRelease(map->namespacesByID);
    DFHashTableRelease(map->namespacesByURI);
    DFHashTableRelease(map->tagsByID);
    DFNameHashTableFree(map->localTagsByNameURI);
    free(map);
}

static NamespaceID NameMap_namespaceIDForURI(DFNameMap *map, const char *URI)
{
    if (URI == NULL)
        return NAMESPACE_NULL;;
    DFNamespaceInfo *ns = DFHashTableLookup(map->namespacesByURI,(const char *)URI);
    if (ns == NULL) {
        ns = DFHashTableLookup(defaultNamespacesByURI,(const char *)URI);
    }
    assert(ns != NULL);
    return ns->nsId;
}

static void DFNameMapAddNamespace(DFNameMap *map, NamespaceID nsId, const char *URI, const char *prefix)
{
    assert(DFHashTableLookup(defaultNamespacesByURI,(const char *)URI) == NULL);
    assert(DFHashTableLookup(map->namespacesByURI,(const char *)URI) == NULL);
    DFNamespaceInfo *ns = DFNamespaceInfoNew(nsId,URI,prefix);
    if (nsId >= PREDEFINED_NAMESPACE_COUNT) {
        DFHashTableAddInt(map->namespacesByID,nsId,ns);
    }
    DFHashTableAdd(map->namespacesByURI,(const char *)URI,ns);
}

NamespaceID DFNameMapFoundNamespace(DFNameMap *map, const char *URI, const char *prefix)
{
    DFNamespaceInfo *existing;
    existing = DFHashTableLookup(defaultNamespacesByURI,(const char *)URI);
    if (existing != NULL)
        return existing->nsId;
    existing = DFHashTableLookup(map->namespacesByURI,(const char *)URI);
    if (existing != NULL)
        return existing->nsId;;
    NamespaceID nsId = map->nextNamespaceId++;
    DFNameMapAddNamespace(map,nsId,URI,prefix);
    return nsId;
}

const NamespaceDecl *DFNameMapNamespaceForID(DFNameMap *map, NamespaceID nsId)
{
    if (nsId < PREDEFINED_NAMESPACE_COUNT)
        return &PredefinedNamespaces[nsId];;
    DFNamespaceInfo *ns = DFHashTableLookupInt(map->namespacesByID,nsId);
    assert(ns != NULL);
    return ns->decl;
}

NamespaceID DFNameMapNamespaceCount(DFNameMap *map)
{
    return map->nextNamespaceId;
}

const TagDecl *DFNameMapNameForTag(DFNameMap *map, Tag tag)
{
    if (tag < PREDEFINED_TAG_COUNT)
        return &PredefinedTags[tag];;
    DFTagInfo *info = DFHashTableLookupInt(map->tagsByID,tag);
    assert(info != NULL);
    return info->decl;
}

Tag DFNameMapTagForName(DFNameMap *map, const char *URI, const char *localName)
{
    const DFNameEntry *entry = DFNameHashTableGet(defaultTagsByNameURI,localName,URI);
    if (entry == NULL)
        entry = DFNameHashTableGet(map->localTagsByNameURI,localName,URI);

    if (entry != NULL)
        return entry->tag;;

    // Dynamically allocate new tag
    NamespaceID nsId = NameMap_namespaceIDForURI(map,URI);
    Tag tag = map->nextTag++;
    DFTagInfo *info = DFTagInfoNew(tag,nsId,localName);
    DFHashTableAddInt(map->tagsByID,tag,info);
    DFNameHashTableAdd(map->localTagsByNameURI,localName,URI,tag,nsId);
    return tag;
}

static void NameMap_staticInit()
{
    if (defaultNamespacesByURI != NULL)
        return;
    defaultNamespacesByURI = DFHashTableNew2(NULL,NULL,997);
    defaultTagsByNameURI = DFNameHashTableNew();

    for (NamespaceID nsId = 1; nsId < PREDEFINED_NAMESPACE_COUNT; nsId++) {
        const NamespaceDecl *decl = &PredefinedNamespaces[nsId];

        const char *key = (const char *)decl->namespaceURI;
        assert(DFHashTableLookup(defaultNamespacesByURI,key) == NULL);
        DFNamespaceInfo *ns = DFNamespaceInfoNew(nsId,decl->namespaceURI,decl->prefix);
        DFHashTableAdd(defaultNamespacesByURI,key,ns);
    }
    for (Tag tag = MIN_ELEMENT_TAG; tag < PREDEFINED_TAG_COUNT; tag++) {
        const TagDecl *tagDecl = &PredefinedTags[tag];
        const NamespaceDecl *nsDecl = &PredefinedNamespaces[tagDecl->namespaceID];
        DFNameHashTableAdd(defaultTagsByNameURI,
                          tagDecl->localName,
                          nsDecl->namespaceURI,
                          tag,
                          tagDecl->namespaceID);
    }
}

static DFNameMap *builtinMap = NULL;

static void initBuiltinMap(void)
{
    builtinMap = DFNameMapNew();
}

static DFNameMap *BuiltinMapGet(void)
{
    static DFOnce once = DF_ONCE_INIT;
    DFInitOnce(&once,initBuiltinMap);
    return builtinMap;
}

const TagDecl *DFBuiltinMapNameForTag(Tag tag)
{
    return DFNameMapNameForTag(BuiltinMapGet(),tag);
}

Tag DFBuiltinMapTagForName(const char *URI, const char *localName)
{
    return DFNameMapTagForName(BuiltinMapGet(),URI,localName);
}
