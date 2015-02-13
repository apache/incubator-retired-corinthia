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
#include "DFMarkupCompatibility.h"
#include "DFNameMap.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEPTH 256

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                      DFMarkupCompatibility                                     //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct MCDecl {
    NamespaceID nsId;
    Tag tag;
    MCAction action;
} MCDecl;

typedef struct MCRecord {
    MCDecl *decls;
    int count;
    int alloc;
    DFHashTable *namespaces;
} MCRecord;

struct MCStack {
    MCRecord records[MAX_DEPTH];
    int depth;
};

struct DFMarkupCompatibility {
    MCRecord records[MAX_DEPTH];
    int depth;
};

DFMarkupCompatibility *DFMarkupCompatibilityNew(void)
{
    DFMarkupCompatibility *mc = (DFMarkupCompatibility *)calloc(1,sizeof(DFMarkupCompatibility));
    return mc;
}

void DFMarkupCompatibilityFree(DFMarkupCompatibility *mc)
{
    while (mc->depth > 0)
        DFMarkupCompatibilityPop(mc);
    free(mc);
}

static void addDeclToRecord(MCRecord *record, NamespaceID nsId, Tag tag, MCAction action)
{
    record->count++;
    record->decls = (MCDecl *)realloc(record->decls,record->count*sizeof(MCDecl));
    record->decls[record->count-1].nsId = nsId;
    record->decls[record->count-1].tag = tag;
    record->decls[record->count-1].action = action;
}

void DFMarkupCompatibilityPush(DFMarkupCompatibility *mc, int nb_namespaces, const char **namespaces, DFNameMap *map)
{
    mc->depth++;
    if (mc->depth < MAX_DEPTH) {
        MCRecord *record = &mc->records[mc->depth-1];
        bzero(record,sizeof(MCRecord));
        if (nb_namespaces > 0) {
            record->namespaces = DFHashTableNew((DFCopyFunction)strdup,(DFFreeFunction)free);
            for (int i = 0; i < nb_namespaces; i++) {
                const char *nsPrefix = namespaces[i*2];
                const char *nsURI = namespaces[i*2+1];
                NamespaceID nsId = DFNameMapFoundNamespace(map,nsURI,nsPrefix);
                char nsIdStr[20];
                snprintf(nsIdStr,20,"%u",nsId);
                const char *prefix = "";
                if (nsPrefix != NULL)
                    prefix = (const char *)nsPrefix;
                DFHashTableAdd(record->namespaces,prefix,nsIdStr);
            }
        }
    }
}

void DFMarkupCompatibilityPop(DFMarkupCompatibility *mc)
{
    assert(mc->depth > 0);
    if (mc->depth < MAX_DEPTH) {
        MCRecord *record = &mc->records[mc->depth-1];
        free(record->decls);
        DFHashTableRelease(record->namespaces);
    }
    mc->depth--;
}

MCAction DFMarkupCompatibilityLookup(DFMarkupCompatibility *mc, NamespaceID nsId, Tag tag, int isElement)
{
    MCAction action = MCActionDefault;
    for (int recordIndex = mc->depth-1; recordIndex >= 0; recordIndex--) {
        MCRecord *record = &mc->records[recordIndex];
        for (int declIndex = 0; declIndex < record->count; declIndex++) {
            MCDecl *decl = &record->decls[declIndex];
            if (decl->nsId == nsId) {
                switch (decl->action) {
                    case MCActionIgnore:
                        return MCActionIgnore;
                    case MCActionProcessContent:
                        if ((decl->tag == 0) || ((decl->tag == tag) && isElement))
                            return MCActionProcessContent;
                        break;
                    case MCActionMustUnderstand:
                        return MCActionMustUnderstand;
                    default:
                        break;
                }
            }
        }
    }
    return action;
}

// FIXME: Not covered by tests
void DFMarkupCompatibilityProcessAttr(DFMarkupCompatibility *mc, Tag attr, const char *value, DFNameMap *map)
{
    const char **tokens = DFStringTokenize(value,isspace);
    for (int tokIndex = 0; tokens[tokIndex]; tokIndex++) {
        const char *component = tokens[tokIndex];

        char *prefix = NULL;
        char *localName = NULL;
        const char *colon = strchr(component,':');
        if (colon != NULL) {
            size_t colonPos = colon - component;
            prefix = DFSubstring(component,0,colonPos);
            localName = DFSubstring(component,colonPos+1,strlen(component));
        }
        else {
            prefix = strdup(component);
            localName = NULL;
        }

        const char *nsIdStr = NULL;

        // Find namespace ID for prefix
        for (int recordIndex = mc->depth-1; recordIndex >= 0; recordIndex--) {
            MCRecord *record = &mc->records[recordIndex];
            if (record->namespaces != NULL)
                nsIdStr = DFHashTableLookup(record->namespaces,prefix);
        }

        if (nsIdStr != NULL) {

            NamespaceID nsId = atoi(nsIdStr);
            Tag tag = 0;

            const NamespaceDecl *nsDecl = DFNameMapNamespaceForID(map,nsId);

            if (localName != NULL)
                tag = DFNameMapTagForName(map,nsDecl->namespaceURI,localName);

            switch (attr) {
                case MC_IGNORABLE:
                    addDeclToRecord(&mc->records[mc->depth-1],nsId,tag,MCActionIgnore);
                    break;
                case MC_PROCESSCONTENT:
                    addDeclToRecord(&mc->records[mc->depth-1],nsId,tag,MCActionProcessContent);
                    break;
                case MC_MUSTUNDERSTAND:
                    addDeclToRecord(&mc->records[mc->depth-1],nsId,tag,MCActionMustUnderstand);
                    break;
            }
        }
        free(prefix);
        free(localName);
    }
    free(tokens);
}
