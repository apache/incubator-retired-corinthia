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
#include "WordDrawing.h"
#include "WordNotes.h"
#include "DFString.h"
#include "DFCommon.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       WordRunContentLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *WordRunContentGet(WordGetData *get, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_T:
        case WORD_DELTEXT: {
            DFBuffer *buf = DFBufferNew();
            DFNodeTextToBuffer(concrete,buf);
            DFNode *abstract = DFCreateTextNode(get->conv->html,buf->data);
            DFBufferRelease(buf);
            return abstract;
        }
        case WORD_DRAWING:
        case WORD_OBJECT:
        case WORD_PICT:
            return WordDrawingGet(get,concrete);
        case WORD_TAB: {
            DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
            DFSetAttribute(span,HTML_CLASS,DFTabClass);
            return span;
        }
        case WORD_BR: {
            const char *type = DFGetAttribute(concrete,WORD_TYPE);
            if (DFStringEquals(type,"column")) {
                DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
                DFSetAttribute(span,HTML_CLASS,DFPlaceholderClass);
                DFCreateChildTextNode(span,"[Column break]");
                return span;
            }
            else if (DFStringEquals(type,"page")) {
                DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
                DFSetAttribute(span,HTML_CLASS,DFPlaceholderClass);
                DFCreateChildTextNode(span,"[Page break]");
                return span;
            }
            else {
                return WordConverterCreateAbstract(get,HTML_BR,concrete);
            }
        }
        default:
            return NULL;
    }
}

static int WordRunContentIsVisible(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_T:
        case WORD_DELTEXT:
            return 1;
        case WORD_DRAWING:
        case WORD_OBJECT:
        case WORD_PICT:
            return WordDrawingIsVisible(put,concrete);
        case WORD_TAB:
            return 1;
        case WORD_BR:
            return 1;
        default:
            return 0;
    }
}

static DFNode *WordRunContentCreate(WordPutData *put, DFNode *abstract)
{
    switch (abstract->tag) {
        case DOM_TEXT: {
            DFNode *text = DFCreateTextNode(put->contentDoc,abstract->value);

            // Text inside a <w:del> element must be stored in a <w:delText> element
            // Text *not* inside a <w:del> element is stored in a <w:t> element
            Tag tag = WORD_T;
            for (DFNode *a = abstract->parent; a != NULL; a = a->parent) {
                if (a->tag == HTML_DEL)
                    tag = WORD_DELTEXT;
            }
            DFNode *t = DFCreateElement(put->contentDoc,tag);
            DFAppendChild(t,text);

            char *trimmed = DFStringTrimWhitespace(abstract->value);
            if (!DFStringEquals(trimmed,abstract->value))
                DFSetAttribute(t,XML_SPACE,"preserve");
            free(trimmed);

            return t;
        }
        case HTML_IMG:
            return WordDrawingCreate(put,abstract);
        case HTML_BR:
            return DFCreateElement(put->contentDoc,WORD_BR);
        case HTML_SPAN: {
            const char *className = DFGetAttribute(abstract,HTML_CLASS);
            if (DFStringEquals(className,DFTabClass))
                return DFCreateElement(put->contentDoc,WORD_TAB);
            return NULL;
        }
        default:
            return NULL;
    }
}

static void WordRunContentPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_DRAWING:
        case WORD_OBJECT:
        case WORD_PICT:
            WordDrawingPut(put,abstract,concrete);
            break;
    }
}

static void WordRunContentRemove(WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_DRAWING:
        case WORD_OBJECT:
        case WORD_PICT:
            WordDrawingRemove(put,concrete);
            break;
        case WORD_FOOTNOTEREFERENCE:
        case WORD_ENDNOTEREFERENCE:
            WordNoteReferenceRemove(put,concrete);
            break;
    }
}

WordLens WordRunContentLens = {
    .isVisible = WordRunContentIsVisible,
    .get = WordRunContentGet,
    .put = WordRunContentPut,
    .create = WordRunContentCreate,
    .remove = WordRunContentRemove,
};
