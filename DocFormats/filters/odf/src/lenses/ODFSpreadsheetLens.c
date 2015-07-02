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

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       ODFSpreadsheetLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *ODFSpreadsheetGet(ODFGetData *get, DFNode *concrete)
{
    switch (concrete->tag) {
        default:
            return NULL;
    }
}

static int ODFSpreadsheetIsVisible(ODFPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        default:
            return 0;
    }
}

static void ODFSpreadsheetPut(ODFPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (concrete->tag) {
        default:
            break;
    }
}

static void ODFSpreadsheetRemove(ODFPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        default:
            break;
    }
}

static DFNode *ODFSpreadsheetCreate(ODFPutData *put, DFNode *abstract)
{
    DFNode *concrete = NULL;
    return concrete;
}

ODFLens ODFSpreadsheetLens = {
    .isVisible = ODFSpreadsheetIsVisible,
    .get = ODFSpreadsheetGet,
    .put = ODFSpreadsheetPut,
    .create = ODFSpreadsheetCreate,
    .remove = ODFSpreadsheetRemove,
};
