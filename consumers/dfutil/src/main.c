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
#include "Commands.h"
#include "BDTTests.h"
#include "WordPlain.h"
#include "HTMLPlain.h"
#include "FunctionTests.h"
#include "StringTests.h"
#include "DFZipFile.h"
#include "DFCommon.h"
#include "DFFilesystem.h"
#include <DocFormats/DocFormats.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int runCommand(int argc, const char **argv, DFError **dferr)
{
    if ((argc == 3) && !strcmp(argv[1],"-pp")) {
        return prettyPrintFile(argv[2],dferr);
    }
    else if ((argc == 4) && !strcmp(argv[1],"-fp")) {
        return fromPlain(argv[2],argv[3],dferr);
    }
    else if ((argc == 3) && !strcmp(argv[1],"-normalize")) {
        return normalizeFile(argv[2],dferr);
    }
    else if ((argc >= 2) && !strcmp(argv[1],"-bdt")) {
        BDT_Test(argc-2,&argv[2]);
        return 1;
    }
    else if ((argc == 3) && !strcmp(argv[1],"-css")) {
        return testCSS(argv[2],dferr);
    }
    else if ((argc == 3) && !strcmp(argv[1],"-parsehtml")) {
        return parseHTMLFile(argv[2],dferr);
    }
    else if ((argc == 3) && !strcmp(argv[1],"-tplist")) {
        return textPackageList(argv[2],dferr);
    }
    else if ((argc == 4) && !strcmp(argv[1],"-tpget")) {
        return textPackageGet(argv[2],argv[3],dferr);
    }
    else if ((argc == 4) && !strcmp(argv[1],"-diff")) {
        return diffFiles(argv[2],argv[3],dferr);
    }
    else if ((argc == 3) && !strcmp(argv[1],"-parsecontent")) {
        parseContent(argv[2]);
        return 1;
    }
    else if ((argc >= 2) && !strcmp(argv[1],"-btos")) {
        const char *inFilename = (argc >= 3) ? argv[2] : NULL;
        const char *outFilename = (argc >= 4) ? argv[3] : NULL;
        return btosFile(inFilename,outFilename,dferr);
    }
    else if ((argc >= 2) && !strcmp(argv[1],"-stob")) {
        const char *inFilename = (argc >= 3) ? argv[2] : NULL;
        const char *outFilename = (argc >= 4) ? argv[3] : NULL;
        return stobFile(inFilename,outFilename,dferr);
    }
    else if ((argc == 3) && (!strcmp(argv[1],"-css-escape"))) {
        return escapeCSSIdent(argv[2],dferr);
    }
    else if ((argc == 3) && (!strcmp(argv[1],"-css-unescape"))) {
        return unescapeCSSIdent(argv[2],dferr);
    }
#ifdef __APPLE__
    else if ((argc == 2) && (!strcmp(argv[1],"-test-unicode"))) {
        return testUnicode();
    }
    else if ((argc == 2) && (!strcmp(argv[1],"-test-strings"))) {
        return testStrings();
    }
    else if ((argc == 2) && (!strcmp(argv[1],"-test-path"))) {
        testPathFunctions();
        return 1;
    }
#endif
    else if ((argc == 4) && !strcmp(argv[1],"-zip")) {
        DFStorage *storage = DFStorageNewFilesystem(argv[3],DFFileFormatUnknown);
        int r = DFZip(argv[2],storage,dferr);
        DFStorageRelease(storage);
        return r;
    }
    else if ((argc == 4) && !strcmp(argv[1],"-unzip")) {
        DFStorage *storage = DFStorageNewFilesystem(argv[3],DFFileFormatUnknown);
        int r = DFUnzip(argv[2],storage,dferr);
        DFStorageRelease(storage);
        return r;
    }
    else {
               ////////////////////////////////////////////////////////////////////////////////
        printf("Usage:\n"
               "\n"
              "dfutil -pp filename\n"
              "    Print a plain text version of a .docx or .odt file to standard output\n"
              "\n"
              "dfutil -fp infilename outfilename\n"
              "    Create a .docx or .odt file based on a plain text representation. If\n"
              "    infilename is -, read from standard input.\n"
              "\n"
              "dfutil -normalize filename\n"
              "    Normalize a HTML file\n"
              "\n"
              "dfutil -parsecontent string\n"
              "    Parse a value as if it were given as a CSS 'content' property, and print parts\n"
              "\n"
              "dfutil -btos [infilename] [outfilename]\n"
              "    Convert binary data to string\n"
              "\n"
              "dfutil -stob [infilename] [outfilename]\n"
              "    Convert string to binary data\n"
              "\n"
              "dfutil -css-escape [infilename]\n"
              "    Escape CSS class name\n"
              "\n"
              "dfutil -css-unescape [infilename]\n"
              "    Unescape CSS class name\n"
              "\n"
               "dfutil -zip zipFilename sourceDir\n"
               "    Create a zip file\n"
               "\n"
               "dfutil -unzip zipFilename destDir\n"
               "    Extract a zip file\n"
               "\n"
              "dfutil input.html output.docx\n"
              "dfutil input.html output.odt\n"
              "dfutil input.docx output.html\n"
              "dfutil input.docx output.html\n"
              "    Convert to/from .docx or .odt and .html\n");
        return 1;
    }
}

int main(int argc, const char * argv[])
{
    int r = 0;
    DFError *dferr = NULL;
    if (!runCommand(argc,argv,&dferr)) {
        fprintf(stderr,"%s\n",DFErrorMessage(&dferr));
        DFErrorRelease(dferr);
        r = 1;
    }
    return r;
}
