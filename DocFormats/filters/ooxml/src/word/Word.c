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

DFDocument *WordGet(DFPackage *concretePackage, DFPackage *abstractPackage, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;
    DFBuffer *warnings = DFBufferNew();
    DFDocument *htmlDoc = NULL;

    wordPackage = WordPackageOpenFrom(concretePackage,error);
    if (wordPackage == NULL)
        goto end;

    htmlDoc = WordPackageGenerateHTML(wordPackage,abstractPackage,"word",error,warnings);
    if (htmlDoc == NULL)
        goto end;

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        goto end;
    }

    HTML_safeIndent(htmlDoc->docNode,0);

    ok = 1;

end:
    DFBufferRelease(warnings);
    WordPackageRelease(wordPackage);
    if (ok) {
        return htmlDoc;
    }
    else {
        DFDocumentRelease(htmlDoc);
        return NULL;
    }
}

int WordPut(DFPackage *concretePackage, DFPackage *abstractPackage, DFDocument *htmlDoc, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;
    DFBuffer *warnings = DFBufferNew();

    const char *idPrefix = "word";

    wordPackage = WordPackageOpenFrom(concretePackage,error);
    if (wordPackage == NULL)
        goto end;

    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,abstractPackage,idPrefix,error,warnings))
        goto end;

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        goto end;
    }

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;

end:
    WordPackageRelease(wordPackage);
    DFBufferRelease(warnings);
    return ok;
}

int WordCreate(DFPackage *concretePackage, DFPackage *abstractPackage, DFDocument *htmlDoc, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;
    DFBuffer *warnings = DFBufferNew();

    const char *idPrefix = "word";

    wordPackage = WordPackageOpenNew(concretePackage,error);
    if (wordPackage == NULL)
        goto end;

    // Change any id attributes starting with "word" or "odf" to a different prefix, so they
    // are not treated as references to nodes in the destination document. This is necessary
    // if the HTML file was previously generated from a word or odf file, and we are creating
    // a new word or odf file from it.
    HTMLBreakBDTRefs(htmlDoc->docNode,idPrefix);

    if (!WordPackageUpdateFromHTML(wordPackage,htmlDoc,abstractPackage,idPrefix,error,warnings))
        goto end;

    if (warnings->len > 0) {
        DFErrorFormat(error,"%s",warnings->data);
        goto end;
    }

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;

end:
    WordPackageRelease(wordPackage);
    DFBufferRelease(warnings);
    return ok;
}
