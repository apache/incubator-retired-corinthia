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

#include "DFCommon.h"
#include "DFPlatform.h"
#include "DFFilesystem.h"
#include "DFString.h"

// This file contains functions that are applicable to Windows

#ifdef WIN32

static BOOL CALLBACK DFInitOnceWrapper(PINIT_ONCE InitOnce,void *p,void *c)
{
    ((DFOnceFunction)p)();
    return 1;
}

void DFInitOnce(DFOnce *once, DFOnceFunction fun)
{
    InitOnceExecuteOnce(once,DFInitOnceWrapper,fun,NULL);
}

int DFMkdirIfAbsent(const char *path,DFError **error)
{
    if (!CreateDirectory(path,NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
        DFErrorSetWin32(error,GetLastError());
        return 0;
    }
    return 1;
}

int DFAddDirContents(const char *absPath, const char *relPath, int recursive, DFArray *array, DFError **error)
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char *pattern = DFFormatString("%s/*",absPath);
    hFind = FindFirstFile(pattern,&ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        DFErrorSetWin32(error,GetLastError());
        free(pattern);
        return 0;
    }

    int ok = 1;
    do {
        if (!strcmp(ffd.cFileName,".") || !strcmp(ffd.cFileName,".."))
            continue;

        char *absSubPath = DFAppendPathComponent(absPath,ffd.cFileName);
        char *relSubPath = DFAppendPathComponent(relPath,ffd.cFileName);

        if (relSubPath[0] == '/')
            DFArrayAppend(array,&relSubPath[1]);
        else
            DFArrayAppend(array,relSubPath);

        if (recursive && (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            ok = DFAddDirContents(absSubPath,relSubPath,recursive,array,error);

        free(absSubPath);
        free(relSubPath);
    } while (ok && (FindNextFile(hFind,&ffd) != 0));

    FindClose(hFind);
    free(pattern);
    return ok;
}

int DFGetImageDimensions(const char *path, unsigned int *width, unsigned int *height, DFError **error)
{
    printf("WARNING: DFGetImageDimensions is not implemented on Windows\n");
    DFErrorFormat(error,"DFGetImageDimensions is not implemented on Windows");
    return 0;
}

#endif
