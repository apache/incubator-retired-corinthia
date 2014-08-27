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

// This file contains functions that are applicable to all Unix-based platforms, including Linux, iOS, and OS X

#ifndef WIN32

int DFMkdirIfAbsent(const char *path, DFError **error)
{
    if ((mkdir(path,0777) != 0) && (errno != EEXIST)) {
        DFErrorSetPosix(error,errno);
        return 0;
    }
    return 1;
}

int DFAddDirContents(const char *absPath, const char *relPath, int recursive, DFArray *array, DFError **error)
{
    DIR *dir = opendir(absPath);
    if (dir == NULL) {
        DFErrorFormat(error,"%s: %s",relPath,strerror(errno));
        return 0;
    }

    struct dirent buffer;
    struct dirent *result = NULL;
    int ok = 1;
    while (ok && (0 == readdir_r(dir,&buffer,&result)) && (result != NULL)) {
        if (!strcmp(result->d_name,".") || !strcmp(result->d_name,".."))
            continue;

        char *absSubPath = DFAppendPathComponent(absPath,result->d_name);
        char *relSubPath = DFAppendPathComponent(relPath,result->d_name);

        if (relSubPath[0] == '/')
            DFArrayAppend(array,&relSubPath[1]);
        else
            DFArrayAppend(array,relSubPath);

        if (recursive && DFIsDirectory(absSubPath))
            ok = DFAddDirContents(absSubPath,relSubPath,recursive,array,error);

        free(absSubPath);
        free(relSubPath);
    }
    closedir(dir);
    return ok;
}

#endif
