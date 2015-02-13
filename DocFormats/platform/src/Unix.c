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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

// This file contains functions that are applicable to all Unix-based platforms, including Linux, iOS, and OS X

#ifndef WIN32

#include <pthread.h>
#include <dirent.h>

static int testAndSet(int *var, int value, pthread_mutex_t *lock)
{
    pthread_mutex_lock(lock);
    int oldValue = *var;
    *var = value;
    pthread_mutex_unlock(lock);
    return oldValue;
}

void DFInitOnce(DFOnce *once, DFOnceFunction fun)
{
    static pthread_mutex_t onceLock = PTHREAD_MUTEX_INITIALIZER;
    if (testAndSet(once,1,&onceLock) == 0)
        fun();
}

int DFMkdirIfAbsent(const char *path, char **errmsg)
{
    if ((mkdir(path,0777) != 0) && (errno != EEXIST)) {
        printf("DFMkdirIfAbsent: errno = %d (%s)\n",errno,strerror(errno));
        if (errmsg != NULL)
            *errmsg = strdup(strerror(errno));
        return 0;
    }
    return 1;
}

int DFAddDirContents(const char *absPath, const char *relPath, int recursive, DFDirEntryList ***list, char **errmsg)
{
    DFDirEntryList **listptr = *list;

    DIR *dir = opendir(absPath);
    if (dir == NULL) {
        if (errmsg != NULL) {
            char temp[1024];
            snprintf(temp,1024,"%s: %s",relPath,strerror(errno));
            *errmsg = strdup(temp);
        }
        return 0;
    }

    struct dirent buffer;
    struct dirent *result = NULL;
    int ok = 1;
    while (ok && (0 == readdir_r(dir,&buffer,&result)) && (result != NULL)) {
        if (!strcmp(result->d_name,".") || !strcmp(result->d_name,".."))
            continue;

        size_t absSubPathLen = strlen(absPath) + 1 + strlen(result->d_name);
        size_t relSubPathLen = strlen(relPath) + 1 + strlen(result->d_name);

        char *absSubPath = (char *)malloc(absSubPathLen+1);
        char *relSubPath = (char *)malloc(relSubPathLen+1);

        snprintf(absSubPath,absSubPathLen+1,"%s/%s",absPath,result->d_name);
        snprintf(relSubPath,relSubPathLen+1,"%s/%s",relPath,result->d_name);

        char *entryName;
        if (relSubPath[0] == '/')
            entryName = &relSubPath[1];
        else
            entryName = relSubPath;

        *listptr = (DFDirEntryList *)calloc(1,sizeof(DFDirEntryList));
        (*listptr)->name = strdup(entryName);
        listptr = &(*listptr)->next;

        struct stat statbuf;
        if (recursive && (0 == stat(absSubPath,&statbuf)) && S_ISDIR(statbuf.st_mode))
            ok = DFAddDirContents(absSubPath,relSubPath,recursive,&listptr,errmsg);

        free(absSubPath);
        free(relSubPath);
    }
    closedir(dir);
    *list = listptr;
    return ok;
}

#endif
