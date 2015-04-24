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

#ifndef DocFormats_WordLenses_h
#define DocFormats_WordLenses_h

#include "DFBDT.h"
#include "WordConverter.h"
#include "OOXMLTypedefs.h"

struct WordLens{
    int (*isVisible)(WordPutData *put, DFNode *concrete);
    DFNode *(*get)(WordGetData *get, DFNode *concrete);
    void (*put)(WordPutData *put, DFNode *abstract, DFNode *concrete);
    DFNode *(*create)(WordPutData *put, DFNode *abstract);
    void (*remove)(WordPutData *put, DFNode *concrete);
};

extern WordLens WordBlockLevelLens;
extern WordLens WordBodyLens;
extern WordLens WordBookmarkLens;
extern WordLens WordChangeLens;
extern WordLens WordDocumentLens;
extern WordLens WordDrawingLens;
extern WordLens WordEquationLens;
extern WordLens WordFieldLens;
extern WordLens WordHyperlinkLens;
extern WordLens WordParagraphLens;
extern WordLens WordParagraphContentLens;
extern WordLens WordRunLens;
extern WordLens WordRunContentLens;
extern WordLens WordSmartTagLens;
extern WordLens WordTableLens;

DFNode *WordContainerGet(WordGetData *get, WordLens *childLens, DFNode *abstract, DFNode *concrete);
void WordContainerPut(WordPutData *put, WordLens *childLens, DFNode *abstract, DFNode *concrete);

#endif
