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

#include "ODFSheet.h"
#include "DFDOM.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ODFStyle                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

ODFStyle *ODFStyleNew()
{
    ODFStyle *style = (ODFStyle *)calloc(1,sizeof(ODFStyle));
    style->retainCount = 1;
    return style;
}

ODFStyle *ODFStyleRetain(ODFStyle *style)
{
    if (style != NULL)
        style->retainCount++;
    return style;
}

void ODFStyleRelease(ODFStyle *style)
{
    if ((style == NULL) || (--style->retainCount > 0))
        return;

    free(style->selector);
    free(style);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ODFSheet                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ODFSheet {
    size_t retainCount;
    DFDocument *stylesDoc;
    DFDocument *contentDoc;
    DFHashTable *stylesBySelector;
};

ODFSheet *ODFSheetNew(DFDocument *stylesDoc, DFDocument *contentDoc)
{
    ODFSheet *sheet = (ODFSheet *)calloc(1,sizeof(ODFSheet));
    sheet->retainCount = 1;
    sheet->stylesDoc = DFDocumentRetain(stylesDoc);
    sheet->contentDoc = DFDocumentRetain(contentDoc);
    sheet->stylesBySelector = DFHashTableNew((DFCopyFunction)ODFStyleRetain,(DFFreeFunction)ODFStyleRelease);
    return sheet;
}

ODFSheet *ODFSheetRetain(ODFSheet *sheet)
{
    if (sheet != NULL)
        sheet->retainCount++;
    return sheet;
}

void ODFSheetRelease(ODFSheet *sheet)
{
    if ((sheet == NULL) || (--sheet->retainCount > 0))
        return;

    DFDocumentRelease(sheet->stylesDoc);
    DFDocumentRelease(sheet->contentDoc);
    DFHashTableRelease(sheet->stylesBySelector);
    free(sheet);
}

ODFStyle *ODFSheetStyleForSelector(ODFSheet *sheet, const char *selector)
{
    if (selector == NULL)
        return NULL;
    return DFHashTableLookup(sheet->stylesBySelector,selector);
}
