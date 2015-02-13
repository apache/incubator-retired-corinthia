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
#include "DFBuffer.h"
#include "DFCommon.h"
#include "DFString.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFBuffer                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFBuffer *DFBufferNew(void)
{
    DFBuffer *buf = (DFBuffer *)calloc(1,sizeof(DFBuffer));
    buf->retainCount = 1;
    buf->alloc = 1;
    buf->len = 0;
    buf->data = (char *)malloc(buf->alloc*sizeof(char));
    buf->data[0] = '\0';
    return buf;
}

DFBuffer *DFBufferRetain(DFBuffer *buf)
{
    if (buf == NULL)
        return NULL;
    buf->retainCount++;
    return buf;
}

void DFBufferRelease(DFBuffer *buf)
{
    if (buf == NULL)
        return;
    assert(buf->retainCount > 0);
    buf->retainCount--;
    if (buf->retainCount == 0) {
        free(buf->data);
        free(buf);
    }
}

void DFStringBufferEnsureSpace(DFBuffer *buf, size_t len)
{
    size_t want = buf->len + len + 1;
    if (buf->alloc < want) {
        while (buf->alloc < want)
            buf->alloc *= 2;
        buf->data = (char *)realloc(buf->data,buf->alloc);
    }
}

void DFBufferAppendData(DFBuffer *buf, const char *data, size_t len)
{
    if (buf == NULL)
        return;
    DFStringBufferEnsureSpace(buf,len);
    memcpy(&buf->data[buf->len],data,len);
    buf->data[buf->len + len] = '\0';
    buf->len += len;
}

void DFBufferAppendString(DFBuffer *buf, const char *data)
{
    DFBufferAppendData(buf,data,strlen(data));
}

void DFBufferAppendChar(DFBuffer *buf, char ch)
{
    DFBufferAppendData(buf,&ch,1);
}

void DFBufferVFormat(DFBuffer *buf, const char *format, va_list ap)
{
    if (buf == NULL) {
        return;
    }

    va_list ap2;
    va_copy(ap2,ap);
    size_t nchars = vsnprintf(NULL,0,format,ap2);
    va_end(ap2);

    DFStringBufferEnsureSpace(buf,nchars);

    va_copy(ap2,ap);
    vsnprintf(&buf->data[buf->len],nchars+1,format,ap2);
    va_end(ap2);

    buf->len += nchars;
}

void DFBufferFormat(DFBuffer *buf, const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    DFBufferVFormat(buf,format,ap);
    va_end(ap);
}

DFBuffer *DFBufferReadFromFile(const char *filename, DFError **error)
{
    FILE *file = fopen(filename,"rb");
    if (file == NULL) {
        DFErrorSetPosix(error,errno);
        return NULL;
    }
    DFBuffer *buf = DFBufferNew();
    size_t r;
    char temp[1024];
    while (0 < (r = fread(temp,1,1024,file)))
        DFBufferAppendData(buf,temp,r);
    fclose(file);
    return buf;
}

DFBuffer *DFBufferReadFromStorage(DFStorage *storage, const char *filename, DFError **error)
{
    void *data = 0;
    size_t len = 0;
    if (!DFStorageRead(storage,filename,&data,&len,error))
        return NULL;;
    DFBuffer *r = DFBufferNew();;
    DFBufferAppendData(r,data,len);
    free(data);
    return r;
}

int DFBufferWriteToStorage(DFBuffer *buf, DFStorage *storage, const char *filename, DFError **error)
{
    return DFStorageWrite(storage,filename,buf->data,buf->len,error);
}

int DFBufferWriteToFile(DFBuffer *buf, const char *filename, DFError **error)
{
    return DFWriteDataToFile(buf->data,buf->len,filename,error);
}

int DFWriteDataToFile(const void *data, size_t len, const char *filename, DFError **error)
{
    FILE *file = fopen(filename,"wb");
    if (file == NULL) {
        DFErrorSetPosix(error,errno);
        return 0;
    }
    size_t w = fwrite(data,1,len,file);
    if (w != len) {
        DFErrorFormat(error,"Incomplete write");
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

// This file isn't really a great place for binaryToString and stringToBinary, but they needed
// to go somewhere after being moved into the DocFormats library from dfutil. At the time of
// writing, they're only used for by the test functions - perhaps we can have a TestLib.c file
// or similar?

char *binaryToString(DFBuffer *input)
{
    const char *hexchars = "0123456789ABCDEF";
    DFBuffer *charBuf = DFBufferNew();
    for (size_t pos = 0; pos < input->len; pos++) {
        if ((pos > 0) && (pos % 40 == 0))
            DFBufferAppendChar(charBuf,'\n');
        unsigned char hi = ((unsigned char *)input->data)[pos] >> 4;
        unsigned char lo = ((unsigned char *)input->data)[pos] & 0x0F;
        DFBufferAppendChar(charBuf,hexchars[hi]);
        DFBufferAppendChar(charBuf,hexchars[lo]);
    }
    if ((input->len % 40) != 0)
        DFBufferAppendChar(charBuf,'\n');
    char *result = strdup(charBuf->data);

    DFBufferRelease(charBuf);
    return result;
}

DFBuffer *stringToBinary(const char *str)
{
    size_t length = strlen(str);
    DFBuffer *outbuf = DFBufferNew();

    int wantHi = 1;
    unsigned char hi = 0;

    for (size_t inpos = 0; inpos < length; inpos++) {
        char c = str[inpos];
        unsigned char nibble = 0;

        if ((c >= '0') && (c <= '9'))
            nibble = c - '0';
        else if ((c >= 'a') && (c <= 'f'))
            nibble = 10 + (c - 'a');
        else if ((c >= 'A') && (c <= 'F'))
            nibble = 10 + (c - 'A');
        else
            continue;

        if (wantHi)
            hi = nibble << 4;
        else
            DFBufferAppendChar(outbuf,hi | nibble);
        wantHi = !wantHi;
    }
    
    return outbuf;
}
