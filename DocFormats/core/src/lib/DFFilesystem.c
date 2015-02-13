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
#include "DFFilesystem.h"
#include "DFString.h"
#include "DFArray.h"
#include "DFBuffer.h"
#include <DocFormats/DFError.h>
#include "DFCharacterSet.h"
#include "DFPlatform.h"
#include "DFCommon.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>



int DFFileExists(const char *path)
{
    struct stat statbuf;
    return (0 == stat(path,&statbuf));
}

int DFIsDirectory(const char *path)
{
    struct stat statbuf;
    return ((0 == stat(path,&statbuf)) && S_ISDIR(statbuf.st_mode));
}

int DFCreateDirectory(const char *path, int intermediates, DFError **error)
{
    size_t len = strlen(path);
    for (size_t pos = 0; pos <= len; pos++) {
        if ((pos == len) || (intermediates && (path[pos] == '/'))) {
            if (pos == 0)
                continue;
            char *partial = DFSubstring(path,0,pos);
            char *errmsg = NULL;
            int ok = DFMkdirIfAbsent(partial,&errmsg);
            free(partial);
            if (!ok) {
                DFErrorFormat(error,"%s",errmsg);
                free(errmsg);
                return 0;
            }
        }
    }
    return 1;
}

int DFEmptyDirectory(const char *path, DFError **error)
{
    if (DFFileExists(path) && !DFDeleteFile(path,error))
        return 0;
    return DFCreateDirectory(path,1,error);
}

int DFCopyFile(const char *srcPath, const char *destPath, DFError **error)
{
    FILE *srcFile = fopen(srcPath,"rb");
    if (srcFile == NULL) {
        DFErrorSetPosix(error,errno);
        return 0;
    }

    FILE *destFile = fopen(destPath,"wb");
    if (destFile == NULL) {
        DFErrorSetPosix(error,errno);
        fclose(srcFile);
        return 0;
    }

    char buf[4096];
    size_t r;
    while ((r = fread(buf,1,4096,srcFile)) > 0)
        fwrite(buf,1,r,destFile);

    fclose(srcFile);
    fclose(destFile);
    return 1;
}

static int DFDeleteSingle(const char *path, DFError **error)
{
    struct stat statbuf;
    if (0 != stat(path,&statbuf)) {
        DFErrorSetPosix(error,errno);
        return 0;
    }
    if (S_ISDIR(statbuf.st_mode)) {
        if (0 != rmdir(path)) {
            DFErrorSetPosix(error,errno);
            return 0;
        }
    }
    else {
        if (0 != unlink(path)) {
            DFErrorSetPosix(error,errno);
            return 0;
        }
    }
    return 1;
}

int DFDeleteFile(const char *path, DFError **error)
{
    if (path == NULL)
        return 1;

    // Determine if path refers to a file or directory. If it does not exist, or is inaccessible,
    // an error will be returned.
    struct stat statbuf;
    if (0 != stat(path,&statbuf)) {
        DFErrorSetPosix(error,errno);
        return 0;
    }

    int ok = 1;
    if (S_ISDIR(statbuf.st_mode)) {
        const char **contents = DFContentsOfDirectory(path,1,error);
        if (contents == NULL)
            return 0;;

        size_t count = DFStringArrayCount(contents);

        for (size_t i = count; ok && (i > 0); i--) {
            const char *relPath = contents[i-1];
            char *fullPath = DFAppendPathComponent(path,relPath);
            if (!DFDeleteSingle(fullPath,error)) {
                DFErrorFormat(error,"%s: %s",relPath,DFErrorMessage(error));
                ok = 0;
            }
            free(fullPath);
        }
        free(contents);

        if (ok && !DFDeleteSingle(path,error))
            ok = 0;
    }
    else {
        if (0 != unlink(path)) {
            DFErrorSetPosix(error,errno);
            ok = 0;
        }
    }

    return ok;
}

static void freeDirEntryList(DFDirEntryList *list)
{
    DFDirEntryList *next;
    for (DFDirEntryList *l = list; l != NULL; l = next) {
        next = l->next;
        free(l->name);
        free(l);
    }
}

static DFArray *arrayFromDirEntryList(DFDirEntryList *list)
{
    DFArray *array = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (DFDirEntryList *l = list; l != NULL; l = l->next)
        DFArrayAppend(array,l->name);
    return array;
}

const char **DFContentsOfDirectory(const char *path, int recursive, DFError **error)
{
    DFDirEntryList *list = NULL;
    DFDirEntryList **listptr = &list;
    char *errmsg = NULL;

    if (!DFAddDirContents(path,"",recursive,&listptr,&errmsg)) {
        DFErrorFormat(error,"%s",errmsg);
        free(errmsg);
        freeDirEntryList(list);
        return NULL;
    }

    DFArray *array = arrayFromDirEntryList(list);
    const char **entries = DFStringArrayFlatten(array);
    DFArrayRelease(array);
    freeDirEntryList(list);
    return entries;
}

