//
//  ODFSheet.c
//  DocFormats
//
//  Created by Peter Kelly on 20/06/2014.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#include "ODFSheet.h"
#include "DFDOM.h"
#include "DFHashTable.h"
#include "DFCommon.h"

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
