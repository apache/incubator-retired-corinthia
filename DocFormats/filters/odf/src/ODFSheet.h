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

#ifndef DocFormats_ODFSheet_h
#define DocFormats_ODFSheet_h

#include <DocFormats/DFXMLForward.h>
#include "DFTypes.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ODFStyle                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ODFStyle ODFStyle;

struct ODFStyle {
    size_t retainCount;
    DFNode *element;
    char *selector;
};

ODFStyle *ODFStyleNew();
ODFStyle *ODFStyleRetain(ODFStyle *style);
void ODFStyleRelease(ODFStyle *style);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ODFSheet                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ODFSheet ODFSheet;

ODFSheet *ODFSheetNew(DFDocument *stylesDoc, DFDocument *contentDoc);
ODFSheet *ODFSheetRetain(ODFSheet *sheet);
void ODFSheetRelease(ODFSheet *sheet);
ODFStyle *ODFSheetStyleForSelector(ODFSheet *sheet, const char *selector);

#endif
