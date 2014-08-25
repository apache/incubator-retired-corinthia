// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "DFBuffer.h"
#include "DFCommon.h"

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
    int fd = open(filename,O_RDONLY);
    if (fd < 0) {
        DFErrorSetPosix(error,errno);
        return NULL;
    }
    DFBuffer *buf = DFBufferNew();
    long r;
    char temp[1024];
    while (0 < (r = read(fd,temp,1024)))
        DFBufferAppendData(buf,temp,r);
    close(fd);
    return buf;
}

int DFBufferWriteToFile(DFBuffer *buf, const char *filename, DFError **error)
{
    return DFWriteDataToFile(buf->data,buf->len,filename,error);
}

int DFWriteDataToFile(const void *data, size_t len, const char *filename, DFError **error)
{
    int fd = creat(filename,0644);
    if (fd < 0) {
        DFErrorSetPosix(error,errno);
        return 0;
    }
    size_t w = write(fd,data,len);
    if (w != len) {
        DFErrorFormat(error,"Incomplete write");
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}
