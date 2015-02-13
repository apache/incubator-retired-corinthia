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
//                                       WordBlockLevelLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordBlockLevelGet(WordGetData *get, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            return WordParagraphLens.get(get,concrete);
        case WORD_TBL:
            return WordTableLens.get(get,concrete);
        default:
            return NULL;
    }
}

static int WordBlockLevelIsVisible(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            return WordParagraphLens.isVisible(put,concrete);
        case WORD_TBL:
            return WordTableLens.isVisible(put,concrete);
        default:
            return 0;
    }
}

static void WordBlockLevelPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            WordParagraphLens.put(put,abstract,concrete);
            break;
        case WORD_TBL:
            WordTableLens.put(put,abstract,concrete);
            break;
        default:
            break;
    }
}

static void WordBlockLevelRemove(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_P:
            WordParagraphLens.remove(put,concrete);
            break;
        case WORD_TBL:
            WordTableLens.remove(put,concrete);
            break;
        default:
            break;
    }
}

static DFNode *WordBlockLevelCreate(WordPutData *put, DFNode *abstract)
{
    DFNode *concrete = NULL;
    switch (abstract->tag) {
        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_P:
        case HTML_FIGURE:
            return WordParagraphLens.create(put,abstract);
        case HTML_TABLE:
            concrete = DFCreateElement(put->contentDoc,WORD_TBL);
            break;
    }
    if (concrete != NULL)
        WordBlockLevelPut(put,abstract,concrete);
    return concrete;
}

WordLens WordBlockLevelLens = {
    .isVisible = WordBlockLevelIsVisible,
    .get = WordBlockLevelGet,
    .put = WordBlockLevelPut,
    .create = WordBlockLevelCreate,
    .remove = WordBlockLevelRemove,
};
