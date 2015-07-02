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

#pragma once

#include "DFBDT.h"
#include "ODFConverter.h"
#include "OOXMLTypedefs.h"
#include "text/color.h"
#include <stdio.h>

typedef struct {
    int (*isVisible)(ODFPutData *put, DFNode *concrete);
    DFNode *(*get)(ODFGetData *get, DFNode *concrete);
    void (*put)(ODFPutData *put, DFNode *abstract, DFNode *concrete);
    DFNode *(*create)(ODFPutData *put, DFNode *abstract);
    void (*remove)(ODFPutData *put, DFNode *concrete);
}ODFLens;

/**
 * Elements of interest at the top level of an ODF Content XML
 * are the body, automatic styles.
 *
 * The body will then have the type of document text, spreadsheet
 * or presentation.
 *
 */
extern ODFLens ODFDocumentLens;
extern ODFLens ODFContentLens;
extern ODFLens ODFBodyLens;
extern ODFLens ODFTextLens;
extern ODFLens ODFTextLevelLens;
extern ODFLens ODFSpreadsheetLens;
extern ODFLens ODFPresentationLens;
extern ODFLens ODFParagraphLens;
extern ODFLens ODFHeaderLens;
extern ODFLens ODFParagraphContentLens;

DFNode *ODFContainerGet(ODFGetData *get, ODFLens *childLens, DFNode *abstract, DFNode *concrete);
void ODFContainerPut(ODFPutData *put, ODFLens *childLens, DFNode *abstract, DFNode *concrete);

