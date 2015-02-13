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
#include "DFString.h"
#include "DFCommon.h"

static void WordChangePut(WordPutData *put, DFNode *abstract, DFNode *concrete);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         WordChangeLens                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordChangeGet(WordGetData *get, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_INS: {
            DFNode *abstract = WordConverterCreateAbstract(get,HTML_INS,concrete);
            WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);
            return abstract;
        }
        case WORD_DEL: {
            DFNode *abstract = WordConverterCreateAbstract(get,HTML_DEL,concrete);
            WordContainerGet(get,&WordParagraphContentLens,abstract,concrete);
            return abstract;
        }
//        case WORD_MOVEFROM:
//        case WORD_MOVETO:
//            break;
        default:
            return NULL;
    }
}

static int WordChangeIsVisible(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_INS:
        case WORD_DEL:
            return 1;
        default:
            return 0;
    }
}

static DFNode *WordChangeCreate(WordPutData *put, DFNode *abstract)
{
    switch (abstract->tag) {
        case HTML_INS: {
            DFNode *concrete = DFCreateElement(put->contentDoc,WORD_INS);
            WordChangePut(put,abstract,concrete);
            return concrete;
        }
        case HTML_DEL: {
            DFNode *concrete = DFCreateElement(put->contentDoc,WORD_INS);
            WordChangePut(put,abstract,concrete);
            return concrete;
        }
        default:
            return NULL;
    }
}

static void WordChangePut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordParagraphContentLens,abstract,concrete);
}

static void WordChangeRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordParagraphContentLens.remove(put,child);
}

WordLens WordChangeLens = {
    .isVisible = WordChangeIsVisible,
    .get = WordChangeGet,
    .put = WordChangePut,
    .create = WordChangeCreate,
    .remove = WordChangeRemove,
};
