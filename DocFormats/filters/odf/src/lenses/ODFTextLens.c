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
 *  There is no direct HTML to map to here. Just create a div
 *  And maybe it can be styled with page attributes or something
 */
static DFNode *ODFTextGet(ODFGetData *get, DFNode *concrete)
{
    printf("ODFTextLensGet\n");
    DFNode *abstract = ODFConverterCreateAbstract(get,HTML_DIV,concrete);
    ODFContainerGet(get,&ODFTextLevelLens,abstract,concrete);
    return abstract;
}

static int ODFTextIsVisible(ODFPutData *put, DFNode *concrete)
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

static void ODFTextPut(ODFPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (concrete->tag) {
        case TEXT_P:
            ODFParagraphLens.put(put,abstract,concrete);
            break;
        case TEXT_H:
            ODFHeaderLens.put(put,abstract,concrete);
            break;
        //case ODF_TBL:
//            ODFTableLens.put(put,abstract,concrete);
            break;
        default:
            break;
    }
}

static void ODFTextRemove(ODFPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case TEXT_P:
            ODFParagraphLens.remove(put,concrete);
            break;
        case TEXT_H:
            ODFHeaderLens.remove(put,concrete);
            break;
//        case ODF_TBL:
//            ODFTableLens.remove(put,concrete);
            break;
        default:
            break;
    }
}

static DFNode *ODFTextCreate(ODFPutData *put, DFNode *abstract)
{
    DFNode *concrete = NULL;
    return concrete;
}

ODFLens ODFTextLens = {
    .isVisible = ODFTextIsVisible,
    .get = ODFTextGet,
    .put = ODFTextPut,
    .create = ODFTextCreate,
    .remove = ODFTextRemove,
};
