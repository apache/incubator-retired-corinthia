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

#ifndef DocFormats_DFBDT_h
#define DocFormats_DFBDT_h

#include <DocFormats/DFXMLForward.h>

typedef DFNode *(*DFLookupConcreteFunction)(void *ctx, DFNode *abstract);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFLens                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {

    /** Indicates whether the particular concrete element is of a type that is known to the program.
     For example, you would return true for paragraphs, but false for embedded spreadsheets. */
    int (*isVisible)(void *ctx, DFNode *concrete);

    /** Converts the node from a Word/ODF document into a HTML element */
    DFNode *(*get)(void *ctx, DFNode *concrete);

    /** Updates an existing node that is already in the Word/ODF document, based on the
     corresponding node in the HTML document */
    void (*put)(void *ctx, DFNode *abstract, DFNode *concrete);

    /** Called when we find a new node in the HTML document for which we have no corresponding
     node in the Word/ODF document yet. This could be because we are updating an existing Word/ODF
     document and new content has been added, or because we are creating a new Word/ODF document
     and thus don't have *any* nodes corresponding to parts of the HTML document. */
    DFNode *(*create)(void *ctx, DFNode *abstract);

    /** Removes a node from the concrete document, because it has been determined that the
     corresponding node in the HTML document was removed */
    void (*remove)(void *ctx, DFNode *concrete);

} DFLens;

DFNode *BDTContainerGet(void *ctx, DFLens *theLens, DFNode *abstract, DFNode *concrete);
void BDTContainerPut(void *ctx, DFLens *theLens, DFNode *abstract, DFNode *concrete,
                     DFLookupConcreteFunction lookupConcrete);

#endif
