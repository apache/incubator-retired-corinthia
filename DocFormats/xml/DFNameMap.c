//
//  DFNameMap.c
//  DocFormats
//
//  Created by Peter Kelly on 15/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "DFNameMap.h"
#include "DFDOM.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <pthread.h>

static void NameMap_staticInit();

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         DFNameHashTable                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define HASH_TABLE_SIZE 983

typedef struct DFNameEntry {
    xmlChar *name;
    xmlChar *URI;
    Tag tag;
    unsigned int namespaceID;
    TagDecl tagDecl;
    struct DFNameEntry *next;
} DFNameEntry;

typedef struct DFNameHashTable {
    DFNameEntry *bins[HASH_TABLE_SIZE];
} DFNameHashTable;

static uint32_t DFNameHashTableHash(const xmlChar *name, const xmlChar *URI)
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

static const DFNameEntry *DFNameHashTableGet(DFNameHashTable *table, const xmlChar *name, const xmlChar *URI)
{
    if (URI == NULL)
        URI = (const xmlChar *)"";
    uint32_t hash = DFNameHashTableHash(name,URI)%HASH_TABLE_SIZE;
    for (DFNameEntry *entry = table->bins[hash]; entry != NULL; entry = entry->next) {
        if (!xmlStrcmp(name,entry->name) && !xmlStrcmp(URI,entry->URI))
            return entry;
    }
    return 0;
}

static void DFNameHashTableAdd(DFNameHashTable *table, const xmlChar *name, const xmlChar *URI,
                               Tag tag, unsigned int namespaceID)
{
    if (URI == NULL)
        URI = (const xmlChar *)"";
    uint32_t hash = DFNameHashTableHash(name,URI)%HASH_TABLE_SIZE;
    DFNameEntry *entry = (DFNameEntry *)malloc(sizeof(DFNameEntry));
    entry->name = xmlStrdup(name);
    entry->URI = xmlStrdup(URI);
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

DFNamespaceInfo *DFNamespaceInfoNew(NamespaceID nsId, const xmlChar *URI, const xmlChar *prefix)
{
    DFNamespaceInfo *info = (DFNamespaceInfo *)calloc(1,sizeof(DFNamespaceInfo));
    info->nsId = nsId;
    info->decl = (NamespaceDecl *)malloc(sizeof(NamespaceDecl));
    info->decl->namespaceURI = (char *)xmlStrdup(URI);
    info->decl->prefix = (char *)xmlStrdup(prefix);
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

DFTagInfo *DFTagInfoNew(Tag tag, NamespaceID nsId, const xmlChar *localName)
{
    DFTagInfo *info = (DFTagInfo *)calloc(1,sizeof(DFTagInfo));
    info->tag = tag;
    info->decl = (TagDecl *)malloc(sizeof(TagDecl));
    info->decl->namespaceID = nsId;
    info->decl->localName = (char *)xmlStrdup(localName);
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

static void DFNameMapAddNamespace(DFNameMap *map, NamespaceID nsId, const xmlChar *URI, const xmlChar *prefix);


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

static NamespaceID NameMap_namespaceIDForURI(DFNameMap *map, const xmlChar *URI)
{
    if (URI == NULL)
        return NAMESPACE_NULL;
    DFNamespaceInfo *ns = DFHashTableLookup(map->namespacesByURI,(const char *)URI);
    if (ns == NULL) {
        ns = DFHashTableLookup(defaultNamespacesByURI,(const char *)URI);
    }
    assert(ns != NULL);
    return ns->nsId;
}

static void DFNameMapAddNamespace(DFNameMap *map, NamespaceID nsId, const xmlChar *URI, const xmlChar *prefix)
{
    assert(DFHashTableLookup(defaultNamespacesByURI,(const char *)URI) == NULL);
    assert(DFHashTableLookup(map->namespacesByURI,(const char *)URI) == NULL);
    DFNamespaceInfo *ns = DFNamespaceInfoNew(nsId,URI,prefix);
    if (nsId >= PREDEFINED_NAMESPACE_COUNT) {
        DFHashTableAddInt(map->namespacesByID,nsId,ns);
    }
    DFHashTableAdd(map->namespacesByURI,(const char *)URI,ns);
}

NamespaceID DFNameMapFoundNamespace(DFNameMap *map, const xmlChar *URI, const xmlChar *prefix)
{
    DFNamespaceInfo *existing;
    existing = DFHashTableLookup(defaultNamespacesByURI,(const char *)URI);
    if (existing != NULL)
        return existing->nsId;
    existing = DFHashTableLookup(map->namespacesByURI,(const char *)URI);
    if (existing != NULL)
        return existing->nsId;
    NamespaceID nsId = map->nextNamespaceId++;
    DFNameMapAddNamespace(map,nsId,URI,prefix);
    return nsId;
}

const NamespaceDecl *DFNameMapNamespaceForID(DFNameMap *map, NamespaceID nsId)
{
    if (nsId < PREDEFINED_NAMESPACE_COUNT)
        return &PredefinedNamespaces[nsId];
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
        return &PredefinedTags[tag];
    DFTagInfo *info = DFHashTableLookupInt(map->tagsByID,tag);
    assert(info != NULL);
    return info->decl;
}

Tag DFNameMapTagForName(DFNameMap *map, const xmlChar *URI, const xmlChar *localName)
{
    const DFNameEntry *entry = DFNameHashTableGet(defaultTagsByNameURI,localName,URI);
    if (entry == NULL)
        entry = DFNameHashTableGet(map->localTagsByNameURI,localName,URI);

    if (entry != NULL)
        return entry->tag;

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
        DFNamespaceInfo *ns = DFNamespaceInfoNew(nsId,(xmlChar *)decl->namespaceURI,(xmlChar *)decl->prefix);
        DFHashTableAdd(defaultNamespacesByURI,key,ns);
    }
    for (Tag tag = MIN_ELEMENT_TAG; tag < PREDEFINED_TAG_COUNT; tag++) {
        const TagDecl *tagDecl = &PredefinedTags[tag];
        const NamespaceDecl *nsDecl = &PredefinedNamespaces[tagDecl->namespaceID];
        DFNameHashTableAdd(defaultTagsByNameURI,
                          (xmlChar *)tagDecl->localName,
                          (xmlChar *)nsDecl->namespaceURI,
                          tag,
                          tagDecl->namespaceID);
    }
}

static DFNameMap *builtinMap = NULL;

static void initBuiltinMap()
{
    builtinMap = DFNameMapNew();
}

static DFNameMap *BuiltinMapGet(void)
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once,initBuiltinMap);
    return builtinMap;
}

const TagDecl *DFBuiltinMapNameForTag(Tag tag)
{
    return DFNameMapNameForTag(BuiltinMapGet(),tag);
}

Tag DFBuiltinMapTagForName(const xmlChar *URI, const xmlChar *localName)
{
    return DFNameMapTagForName(BuiltinMapGet(),URI,localName);
}
