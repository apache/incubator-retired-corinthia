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
#include "DFString.h"
#include "WordNotes.h"
#include "WordObjects.h"
#include "WordSheet.h"
#include "WordStyles.h"
#include "CSS.h"
#include "CSSProperties.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordRunLens                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int WordRunIsVisible(WordPutData *put, DFNode *concrete)
{
    return (concrete->tag == WORD_R);
}

static void WordRunPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    if ((abstract->tag != HTML_SPAN) || (concrete->tag != WORD_R))
        return;

    if (WordRunPutNote(put,abstract,concrete))
        return;

    WordContainerPut(put,&WordRunContentLens,abstract,concrete);

    DFNode *rPr = DFChildWithTag(concrete,WORD_RPR);
    if (rPr == NULL)
        rPr = DFCreateElement(put->contentDoc,WORD_RPR);
    DFInsertBefore(concrete,rPr,concrete->first); // Ensure first, in case [super put] moved it

    char *selector = CSSMakeNodeSelector(abstract);
    const char *styleId = WordSheetStyleIdForSelector(put->conv->styles,selector);

    const char *inlineCSSText = DFGetAttribute(abstract,HTML_STYLE);
    CSSProperties *properties = CSSPropertiesNewWithString(inlineCSSText);
    WordPutRPr(rPr,properties,styleId,put->conv->theme);
    CSSPropertiesRelease(properties);

    if (rPr->first == NULL)
        DFRemoveNode(rPr);

    free(selector);
}

static DFNode *WordRunGet(WordGetData *get, DFNode *concrete)
{
    assert(concrete->tag == WORD_R);

    // First check the run to see if it's a footnote or endnote reference. These need to be handled specially,
    // as we place the actual content of the footnote or endnote in-line in the HTML output.
    DFNode *note = WordRunGetNote(get,concrete);
    if (note != NULL)
        return note;

    // If we didn't find a footnote or endnote reference, treat it as a normal content run
    assert(concrete->tag == WORD_R);
    DFNode *rPr = DFChildWithTag(concrete,WORD_RPR);
    CSSProperties *properties = CSSPropertiesNew();
    const char *styleId = NULL;
    if (rPr != NULL)
        WordGetRPr(rPr,properties,&styleId,get->conv->theme);;

    const char *selector = WordSheetSelectorForStyleId(get->conv->styles,"character",styleId);
    Tag elementName = (selector == NULL) ? HTML_SPAN : CSSSelectorGetTag(selector);
    char *className = (selector == NULL) ? NULL : CSSSelectorCopyClassName(selector);
    DFNode *abstract = WordConverterCreateAbstract(get,elementName,concrete);
    DFSetAttribute(abstract,HTML_CLASS,className);
    free(className);

    DFHashTable *collapsed = CSSCollapseProperties(properties);
    char *styleValue = CSSSerializeProperties(collapsed);
    DFHashTableRelease(collapsed);
    if (strlen(styleValue) > 0)
        DFSetAttribute(abstract,HTML_STYLE,styleValue);
    free(styleValue);

    CSSPropertiesRelease(properties);

    WordContainerGet(get,&WordRunContentLens,abstract,concrete);
    return abstract;
}

static void WordRunRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordRunContentLens.remove(put,child);
}

WordLens WordRunLens = {
    .isVisible = WordRunIsVisible,
    .get = WordRunGet,
    .put = WordRunPut,
    .create = NULL, // LENS FIXME
    .remove = WordRunRemove,
};
