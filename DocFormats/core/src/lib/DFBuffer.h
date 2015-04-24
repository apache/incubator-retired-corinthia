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

#ifndef DocFormats_DFBuffer_h
#define DocFormats_DFBuffer_h

#include <DocFormats/DFError.h>
#include "DFTypes.h"
#include <DocFormats/DFStorage.h>
#include <stdarg.h>

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
void DFBufferFormat(DFBuffer *buf, const char *format, ...) ATTRIBUTE_FORMAT(printf,2,3);
DFBuffer *DFBufferReadFromFile(const char *filename, DFError **error);
DFBuffer *DFBufferReadFromStorage(DFStorage *storage, const char *filename, DFError **error);
int DFBufferWriteToFile(DFBuffer *buf, const char *filename, DFError **error);
int DFBufferWriteToStorage(DFBuffer *buf, DFStorage *storage, const char *filename, DFError **error);

int DFWriteDataToFile(const void *data, size_t len, const char *filename, DFError **error);

char *binaryToString(DFBuffer *input);
DFBuffer *stringToBinary(const char *str);

#endif
