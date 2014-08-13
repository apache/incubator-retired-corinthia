//
//  DFError.c
//  DocFormats
//
//  Created by Peter Kelly on 7/11/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#include "DFError.h"
#include "DFCommon.h"

struct DFError {
    size_t retainCount;
    char *message;
};

void DFErrorSetPosix(DFError **error, int code)
{
    DFErrorFormat(error,"%s",strerror(code));
}

void DFErrorVFormat(DFError **error, const char *format, va_list ap)
{
    if (error == NULL)
        return;

    va_list ap2;
    va_copy(ap2,ap);
    size_t nchars = vsnprintf(NULL,0,format,ap2);
    va_end(ap2);

    char *message = (char *)malloc(nchars+1);

    va_copy(ap2,ap);
    vsnprintf(message,nchars+1,format,ap2);
    va_end(ap2);

    if (*error != NULL) {
        // Error object exists; replace the existing message
        free((*error)->message);
        (*error)->message = message;
    }
    else {
        // Error object does not exist; create a new one
        (*error) = (DFError *)calloc(1,sizeof(DFError));
        (*error)->retainCount = 1;
        (*error)->message = message;
    }
}

void DFErrorFormat(DFError **error, const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    DFErrorVFormat(error,format,ap);
    va_end(ap);
}

DFError *DFErrorRetain(DFError *error)
{
    if (error == NULL)
        return NULL;
    error->retainCount++;
    return error;
}

void DFErrorRelease(DFError *error)
{
    if (error == NULL)
        return;
    assert(error->retainCount > 0);
    error->retainCount--;
    if (error->retainCount == 0) {
        free(error->message);
        free(error);
    }
}

const char *DFErrorMessage(DFError **error)
{
    if ((error == NULL) || (*error == NULL))
        return "Unknown error";
    else
        return (*error)->message;
}
