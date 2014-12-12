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

#include "TestCase.h"
#include "TestFunctions.h"
#include "WordPlain.h"
#include "HTMLPlain.h"
#include "DFCommon.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFUnitTest.h"
#include <stdlib.h>
#include <string.h>

DFStorage *TestCaseOpenPackage(DFError **error)
{
    const char *inputDocx = DFHashTableLookup(utgetdata(),"input.docx");
    if (inputDocx == NULL) {
        DFErrorFormat(error,"input.docx not defined");
        return NULL;
    }

    return Word_fromPlain(inputDocx,utgetpath(),error);
}

DFDocument *TestCaseGetHTML(DFStorage *storage, DFError **error)
{
    const char *inputHtml = DFHashTableLookup(utgetdata(),"input.html");
    if (inputHtml == NULL) {
        DFErrorFormat(error,"input.html not defined");
        return NULL;
    }
    return HTML_fromPlain(inputHtml,utgetpath(),storage,error);
}
