//
//  ODFSheet.h
//  DocFormats
//
//  Created by Peter Kelly on 20/06/2014.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_ODFSheet_h
#define DocFormats_ODFSheet_h

#include "DFXMLForward.h"
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
