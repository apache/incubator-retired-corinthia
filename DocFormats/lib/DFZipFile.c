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

static int zipError(DFError **error, const char *format, ...) ATTRIBUTE_FORMAT(printf,2,3);

static int zipError(DFError **error, const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    DFErrorVFormat(error,format,ap);
    va_end(ap);
    return 0;
}

int DFUnzip(const char *zipFilename, DFStore *store, DFError **error)
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

        char *outParentPath = DFPathDirName(entryName);
        if (!DFStoreExists(store,outParentPath)) {
            if (!DFStoreMkDir(store,outParentPath,1,error)) {
                free(outParentPath);
                return 0;
            }
        }
        free(outParentPath);

        if (DFStringHasSuffix(entryName,"/")) {
            // Directory
            if (!DFStoreExists(store,entryName) &&
                !DFStoreMkDir(store,entryName,0,error))
                return 0;
        }
        else {
            // File
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

            if (!DFBufferWriteToStore(content,store,entryName,error)) {
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

static int zipRecursive(zipFile zip, DFStore *store, const char *sourceRel, const char *dest, DFError **error)
{
    // FIXME: Not covered by tests
    if (!DFStoreExists(store,sourceRel))
        return zipError(error,"%s: No such file or directory",dest);
    if (DFStoreIsDir(store,sourceRel)) {
        const char **entries = DFStoreList(store,sourceRel,0,error);
        if (entries == NULL)
            return 0;
        int ok = 1;
        for (int i = 0; entries[i] && ok; i++) {
            const char *entry = entries[i];
            char *childSource = DFAppendPathComponent(sourceRel,entry);
            char *childDest = DFAppendPathComponent(dest,entry);
            if (!zipRecursive(zip,store,childSource,childDest,error))
                ok = 0;
            free(childSource);
            free(childDest);
        }
        free(entries);
        return ok;
    }
    else {
        DFBuffer *content = DFBufferReadFromStore(store,sourceRel,error);
        if (content == NULL) {
            return zipError(error,"%s: %s",sourceRel,DFErrorMessage(error));
        }

        int ok = zipAddFile(zip,dest,content,error);
        DFBufferRelease(content);
        return ok;
    }
}

int DFZip(const char *zipFilename, DFStore *store, DFError **error)
{
    zipFile zip = zipOpen(zipFilename,APPEND_STATUS_CREATE);
    if (zip == NULL)
        return zipError(error,"Cannot create file");

    int recursiveOK = zipRecursive(zip,store,"","",error);

    if (ZIP_OK != zipClose(zip,NULL))
        return zipError(error,"Cannot close file");
    return recursiveOK;
}
