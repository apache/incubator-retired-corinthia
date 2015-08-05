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
	unsigned char   *buf;
    DFextZipHandleP  zipHandle;
	int              i;

    zipHandle = DFextZipOpen(zipFilename);
    if (!zipHandle)
      return zipError(error,"Cannot open file");

	for (i = 0; i < zipHandle->zipFileCount; i++) {
		if ( (buf = DFextZipReadFile(zipHandle, &zipHandle->zipFileEntries[i])) == NULL)
			return zipError(error, "Cannot read file in zip");

		DFBuffer *content = DFBufferNew();
		DFBufferAppendData(content, (void *)buf, zipHandle->zipFileEntries[i].uncompressedSize);
		free(buf);

		if (!DFBufferWriteToStorage(content, storage, zipHandle->zipFileEntries[i].fileName, error)) {
			DFBufferRelease(content);
			return zipError(error, "%s: %s", zipHandle->zipFileEntries[i].fileName, DFErrorMessage(error));
		}
		DFBufferRelease(content);
	}

    DFextZipClose(zipHandle);

    return 1;
}

static int zipAddFile(DFextZipHandleP zipHandle, const char *dest, DFBuffer *content, DFError **error)
{
    if (DFextZipWriteFile(zipHandle, dest, content->data, content->len) < 0)
        return zipError(error,"%s: Cannot create entry in zip file",dest);
    return 1;
}



int DFZip(const char *zipFilename, DFStorage *storage, DFError **error)
{
    const char **allPaths = NULL;
    DFBuffer *content = NULL;
    int ok = 0;
    DFextZipHandleP zipHandle = NULL;

    allPaths = DFStorageList(storage,error);
    if (allPaths == NULL || !(zipHandle = DFextZipCreate(zipFilename)))
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
