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

#include <DocFormats/DFStorage.h>
#include "DFDOM.h"
#include "ODFPackage.h"

#include "CSS.h"
#include "CSSSheet.h"

typedef struct {
    DFDocument *html;
    DFNode *body; //just cos its easier for the moment
    DFStorage *abstractStorage;
    char *idPrefix;
    ODFPackage *package;
    struct ODFSheet *styles;
    DFBuffer *warnings;
    CSSSheet *styleSheet;
} ODFConverter ;

typedef struct {
    ODFConverter *conv;
} ODFGetData;

typedef struct {
    ODFConverter *conv;
    DFDocument *contentDoc;
    DFHashTable *numIdByHtmlId;
    DFHashTable *htmlIdByNumId;
} ODFPutData;

int ODFConverterGet(DFDocument *html, DFStorage *abstractStorage, ODFPackage *package, const char *idPrefix, DFError **error);

DFNode *ODFConverterCreateAbstract(ODFGetData *get, Tag tag, DFNode *concrete);
DFNode *ODFConverterGetConcrete(ODFPutData *put, DFNode *abstract);
