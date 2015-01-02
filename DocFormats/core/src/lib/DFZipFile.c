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

#include "DFPlatform.h"
#include "DFZipFile.h"
#include "DFPlatform.h"
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
    char            entryName[4096];
    DFextZipHandleP zipHandle;

    zipHandle = DFextZipOpen(zipFilename, 1);
    if (!zipHandle)
      return zipError(error,"Cannot open file");

    int ret;
    for (; (ret = DFextZipOpenNextFile(zipHandle, entryName, sizeof(entryName))) > 0;) {
        DFBuffer *content = DFBufferNew();

        unsigned char buf[4096];
        int r;
        while (0 < (r = DFextZipReadCurrentFile(zipHandle, buf, sizeof(buf))))
            DFBufferAppendData(content,(void *)buf,r);
        if (0 > r) {
            DFBufferRelease(content);
            return zipError(error,"%s: decompression failed",entryName);
        }

        if (DFextZipCloseFile(zipHandle) < 0) {
            DFBufferRelease(content);
            return zipError(error,"%s: decompression failed",entryName);
        }

        if (!DFBufferWriteToStorage(content,storage,entryName,error)) {
            DFBufferRelease(content);
            return zipError(error,"%s: %s",entryName,DFErrorMessage(error));
        }
        DFBufferRelease(content);
    }

    if (ret < 0)
        return zipError(error,"Zip directory is corrupt");

    DFextZipClose(zipHandle);

    return 1;
}

static int zipAddFile(DFextZipHandleP zipHandle, const char *dest, DFBuffer *content, DFError **error)
{
    if (DFextZipAppendNewFile(zipHandle, dest) < 0)
        return zipError(error,"%s: Cannot create entry in zip file",dest);

    if (DFextZipWriteCurrentFile(zipHandle, content->data, (unsigned int)content->len) < 0)
        return zipError(error,"%s: Error writing to entry in zip file",dest);

    if (DFextZipCloseFile(zipHandle) <0)
        return zipError(error,"%s: Error closing entry in zip file",dest);
    return 1;
}



int DFZip(const char *zipFilename, DFStorage *storage, DFError **error)
{
    const char **allPaths = NULL;
    DFBuffer *content = NULL;
    int ok = 0;
    DFextZipHandleP zipHandle = NULL;

    allPaths = DFStorageList(storage,error);
    if (allPaths == NULL || !(zipHandle = DFextZipOpen(zipFilename, 0)))
    {
      DFErrorFormat(error,"Cannot create file");
    }
    else
    {
      for (int i = 0; allPaths[i]; i++) {
          const char *path = allPaths[i];

          DFBufferRelease(content);
          content = DFBufferReadFromStorage(storage,path,error);
          if (content == NULL) {
              DFErrorFormat(error,"%s: %s",path,DFErrorMessage(error));
              goto end;
          }

          if (!zipAddFile(zipHandle, path, content, error))
              goto end;
      }

      ok = 1;
    }

end:
    DFBufferRelease(content);
    free(allPaths);
    if (zipHandle != NULL)
        DFextZipClose(zipHandle);
    return ok;
}
