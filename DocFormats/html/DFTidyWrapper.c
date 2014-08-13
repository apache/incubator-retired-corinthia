//
//  DFTidyWrapper.c
//  DocFormats
//
//  Created by Peter Kelly on 24/09/13.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#include "DFTidyWrapper.h"
#include "DFHTDocument.h"
#include "DFCommon.h"

static int errorReturn(int rc, DFError **error)
{
    if (rc >= 0) {
        return 1;
    }
    else {
        DFErrorFormat(error,"Operation failed: error %d\n",-rc);
        return 0;
    }
}

static int DFHTDocumentParseData(DFHTDocument *htd, DFBuffer *data, DFError **error)
{
    TidyBuffer inbuf;
    tidyBufInit(&inbuf);
    tidyBufAttach(&inbuf,(byte*)data->data,(unsigned int)data->len);
    int rc = tidyParseBuffer(htd->doc,&inbuf);
    tidyBufDetach(&inbuf);
    tidyBufFree(&inbuf);

    return errorReturn(rc,error);
}

static int DFHTDocumentCleanAndRepair(DFHTDocument *htd, DFError **error)
{
    return errorReturn(tidyCleanAndRepair(htd->doc),error);
}

static int DFHTDocumentRunDiagnostics(DFHTDocument *htd, DFError **error)
{
    int rc = tidyRunDiagnostics(htd->doc);
    if (rc > 1)                                 // If error, force output.
        rc = ( tidyOptSetBool(htd->doc,TidyForceOutput,yes) ? rc : -1);
    return errorReturn(rc,error);
}

static int DFHTDocumentSaveBuffer(DFHTDocument *htd, DFBuffer *output, DFError **error)
{
    TidyBuffer outbuf;
    tidyBufInit(&outbuf);
    int rc = tidySaveBuffer(htd->doc,&outbuf);      // Pretty Print
    if (rc >= 0) {
        DFBufferAppendData(output,(const void *)outbuf.bp,outbuf.size);
        tidyBufFree(&outbuf);
        return 1;
    }
    else {
        tidyBufFree(&outbuf);
        DFErrorFormat(error,"Pretty printing failed: error %d",-rc);
        return 0;
    }
}

static int DFHTMLTidy2(DFBuffer *input, DFBuffer *output, int xHTML, DFError **error, DFHTDocument *document)
{
    if (!DFHTDocumentParseData(document,input,error))
        return 0;
    DFHTDocumentRemoveUXWriteSpecial(document);
    if (!DFHTDocumentCleanAndRepair(document,error))
        return 0;
    if (!DFHTDocumentRunDiagnostics(document,error))
        return 0;
    if (!DFHTDocumentSaveBuffer(document,output,error))
        return 0;
    return 1;
}

int DFHTMLTidy(DFBuffer *input, DFBuffer *output, int xHTML, DFError **error)
{
    DFHTDocument *htd = DFHTDocumentNew();
    int result = DFHTMLTidy2(input,output,xHTML,error,htd);
    DFHTDocumentFree(htd);
    return result;
}
