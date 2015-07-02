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
#include "ODFLenses.h"
#include "DFDOM.h"
#include "DFCommon.h"
#include "text/gbg_test.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       ODFTextLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Not a standard lens - we switch to the correct lens
 * based on our tag value
 */
static DFNode *ODFTextLevelGet(ODFGetData *get, DFNode *concrete)
{
    printf(CYAN "ODFTextLevelLensGet\n" RESET);
    printf("Tag %s\n", translateXMLEnumName[concrete->tag]);
    switch (concrete->tag) {
        case TEXT_P:
            printf("ODFTextLevelGet found TEXT_P\n");
            return ODFParagraphLens.get(get,concrete);
        case TEXT_H:
            printf("ODFTextLevelGet found TEXT_H\n");
            return ODFHeaderLens.get(get,concrete);
            //        case ODF_TBL:
//            return ODFTableLens.get(get,concrete);
        default:
            printf("Tag %s\n", translateXMLEnumName[concrete->tag]);
            return NULL;
    }
}

static int ODFTextLevelIsVisible(ODFPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case TEXT_P:
            return ODFParagraphLens.isVisible(put,concrete);
        case TEXT_H:
            return ODFHeaderLens.isVisible(put,concrete);
            //        case ODF_TBL:
//            return ODFTableLens.isVisible(put,concrete);
        default:
            return 0;
    }
}

static void ODFTextLevelPut(ODFPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (concrete->tag) {
        case TEXT_P:
            ODFParagraphLens.put(put,abstract,concrete);
            break;
        case TEXT_H:
            ODFHeaderLens.put(put,abstract,concrete);
            break;
        default:
            break;
    }
}

static void ODFTextLevelRemove(ODFPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case TEXT_P:
            ODFParagraphLens.remove(put,concrete);
            break;
        case TEXT_H:
            ODFHeaderLens.remove(put,concrete);
            break;
        default:
            break;
    }
}

static DFNode *ODFTextLevelCreate(ODFPutData *put, DFNode *abstract)
{
    DFNode *concrete = NULL;
    return concrete;
}

ODFLens ODFTextLevelLens = {
    .isVisible = ODFTextLevelIsVisible,
    .get = ODFTextLevelGet,
    .put = ODFTextLevelPut,
    .create = ODFTextLevelCreate,
    .remove = ODFTextLevelRemove,
};
