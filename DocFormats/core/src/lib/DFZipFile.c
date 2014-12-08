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

#include "DFZipFile.h"
#include "unzip.h"
#include "zip.h"
#include "DFFilesystem.h"
#include "DFString.h"
#include "DFCommon.h"
#include "DFBuffer.h"
#include <stdlib.h>
#include <string.h>

static int zipError(DFError **error, const char *format, ...) ATTRIBUTE_FORMAT(printf,2,3);

static int zipError(DFError **error, const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    DFErrorVFormat(error,format,ap);
    va_end(ap);
    return 0;
}

int DFUnzip(const char *zipFilename, DFStorage *storage, DFError **error)
{
    unzFile zipFile = unzOpen(zipFilename);
    if (zipFile == NULL)
        return zipError(error,"Cannot open file");

    int ret;
    for (ret = unzGoToFirstFile(zipFile); ret == UNZ_OK; ret = unzGoToNextFile(zipFile)) {
        unz_file_info info;
        char entryName[4096];
        if (UNZ_OK != unzGetCurrentFileInfo(zipFile,&info,entryName,4096,NULL,0,NULL,0))
            return zipError(error,"Zip directory is corrupt");

        if (!DFStringHasSuffix(entryName,"/")) {
            // Regular file
            if (UNZ_OK != unzOpenCurrentFile(zipFile))
                return zipError(error,"%s: Cannot open zip entry",entryName);;

            DFBuffer *content = DFBufferNew();

            unsigned char buf[4096];
            int r;
            while (0 < (r = unzReadCurrentFile(zipFile,buf,4096)))
                DFBufferAppendData(content,(void *)buf,r);
            if (0 > r) {
                DFBufferRelease(content);
                return zipError(error,"%s: decompression failed",entryName);
            }

            if (UNZ_OK != unzCloseCurrentFile(zipFile)) {
                DFBufferRelease(content);
                return zipError(error,"%s: decompression failed",entryName);
            }

            if (!DFBufferWriteToStorage(content,storage,entryName,error)) {
                DFBufferRelease(content);
                return zipError(error,"%s: %s",entryName,DFErrorMessage(error));
            }
            DFBufferRelease(content);
        }
    }

    if (UNZ_END_OF_LIST_OF_FILE != ret)
        return zipError(error,"Zip directory is corrupt");

    if (UNZ_OK != unzClose(zipFile))
        return zipError(error,"Cannot close file");

    return 1;
}

static int zipAddFile(zipFile zip, const char *dest, DFBuffer *content, DFError **error)
{
    zip_fileinfo fileinfo;
    bzero(&fileinfo,sizeof(fileinfo));

    if (ZIP_OK != zipOpenNewFileInZip(zip,
                                      dest,
                                      &fileinfo,
                                      NULL,0,
                                      NULL,0,
                                      NULL,
                                      Z_DEFLATED,
                                      Z_DEFAULT_COMPRESSION)) {
        return zipError(error,"%s: Cannot create entry in zip file",dest);
    }

    if (ZIP_OK != zipWriteInFileInZip(zip,content->data,(unsigned int)content->len))
        return zipError(error,"%s: Error writing to entry in zip file",dest);


    if (ZIP_OK != zipCloseFileInZip(zip))
        return zipError(error,"%s: Error closing entry in zip file",dest);

    return 1;
}

int DFZip(const char *zipFilename, DFStorage *storage, DFError **error)
{
    const char **allPaths = NULL;
    zipFile zip = NULL;
    DFBuffer *content = NULL;
    int ok = 0;

    allPaths = DFStorageList(storage,error);
    if (allPaths == NULL)
        goto end;

    zip = zipOpen(zipFilename,APPEND_STATUS_CREATE);
    if (zip == NULL) {
        DFErrorFormat(error,"Cannot create file");
        goto end;
    }

    for (int i = 0; allPaths[i]; i++) {
        const char *path = allPaths[i];

        DFBufferRelease(content);
        content = DFBufferReadFromStorage(storage,path,error);
        if (content == NULL) {
            DFErrorFormat(error,"%s: %s",path,DFErrorMessage(error));
            goto end;
        }

        if (!zipAddFile(zip,path,content,error))
            goto end;
    }

    ok = 1;

end:
    DFBufferRelease(content);
    free(allPaths);
    if ((zip != NULL) && (ZIP_OK != zipClose(zip,NULL)))
        return zipError(error,"Cannot close file");
    return ok;
}
