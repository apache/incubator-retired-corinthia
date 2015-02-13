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
#include "DFBDT.h"
#include "DFDOM.h"
#include "DFXML.h"
#include "DFCommon.h"
#include "DFHashTable.h"
#include <assert.h>
#include <stdlib.h>

int nullIsVisible(void *ctx, DFNode *concrete)
{
    return 0;
}

DFNode *BDTContainerGet(void *ctx, DFLens *theLens, DFNode *abstract, DFNode *concrete)
{
    assert(abstract != NULL);
    if ((theLens->isVisible != NULL) && (theLens->get != NULL)) {
        for (DFNode *child = concrete->first; child != NULL; child = child->next) {
            DFNode *absChild = theLens->get(ctx,child);
            if (absChild != NULL)
                DFAppendChild(abstract,absChild);
        }
    }
    return abstract;
}

void BDTContainerPut(void *ctx, DFLens *theLens, DFNode *abstract, DFNode *concrete,
                     DFLookupConcreteFunction lookupConcrete)
{
    int (*isVisible)(void *ctx, DFNode *concrete) = (theLens->isVisible != NULL) ? theLens->isVisible : nullIsVisible;

    int count = 0;
    for (DFNode *abs = abstract->first; abs != NULL; abs = abs->next)
        count++;

    DFNode **conChildren = (DFNode **)malloc(count*sizeof(DFNode*));
    count = 0;

    for (DFNode *abs = abstract->first; abs != NULL; abs = abs->next) {
        DFNode *con = NULL;

        if (lookupConcrete != NULL)
            con = lookupConcrete(ctx,abs);

        if (con == NULL) {
            if (theLens->create != NULL)
                con = theLens->create(ctx,abs);
        }
        else {
            if (theLens->put != NULL)
                theLens->put(ctx,abs,con);
        }

        if (con != NULL)
            conChildren[count++] = con;
    }

    DFHashTable *oldPrevHidden = DFHashTableNew(NULL,NULL);
    for (int i = count-1; i >= 0; i--) {
        DFNode *con = conChildren[i];
        DFNode *prevHidden = con->prev;
        while ((prevHidden != NULL) && isVisible(ctx,prevHidden))
            prevHidden = prevHidden->prev;
        if (prevHidden != NULL) {
            DFHashTableAddInt(oldPrevHidden,con->seqNo,prevHidden);
        }
        else {
            DFHashTableAddInt(oldPrevHidden,con->seqNo,(void *)1);
        }
    }

    // Remove concrete nodes for which their abstract counterparts no longer exist
    DFHashTable *remaining = DFHashTableNew(NULL,NULL);
    for (int i = 0; i < count; i++)
        DFHashTableAddInt(remaining,conChildren[i]->seqNo,"");
    DFNode *next;
    for (DFNode *con = concrete->first; con != NULL; con = next) {
        next = con->next;
        if (isVisible(ctx,con)) {
            if (DFHashTableLookupInt(remaining,con->seqNo) == NULL) {
                if (theLens->remove != NULL)
                    theLens->remove(ctx,con);
                DFRemoveNode(con);
            }
        }
    }

    // Find the last node
    DFNode *last = NULL;
    if (concrete->last != NULL) {
        last = concrete->last;
        while ((last->prev != NULL) && !isVisible(ctx,last->prev))
            last = last->prev;
    }

    // Reinsert all the nodes in the correct order
    for (int i = count-1; i >= 0; i--) {
        DFNode *con = conChildren[i];

        DFNode *oldNext = con->next;
        while ((oldNext != NULL) && !isVisible(ctx,oldNext))
            oldNext = oldNext->next;

        DFNode *newNext = (i+1 < count) ? conChildren[i+1] : NULL;
        if (newNext == NULL)
            newNext = last;

        DFInsertBefore(concrete,con,newNext);
    }

    // Fixup stage - move nodes backwards as much as possible to their previous prevHidden
    for (DFNode *con = concrete->first; con != NULL; con = next) {
        next = con->next;
        if (!isVisible(ctx,con))
            continue;
        void *ph = DFHashTableLookupInt(oldPrevHidden,con->seqNo);
        if (ph == NULL)
            continue;
        if (ph == (void*)1)
            ph = NULL;;
        DFNode *prevHidden = (DFNode *)ph;
        if (prevHidden == NULL)
            continue;
        DFNode *insertionPoint = con->next;
        DFNode *actual = con->prev;
        int blockedByPrev = 0;
        int found = 0;
        for (;;) {
            if (!blockedByPrev) {
                if (actual == NULL)
                    insertionPoint = concrete->first;
                else
                    insertionPoint = actual->next;
            }
            if ((actual != NULL) && isVisible(ctx,actual))
                blockedByPrev = 1;
            if (actual == prevHidden) {
                found = 1;
                break;
            }
            if (actual == NULL) {
                break;
            }
            actual = actual->prev;
        }
        if (found)
            DFInsertBefore(concrete,con,insertionPoint);
    }

    free(conChildren);
    DFHashTableRelease(oldPrevHidden);
    DFHashTableRelease(remaining);
}
