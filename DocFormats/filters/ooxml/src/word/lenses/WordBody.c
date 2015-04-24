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
//                                          WordBodylens                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordBodyGet(WordGetData *get, DFNode *concrete)
{
    DFNode *abstract = WordConverterCreateAbstract(get,HTML_BODY,concrete);
    WordContainerGet(get,&WordBlockLevelLens,abstract,concrete);
    return abstract;
}

static void WordBodyPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    WordContainerPut(put,&WordBlockLevelLens,abstract,concrete);
}

static void WordBodyRemove(WordPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        WordBlockLevelLens.remove(put,child);
}

WordLens WordBodyLens = {
    .isVisible = NULL, // LENS FIXME
    .get = WordBodyGet,
    .put = WordBodyPut,
    .create = NULL, // LENS FIXME
    .remove = WordBodyRemove,
};
