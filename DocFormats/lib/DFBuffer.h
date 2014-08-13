//
//  DFBuffer.h
//  DocFormats
//
//  Created by Peter Kelly on 2/03/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFBuffer_h
#define DocFormats_DFBuffer_h

#include "DFError.h"
#include "DFTypes.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFBuffer                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DFBuffer DFBuffer;

struct DFBuffer {
    size_t retainCount;
    size_t alloc;
    size_t len;
    char *data;
};

DFBuffer *DFBufferNew(void);
DFBuffer *DFBufferRetain(DFBuffer *buf);
void DFBufferRelease(DFBuffer *buf);

void DFBufferAppendData(DFBuffer *buf, const char *data, size_t len);
void DFBufferAppendString(DFBuffer *buf, const char *data);
void DFBufferAppendChar(DFBuffer *buf, char ch);
void DFBufferVFormat(DFBuffer *buf, const char *format, va_list ap);
void DFBufferFormat(DFBuffer *buf, const char *format, ...) __attribute__((format(printf,2,3)));
DFBuffer *DFBufferReadFromFile(const char *filename, DFError **error);
int DFBufferWriteToFile(DFBuffer *buf, const char *filename, DFError **error);

int DFWriteDataToFile(const void *data, size_t len, const char *filename, DFError **error);

#endif
