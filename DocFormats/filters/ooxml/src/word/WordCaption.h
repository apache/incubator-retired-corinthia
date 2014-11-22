// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DocFormats_WordCaption_h
#define DocFormats_WordCaption_h

#include "DFXMLForward.h"
#include "DFTypes.h"
#include "OOXMLTypedefs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordCaption                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordCaption {
    size_t retainCount;
    DFNode *element;
    DFNode *number;
    DFNode *contentStart;
    struct WordBookmark *captionTextBookmark;
    struct WordBookmark *labelNumBookmark;
    struct WordBookmark *textBookmark;
};

WordCaption *WordCaptionNew(DFNode *element);
WordCaption *WordCaptionRetain(WordCaption *caption);
void WordCaptionRelease(WordCaption *caption);

#endif
