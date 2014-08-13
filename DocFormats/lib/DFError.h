//
//  DFError.h
//  DocFormats
//
//  Created by Peter Kelly on 7/11/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_DFError_h
#define DocFormats_DFError_h

#include "DFTypes.h"

typedef struct DFError DFError;

void DFErrorSetPosix(DFError **error, int code);
void DFErrorVFormat(DFError **error, const char *format, va_list ap);
void DFErrorFormat(DFError **error, const char *format, ...) __attribute__((format(printf,2,3)));
DFError *DFErrorRetain(DFError *error);
void DFErrorRelease(DFError *error);
const char *DFErrorMessage(DFError **error);

#endif
