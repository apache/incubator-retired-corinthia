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

#include "Word.h"
#include "WordStyles.h"
#include "DFHTML.h"
#include "DFFilesystem.h"
#include "CSSProperties.h"
#include "DFString.h"
#include "DFCommon.h"
#include "DFZipFile.h"
#include <stdlib.h>

int DFHTMLToWord(const char *sourcePath, const char *destPath, DFError **error)
{
    int ok = 0;
    char *idPrefix = DFFormatString("bdt%u-",(unsigned int)rand());
    char *htmlPath = DFPathDirName(sourcePath);
    DFDocument *htmlDoc = NULL;
    DFBuffer *warnings = DFBufferNew();
    DFPackage *rawPackage = NULL;
    WordPackage *wordPackage = NULL;

    htmlDoc = DFParseHTMLFile(sourcePath,0,error);
    if (htmlDoc == NULL) {
        char *sourceFilename = DFPathBaseName(sourcePath);
        DFErrorFormat(error,"%s: %s",sourceFilename,DFErrorMessage(error));
        free(sourceFilename);
        goto end;
    }

    if (DFFileExists(destPath) && !DFDeleteFile(destPath,error))
        goto end;

    rawPackage = DFPackageNewZip(destPath,0,error);
    if (rawPackage == NULL)
        goto end;

    wordPackage = WordPackageOpenNew(rawPackage,error);
    if (wordPackage == NULL)
        goto end;

    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,htmlPath,idPrefix,error,warnings))
        goto end;

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        goto end;
    }

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;

end:
    free(idPrefix);
    free(htmlPath);
    DFDocumentRelease(htmlDoc);
    DFBufferRelease(warnings);
    DFPackageRelease(rawPackage);
    WordPackageRelease(wordPackage);
    return ok;
}
