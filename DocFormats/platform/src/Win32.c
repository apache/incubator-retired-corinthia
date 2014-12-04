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

#include <windows.h>
#include <SDL_image.h>
#include <stdio.h>

void DFErrorSetWin32(DFError **error, DWORD code)
{
    char *lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpMsgBuf,
                  0, NULL);
    size_t len = strlen(lpMsgBuf);
    while ((len > 0) &&
           ((lpMsgBuf[len - 1] == '\n') ||
            (lpMsgBuf[len - 1] == '\r') ||
            (lpMsgBuf[len - 1] == '.')))
        len--;
    lpMsgBuf[len] = '\0';
    DFErrorFormat(error, "%s", lpMsgBuf);
    LocalFree(lpMsgBuf);
}

HANDLE onceMutex = NULL;

static int testAndSet(int *var,int value,HANDLE lock)
{
    WaitForSingleObject(lock,INFINITE);
    int oldValue = *var;
    *var = value;
    ReleaseMutex(lock);
    return oldValue;
}

BOOL CALLBACK initOnceMutex(PINIT_ONCE initOnce,PVOID Parameter,PVOID *Context)
{
    onceMutex = CreateMutex(NULL,FALSE,NULL);
    return TRUE;
}

void DFInitOnce(DFOnce *once, DFOnceFunction fun)
{
    static INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;
    InitOnceExecuteOnce(&initOnce,initOnceMutex,NULL,NULL);
    if (testAndSet(once,1,onceMutex) == 0)
        fun();
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

int DFGetImageDimensions(const void *data, size_t len, const char *ext,
                         unsigned int *width, unsigned int *height, DFError **error)
{
    SDL_Surface *image = IMG_Load_RW(SDL_RWFromMem((void *)data,len),1);
    if (image == NULL) {
        DFErrorFormat(error,"%s",IMG_GetError());
        return 0;
    }

    *width = (unsigned int)image->w;
    *height = (unsigned int)image->h;
    SDL_FreeSurface(image);
    return 1;
}

#endif
