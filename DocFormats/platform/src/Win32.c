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

// This file contains functions that are applicable to Windows

#ifdef WIN32

#include <windows.h>
#include <SDL_image.h>
#include <stdio.h>


static void DFErrorMsgSetWin32(char **errmsg, DWORD code)
{
    if (errmsg == NULL)
        return;

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
    *errmsg = strdup(lpMsgBuf);
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

static BOOL CALLBACK initOnceMutex(PINIT_ONCE initOnce,PVOID Parameter,PVOID *Context)
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

int DFMkdirIfAbsent(const char *path, char **errmsg)
{
    if (!CreateDirectory(path,NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) {
        DFErrorMsgSetWin32(errmsg,GetLastError());
        return 0;
    }
    return 1;
}

int DFAddDirContents(const char *absPath, const char *relPath, int recursive, DFDirEntryList ***list, char **errmsg)
{
    DFDirEntryList **listptr = *list;
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    size_t patternLen = strlen(absPath) + 2;
    char *pattern = (char *)malloc(patternLen+1);
    snprintf(pattern,patternLen+1,"%s/*",absPath);
    hFind = FindFirstFile(pattern,&ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        DFErrorMsgSetWin32(errmsg,GetLastError());
        free(pattern);
        return 0;
    }

    int ok = 1;
    do {
        if (!strcmp(ffd.cFileName,".") || !strcmp(ffd.cFileName,".."))
            continue;

        size_t absSubPathLen = strlen(absPath) + 1 + strlen(ffd.cFileName);
        size_t relSubPathLen = strlen(relPath) + 1 + strlen(ffd.cFileName);

        char *absSubPath = (char *)malloc(absSubPathLen+1);
        char *relSubPath = (char *)malloc(relSubPathLen+1);

        snprintf(absSubPath,absSubPathLen+1,"%s/%s",absPath,ffd.cFileName);
        snprintf(relSubPath,relSubPathLen+1,"%s/%s",relPath,ffd.cFileName);

        char *entryName;
        if (relSubPath[0] == '/')
            entryName = &relSubPath[1];
        else
            entryName = relSubPath;

        *listptr = (DFDirEntryList *)calloc(1,sizeof(DFDirEntryList));
        (*listptr)->name = strdup(entryName);
        listptr = &(*listptr)->next;

        if (recursive && (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            ok = DFAddDirContents(absSubPath,relSubPath,recursive,&listptr,errmsg);

        free(absSubPath);
        free(relSubPath);
    } while (ok && (FindNextFile(hFind,&ffd) != 0));

    FindClose(hFind);
    free(pattern);
    *list = listptr;
    return ok;
}

int DFGetImageDimensions(const void *data, size_t len, const char *ext,
                         unsigned int *width, unsigned int *height, char **errmsg)
{
    SDL_Surface *image = IMG_Load_RW(SDL_RWFromMem((void *)data,len),1);
    if (image == NULL) {
        if (errmsg != NULL)
            *errmsg = strdup(IMG_GetError());
        return 0;
    }

    *width = (unsigned int)image->w;
    *height = (unsigned int)image->h;
    SDL_FreeSurface(image);
    return 1;
}

#endif
