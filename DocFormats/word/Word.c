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

int DFHTMLToWord2(WordPackage *package, const char *sourcePath, const char *destPath, DFError **error)
{

    DFDocument *htmlDoc = DFParseHTMLFile(sourcePath,0,error);
    if (htmlDoc == NULL) {
        char *sourceFilename = DFPathBaseName(sourcePath);
        DFErrorFormat(error,"%s: %s",sourceFilename,DFErrorMessage(error));
        free(sourceFilename);
        return 0;
    }

    char *idPrefix = DFFormatString("bdt%u-",(unsigned int)rand());
    if (!WordPackageOpenNew(package,error)) {
        DFDocumentRelease(htmlDoc);
        free(idPrefix);
        return 0;
    }

    DFBuffer *warnings = DFBufferNew();
    char *htmlPath = DFPathDirName(sourcePath);
    if (!WordPackageUpdateFromHTML(package,htmlDoc,htmlPath,idPrefix,error,warnings)) {
        free(htmlPath);
        DFBufferRelease(warnings);
        DFDocumentRelease(htmlDoc);
        free(idPrefix);
        return 0;
    }
    free(idPrefix);
    free(htmlPath);
    DFDocumentRelease(htmlDoc);

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        DFBufferRelease(warnings);
        return 0;
    }
    DFBufferRelease(warnings);

    if (!WordPackageSaveTo(package,destPath,error)) {
        return 0;
    }

    return 1;
}

int DFHTMLToWord(const char *sourcePath, const char *destPath, const char *tempPath, DFError **error)
{
    WordPackage *package = WordPackageNew(tempPath);
    int r = DFHTMLToWord2(package,sourcePath,destPath,error);
    WordPackageRelease(package);
    return r;
}
