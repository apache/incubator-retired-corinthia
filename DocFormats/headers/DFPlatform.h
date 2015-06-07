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

#pragma once

#ifdef _WINDOWS
#include <direct.h>

#define _CRT_SECURE_NO_WARNINGS
#define snprintf _snprintf
#define strcasecmp _stricmp
#define mktemp _mktemp
#define mkdir  _mkdir
#define bzero(mem,size) memset(mem,0,size)

#ifndef snprintf
#define snprintf _snprintf
#endif // snprintf

#else
#include <unistd.h>
#endif // _WINDOWS

#ifndef ATTRIBUTE_FORMAT
#ifdef _MSC_VER
#define ATTRIBUTE_FORMAT(archetype,index,first)
#define ATTRIBUTE_ALIGNED(n) __declspec(align(8))
#else
#define ATTRIBUTE_FORMAT(archetype,index,first) __attribute__((format(archetype,index,first)))
#define ATTRIBUTE_ALIGNED(n) __attribute__((aligned (n)))
#endif // _MSC_VER
#endif // ATTRIBUTE_FORMAT

typedef struct DFDirEntryList DFDirEntryList;

struct DFDirEntryList {
    char *name;
    DFDirEntryList *next;
};

int DFMkdirIfAbsent(const char *path, char **errmsg);

int DFAddDirContents(const char *absPath, const char *relPath, 
                     int recursive, DFDirEntryList ***list, 
                     char **errmsg);

int DFGetImageDimensions(const void *data, size_t len, 
                         const char *ext, unsigned int *width, 
                         unsigned int *height, char **errmsg);

#define DF_ONCE_INIT 0
typedef int DFOnce;
typedef void (*DFOnceFunction)(void);

void DFInitOnce(DFOnce *once, DFOnceFunction fun);

// Zip functions
typedef struct {
        void *handle;
        int   zipFlag;
        int   zipFirst;
        } DFextZipHandle;

typedef DFextZipHandle * DFextZipHandleP;

DFextZipHandleP DFextZipOpen(const char *zipFilename, int doUnzip);

int DFextZipClose(DFextZipHandleP zipHandle);

int DFextZipOpenNextFile(DFextZipHandleP zipHandle,
                         char *entryName,
                         const int maxName);

int DFextZipAppendNewFile(DFextZipHandleP zipHandle,
                          const char *entryName);

int DFextZipCloseFile(DFextZipHandleP zipHandle);

int DFextZipReadCurrentFile(DFextZipHandleP zipHandle,
                            void *buf,
                            const int maxLen);

int DFextZipWriteCurrentFile(DFextZipHandleP zipHandle,
                             const void *buf,
                             const int len);

void *xmalloc(size_t size);

void *xcalloc(size_t nmemb, size_t size);

void *xrealloc(void *ptr, size_t size);

char *xstrdup(const char *s);
