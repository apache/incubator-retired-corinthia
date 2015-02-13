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
#include "Word.h"
#include "WordStyles.h"
#include "DFHTML.h"
#include "DFFilesystem.h"
#include "CSSProperties.h"
#include "DFString.h"
#include "DFCommon.h"
#include "DFZipFile.h"
#include <stdlib.h>

DFDocument *WordGet(DFStorage *concreteStorage, DFStorage *abstractStorage, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;
    DFDocument *htmlDoc = NULL;

    wordPackage = WordPackageOpenFrom(concreteStorage,error);
    if (wordPackage == NULL)
        goto end;

    htmlDoc = DFDocumentNew();
    if (!WordConverterGet(htmlDoc,abstractStorage,wordPackage,error))
        goto end;

    ok = 1;

end:
    WordPackageRelease(wordPackage);
    if (ok) {
        return htmlDoc;
    }
    else {
        DFDocumentRelease(htmlDoc);
        return NULL;
    }
}

int WordPut(DFStorage *concreteStorage, DFStorage *abstractStorage, DFDocument *htmlDoc, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;

    wordPackage = WordPackageOpenFrom(concreteStorage,error);
    if (wordPackage == NULL)
        goto end;

    if (!WordConverterPut(htmlDoc,abstractStorage,wordPackage,error))
        goto end;

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;

end:
    WordPackageRelease(wordPackage);
    return ok;
}

int WordCreate(DFStorage *concreteStorage, DFStorage *abstractStorage, DFDocument *htmlDoc, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;

    wordPackage = WordPackageOpenNew(concreteStorage,error);
    if (wordPackage == NULL)
        goto end;

    // Change any id attributes starting with "word" to a different prefix, so they
    // are not treated as references to nodes in the destination document. This is necessary
    // if the HTML file was previously generated from a word file, and we are creating
    // a new word file from it.
    HTMLBreakBDTRefs(htmlDoc->docNode,"word");

    if (!WordConverterPut(htmlDoc,abstractStorage,wordPackage,error))
        goto end;

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;

end:
    WordPackageRelease(wordPackage);
    return ok;
}

int WordCollapseBookmarks(DFStorage *concreteStorage, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;

    wordPackage = WordPackageOpenFrom(concreteStorage,error);
    if (wordPackage == NULL)
        goto end;

    WordPackageCollapseBookmarks(wordPackage);

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;
end:
    WordPackageRelease(wordPackage);
    return ok;
}

int WordExpandBookmarks(DFStorage *concreteStorage, DFError **error)
{
    int ok = 0;
    WordPackage *wordPackage = NULL;

    wordPackage = WordPackageOpenFrom(concreteStorage,error);
    if (wordPackage == NULL)
        goto end;

    WordPackageExpandBookmarks(wordPackage);

    if (!WordPackageSave(wordPackage,error))
        goto end;

    ok = 1;
end:
    WordPackageRelease(wordPackage);
    return ok;
}
