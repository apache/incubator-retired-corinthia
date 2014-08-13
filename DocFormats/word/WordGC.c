//
//  WordGC.m
//  DocFormats
//
//  Created by Peter Kelly on 1/01/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#include "WordGC.h"
#include "WordConverter.h"
#include "WordPackage.h"
#include "OPC.h"
#include "DFDOM.h"
#include "DFFilesystem.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFCommon.h"

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
            if (!rel->external) {
                char *path = DFAppendPathComponent(package->tempPath,rel->target);
                DFDeleteFile(path,NULL);
                free(path);
            }
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
