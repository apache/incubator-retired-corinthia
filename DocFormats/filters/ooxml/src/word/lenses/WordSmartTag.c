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
#include "DFDOM.h"
#include "DFCommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        WordSmartTagLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordSmartTagGet(WordGetData *get, DFNode *concrete)
{
    DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
    DFSetAttribute(span,HTML_CLASS,DFContainerClass);
    WordContainerGet(get,&WordParagraphContentLens,span,concrete);
    return span;
}

static int WordSmartTagIsVisible(WordPutData *put, DFNode *concrete)
{
    return 1;
}

static void WordSmartTagPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);

    // If there is a smartTagPr element, it must come first
    DFNode *smartTagPr = DFChildWithTag(concrete,WORD_SMARTTAGPR);
    if (smartTagPr != NULL)
        DFInsertBefore(concrete,smartTagPr,concrete->first);
}

static void WordSmartTagRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordParagraphContentLens.remove(put,child);
}

WordLens WordSmartTagLens = {
    .isVisible = WordSmartTagIsVisible,
    .get = WordSmartTagGet,
    .put = WordSmartTagPut,
    .create = NULL, // LENS FIXME
    .remove = WordSmartTagRemove,
};
