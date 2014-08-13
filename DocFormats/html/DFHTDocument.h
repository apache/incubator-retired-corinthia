//
//  DFHTDocument.h
//  DocFormats
//
//  Created by Peter Kelly on 6/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFHTDocument_h
#define DocFormats_DFHTDocument_h

#include "tidy.h"
#include "buffio.h"
#include "DFError.h"
#include "DFBuffer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          DFHTDocument                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DFHTDocument DFHTDocument;

struct DFHTDocument {
    TidyBuffer errbuf;
    TidyDoc doc;
};

DFHTDocument *DFHTDocumentNew();
void DFHTDocumentFree(DFHTDocument *htd);
int DFHTDocumentParseCString(DFHTDocument *htd, const char *str, DFError **error);
void DFHTDocumentRemoveUXWriteSpecial(DFHTDocument *htd);

#endif
