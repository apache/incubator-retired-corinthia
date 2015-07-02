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
#include "DFHTML.h"
#include "DFNameMap.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        ODFParagraphLens                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *ODFParagraphCreateAbstractNode(ODFGetData *get, DFNode *concrete)
{
    DFNode *abstract = NULL;
    abstract = ODFConverterCreateAbstract(get, HTML_P, concrete);
    const char * styleName = DFGetAttribute(concrete,TEXT_STYLE_NAME);
    if(styleName != NULL) {
        printf("Found style name %s\n", styleName);
        DFSetAttribute(abstract, HTML_CLASS, styleName); //DFGetAttribute(odfNode,TEXT_STYLE_NAME));
    }
    return abstract;
}

/**
 * Map to an HTML_P
 * and treat as a paragraph
 */
static DFNode *ODFParagraphGet(ODFGetData *get, DFNode *concrete)
{
    printf("ODFParagraphGet look at the content\n");

    //DFNode *abstract = ODFParagraphCreateAbstractNode(get,concrete);
    DFNode *abstract = ODFParagraphCreateAbstractNode(get, concrete);
    ODFContainerGet(get,&ODFParagraphContentLens,abstract,concrete);

    return abstract;
}

static void ODFParagraphPut(ODFPutData *put, DFNode *abstract, DFNode *concrete)
{
    if (!HTML_isParagraphTag(abstract->tag) && (abstract->tag != HTML_FIGURE))
        return;

    if (concrete->tag != TEXT_P)
        return;

    ODFContainerPut(put,&ODFParagraphContentLens,abstract,concrete);
}

static DFNode *ODFParagraphCreate(ODFPutData *put, DFNode *abstract)
{
    DFNode *concrete = DFCreateElement(put->contentDoc,TEXT_P);
    ODFParagraphPut(put,abstract,concrete);
    return concrete;
}

static int ODFParagraphIsVisible(ODFPutData *put, DFNode *concrete)
{
    return 1;
}

static void ODFParagraphRemove(ODFPutData *put, DFNode *concrete)
{
    for (DFNode *child = concrete->first; child != NULL; child = child->next)
        ODFParagraphContentLens.remove(put,child);
}

ODFLens ODFParagraphLens = {
    .isVisible = ODFParagraphIsVisible,
    .get = ODFParagraphGet,
    .put = ODFParagraphPut,
    .create = ODFParagraphCreate,
    .remove = ODFParagraphRemove,
};