int DFPathContentsEqual(const char *path1, const char *path2)
{
    DFBuffer *content1 = DFBufferReadFromFile(path1,NULL);
    DFBuffer *content2 = DFBufferReadFromFile(path2,NULL);
    int r = ((content1 != NULL) && (content2 != NULL) &&
             (content1->len == content2->len) &&
             !memcmp(content1->data,content2->data,content1->len));
    DFBufferRelease(content1);
    DFBufferRelease(content2);
    return r;
}

char *DFPathDirName(const char *path)
{
    size_t len = strlen(path);
    if ((len > 0) && (path[len-1] == '/'))
        len--;;

    size_t pos = len;
    while (1) {
        if (pos == 0)
            return strdup("");
        if (path[pos-1] == '/') {
            if (pos == 1)
                return strdup("/");
            else
                return DFSubstring(path,0,pos-1);
        }
        pos--;
    }
}

char *DFPathBaseName(const char *path)
{
    size_t len = strlen(path);
    if ((len > 0) && (path[len-1] == '/'))
        len--;;

    size_t pos = len;
    while (1) {
        if (pos == 0)
            return DFSubstring(path,0,len);
        if (path[pos-1] == '/') {
            if (pos == 1)
                return DFSubstring(path,1,len);
            else
                return DFSubstring(path,pos,len);
        }
        pos--;
    }
}

char *DFPathExtension(const char *path)
{
    size_t len = strlen(path);
    if ((len > 0) && (path[len-1] == '/'))
        len--;;

    size_t pos = len;
    while (1) {
        if (pos == 0)
            return strdup("");
        if (path[pos-1] == '/')
            return strdup("");
        if (path[pos-1] == '.')
            return DFSubstring(path,pos,len);
        pos--;
    }
}

char *DFPathWithoutExtension(const char *path)
{
    size_t len = strlen(path);
    if ((len > 0) && (path[len-1] == '/'))
        len--;

    if (!strcmp(path,"/"))
        return strdup("/");;

    size_t pos = len;
    while (1) {
        if (pos == 0)
            return DFSubstring(path,0,len);
        if (path[pos-1] == '/')
            return DFSubstring(path,0,len);
        if (path[pos-1] == '.')
            return DFSubstring(path,0,pos-1);
        pos--;
    }
}

char *DFPathNormalize(const char *path)
{
    size_t len = strlen(path);
    char *result = (char *)malloc(len+1);
    size_t outpos = 0;
    for (size_t pos = 0; pos < len; pos++) {
        if ((path[pos] != '/') || (pos == 0) || (path[pos-1] != '/'))
            result[outpos++] = path[pos];
    }
    result[outpos] = '\0';
    return result;
}

char *DFPathResolveAbsolute(const char *rawBase, const char *relative)
{
    if (relative[0] == '/')
        return strdup(relative);

    char *base;
    if (rawBase[0] == '/') {
        base = strdup(rawBase);
    }
    else {
        char cwd[4096];
        getcwd(cwd,4096);
        base = DFFormatString("%s/%s",cwd,rawBase);
    }

    char *baseDirectory = DFStringHasSuffix(base,"/") ? strdup(base) : DFPathDirName(base);

    DFBuffer *path = DFBufferNew();
    DFBufferAppendString(path,baseDirectory);
    const char **relComponents = DFStringSplit(relative,"/",0);

    for (size_t i = 0; relComponents[i]; i++) {
        const char *adjust = relComponents[i];
        if (!strcmp(adjust,".") || !strcmp(adjust,"")) {
            // Do nothing
        }
        else if (!strcmp(adjust,"..")) {
            // Remove last component from path
            while ((path->len > 0) && (path->data[path->len-1] != '/'))
                path->len--;
            if (path->len > 0) {
                path->data[path->len-1] = '\0';
            }
        }
        else {
            // Add new component to path
            DFBufferAppendChar(path,'/');
            DFBufferAppendString(path,adjust);
        }
    }

    if (path->len == 0)
        DFBufferAppendChar(path,'/');

    char *result = DFPathNormalize(path->data);
    free(relComponents);
    free(base);
    free(baseDirectory);
    DFBufferRelease(path);
    return result;
}

static int numFromHex(uint32_t ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    else if ((ch >= 'a') && (ch <= 'f'))
        return 0xA + ch - 'a';
    else if ((ch >= 'A') && (ch <= 'F'))
        return 0xA + ch - 'A';
    else
        return -1;
}

char *DFRemovePercentEncoding(const char *encoded)
{
    size_t inlen = strlen(encoded);
    char *output = (char *)malloc(inlen+1);
    size_t outpos = 0;
    for (size_t inpos = 0; inpos < inlen; inpos++) {
        char ch = encoded[inpos];
        int hi;
        int lo;
        if ((ch == '%') &&
            (inpos+2 < inlen) &&
            ((hi = numFromHex(encoded[inpos+1])) >= 0) &&
            ((lo = numFromHex(encoded[inpos+2])) >= 0)) {
            ((unsigned char *)output)[outpos++] = (hi << 4) | lo;
            inpos += 2;
        }
        else {
            output[outpos++] = ch;
        }
    }
    assert(outpos <= inlen);
    output[outpos] = 0;
    return output;
}
