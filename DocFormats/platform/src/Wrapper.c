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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "DFPlatform.h"
#include "unzip.h"
#include "zip.h"


DFextZipHandleP DFextZipOpen(const char *zipFilename, int doUnzip) {
    DFextZipHandleP zipHandle = xmalloc(sizeof(DFextZipHandle));
 
    // no more memory
    if (!zipHandle)
        return NULL;

    // Open file
    zipHandle->zipFirst = 1;
    zipHandle->zipFlag = doUnzip;
    if (doUnzip)
        zipHandle->handle = unzOpen(zipFilename);
    else
        zipHandle->handle = zipOpen(zipFilename, APPEND_STATUS_CREATE);

    if (zipHandle->handle)
        return zipHandle;

    free(zipHandle);
    return NULL;
}



int DFextZipClose(DFextZipHandleP zipHandle)
{
    int rc = 0;

    if (zipHandle->handle) {
        if (zipHandle->zipFlag)
            rc = (unzClose(zipHandle->handle) == UNZ_OK);
        else
            rc = (zipClose(zipHandle->handle, NULL) == ZIP_OK);
        zipHandle->handle = NULL;
    }

    free(zipHandle);
    return rc ? 1 : -1;
}



int DFextZipOpenNextFile(DFextZipHandleP zipHandle, char *entryName, const int maxName)
{
    int rc;


    if (zipHandle->zipFlag) {
        unz_file_info info;

        // handling of first file and all others are different
        if (zipHandle->zipFirst) {
            rc = unzGoToFirstFile(zipHandle->handle);
            zipHandle->zipFirst = 0;
        }
        else
            rc = unzGoToNextFile(zipHandle->handle);

        // Error or past last file
        if (rc != UNZ_OK)
            return (rc == UNZ_END_OF_LIST_OF_FILE) ? 0 : -1;

        // get file name
        if (unzGetCurrentFileInfo(zipHandle->handle, &info, entryName, maxName, NULL, 0, NULL, 0) != UNZ_OK)
            return -1;

        // check for prefix "/" and if present skip file
        if (entryName[strlen(entryName) - 1] == '/')
            return DFextZipOpenNextFile(zipHandle, entryName, maxName);

        // open Regular file
        if (unzOpenCurrentFile(zipHandle->handle) != UNZ_OK)
            return -1;
    }
    else {
        return -1; // Zip file is open in write-only mode
    }

    // ready to read
    return 1;
}

int DFextZipAppendNewFile(DFextZipHandleP zipHandle, const char *entryName)
{
    zip_fileinfo fileinfo;
    memset(&fileinfo, 0, sizeof(fileinfo));

    if (zipHandle->zipFlag)
        return -1; // Zip file is open in read-only mode

    if (zipOpenNewFileInZip(zipHandle->handle,
                            entryName,
                            &fileinfo,
                            NULL, 0,
                            NULL, 0,
                            NULL,
                            Z_DEFLATED,
                            Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        return -1;
    }

    return 1;
}

int DFextZipCloseFile(DFextZipHandleP zipHandle)
{
    if (zipHandle->zipFlag)
        return (unzCloseCurrentFile(zipHandle->handle) != UNZ_OK) ? -1 : 1;
    else
        return (zipCloseFileInZip(zipHandle->handle) != UNZ_OK) ? -1 : 1;
}


 

int DFextZipReadCurrentFile(DFextZipHandleP zipHandle, void *buf, const int maxLen)
{
    return unzReadCurrentFile(zipHandle->handle, buf, maxLen);
}



int DFextZipWriteCurrentFile(DFextZipHandleP zipHandle, const void *buf, const int len)
{
    return (zipWriteInFileInZip(zipHandle->handle, buf, len) == ZIP_OK) ? 1 : -1;
}



void *xmalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL) {
        perror("xmalloc: out of memory.\n");
        abort();
        return NULL;
    }

    return ptr;
}



void *xcalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);

    if (ptr == NULL) {
        perror("xcalloc: out of memory.\n");
        abort();
        return NULL;
    }

    return ptr;
}



void *xrealloc(void *in_ptr, size_t size)
{
    void *ptr = realloc(in_ptr, size);

    if (ptr == NULL) {
        perror("xrealloc: out of memory.\n");
        abort();
        return NULL;
    }

    return ptr;
}
