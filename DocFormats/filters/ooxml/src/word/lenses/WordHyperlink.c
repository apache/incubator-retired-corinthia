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
#include "WordLenses.h"
#include "WordStyles.h"
#include "OPC.h"
#include "CSSProperties.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <string.h>

static void WordHyperlinkPut(WordPutData *put, DFNode *abstract, DFNode *concrete);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        WordHyperlinkLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *WordHyperlinkGetHref(WordConverter *converter, DFNode *concrete)
{
    const char *rId = DFGetAttribute(concrete,OREL_ID);
    if (rId == NULL)
        return NULL;;
    OPCRelationship *rel = OPCRelationshipSetLookupById(converter->package->documentPart->relationships,rId);
    if (rel == NULL)
        return NULL;
    if (rel->external && !strcmp(rel->type,WORDREL_HYPERLINK))
        return rel->target;
    else
        return NULL;
}

static DFNode *WordHyperlinkGet(WordGetData *get, DFNode *concrete)
{
    const char *href = WordHyperlinkGetHref(get->conv,concrete);
    if (href == NULL)
        return NULL;;
    DFNode *abstract = WordConverterCreateAbstract(get,HTML_A,concrete);
    DFSetAttribute(abstract,HTML_HREF,href);
    WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);
    return abstract;
}

static int WordHyperlinkIsVisible(WordPutData *put, DFNode *concrete)
{
    return (WordHyperlinkGetHref(put->conv,concrete) != NULL);
}

static DFNode *WordHyperlinkCreate(WordPutData *put, DFNode *abstract)
{
    if (DFGetAttribute(abstract,HTML_HREF) == NULL)
        return NULL;;
    DFNode *concrete = DFCreateElement(put->contentDoc,WORD_HYPERLINK);
    DFSetAttribute(concrete,WORD_HISTORY,"1");
    WordHyperlinkPut(put,abstract,concrete);

    CSSSheet *sheet = put->conv->styleSheet;

    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        if (child->tag == WORD_R) {
            DFNode *rPr = DFChildWithTag(child,WORD_RPR);
            if (rPr == NULL) {
                rPr = DFCreateElement(put->contentDoc,WORD_RPR);
                DFInsertBefore(child,rPr,child->first);
            }
            CSSProperties *properties = CSSPropertiesNew();
            const char *styleId = NULL;
            WordGetRPr(rPr,properties,&styleId,put->conv->theme);
            WordPutRPr(rPr,properties,"Hyperlink",put->conv->theme);
            CSSPropertiesRelease(properties);

            CSSStyle *style = CSSSheetLookupElement(sheet,"span","Hyperlink",0,0);
            assert(style != NULL);
        }
    }

    return concrete;
}

static void WordHyperlinkPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    if ((abstract->tag != HTML_A) || (concrete->tag != WORD_HYPERLINK))
        return;;

    const char *oldRelId = DFGetAttribute(concrete,OREL_ID);
    OPCPart *part = put->conv->package->documentPart;
    OPCRelationship *oldRel = NULL;
    if (oldRelId != NULL)
        oldRel = OPCRelationshipSetLookupById(part->relationships,oldRelId);;

    const char *oldHref = (oldRel != NULL) ? oldRel->target : NULL;
    const char *href = DFGetAttribute(abstract,HTML_HREF);
    if (!DFStringEquals(oldHref,href)) {
        if (href != NULL) {
            OPCRelationship *rel = OPCRelationshipSetAddType(put->conv->package->documentPart->relationships,
                                                             WORDREL_HYPERLINK,href,1);
            DFSetAttribute(concrete,OREL_ID,rel->rId);
        }
        if (oldRel != NULL)
            oldRel->needsRemoveCheck = 1;
    }
    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);
}

static void WordHyperlinkRemove(WordPutData *put, DFNode *concrete)
{
    const char *rId = DFGetAttribute(concrete,OREL_ID);
    if (rId == NULL)
        return;;
    OPCPart *part = put->conv->package->documentPart;
    OPCRelationship *rel = OPCRelationshipSetLookupById(part->relationships,rId);
    if (rel == NULL)
        return;

    rel->needsRemoveCheck = 1;
}

WordLens WordHyperlinkLens = {
    .isVisible = WordHyperlinkIsVisible,
    .get = WordHyperlinkGet,
    .put = WordHyperlinkPut,
    .create = WordHyperlinkCreate,
    .remove = WordHyperlinkRemove,
};
