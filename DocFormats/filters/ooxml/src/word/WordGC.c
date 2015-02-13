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
#include "WordGC.h"
#include "WordConverter.h"
#include "WordPackage.h"
#include "OPC.h"
#include "DFDOM.h"
#include "DFFilesystem.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             WordGC                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void findReferences(DFHashTable *referencedIds, DFNode *node)
{
    if (node->tag >= MIN_ELEMENT_TAG) {
        switch (node->tag) {
            case WORD_HYPERLINK: {
                const char *rId = DFGetAttribute(node,OREL_ID);
                if (rId != NULL)
                    DFHashTableAdd(referencedIds,rId,"");
                break;
            }
            case DML_MAIN_BLIP: {
                const char *rId = DFGetAttribute(node,OREL_EMBED);
                if (rId != NULL)
                    DFHashTableAdd(referencedIds,rId,"");
                break;
            }
        }
    }

    for (DFNode *child = node->first; child != NULL; child = child->next)
        findReferences(referencedIds,child);
}

static void collect(WordPackage *package, DFHashTable *referencedIds)
{
    findReferences(referencedIds,package->document->docNode);

    OPCRelationshipSet *relationships = package->documentPart->relationships;
    const char **allIds = OPCRelationshipSetAllIds(relationships);
    for (int i = 0; allIds[i]; i++) {
        const char *rId = allIds[i];
        // FIXME: We should check all targets of all relationships in the package to determine
        // the set of referenced files, and only delete the file if there are no more references
        // to it.
        OPCRelationship *rel = OPCRelationshipSetLookupById(relationships,rId);
        if (rel->needsRemoveCheck && (DFHashTableLookup(referencedIds,rel->rId) == NULL)) {
            if (!rel->external)
                DFStorageDelete(package->opc->storage,rel->target,NULL);
            OPCRelationshipSetRemove(relationships,rel);
        }
    }
    free(allIds);
}

void WordGarbageCollect(WordPackage *package)
{
    assert(package->documentPart != NULL);
    assert(package->document != NULL);
    DFHashTable *referencedIds = DFHashTableNew(NULL,NULL); // used as a set
    collect(package,referencedIds);
    DFHashTableRelease(referencedIds);
}
