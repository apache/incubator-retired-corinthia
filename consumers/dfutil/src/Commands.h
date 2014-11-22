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

#ifndef dfutil_Commands_h
#define dfutil_Commands_h

#include "DFError.h"
#include "DFBuffer.h"

int prettyPrintFile(const char *filename, DFError **error);
int fromPlain(const char *inFilename, const char *outFilename, DFError **error);
int normalizeFile(const char *filename, DFError **error);
int convertFile(const char *inFilename, const char *outFilename, DFError **error);
int testCSS(const char *filename, DFError **error);
int parseHTMLFile(const char *filename, DFError **error);
int simplifyFields(const char *inFilename, const char *outFilename, DFError **error);
int textPackageList(const char *filename, DFError **error);
int textPackageGet(const char *filename, const char *itemPath, DFError **error);
int runTests(int argc, const char **argv, int diff, DFError **error);
int printTree(const char *filename, DFError **error);
int diffFiles(const char *filename1, const char *filename2, DFError **error);
void parseContent(const char *content);
int btosFile(const char *inFilename, const char *outFilename, DFError **error);
int stobFile(const char *inFilename, const char *outFilename, DFError **error);
int escapeCSSIdent(const char *filename, DFError **error);
int unescapeCSSIdent(const char *filename, DFError **error);

char *createTempDir(DFError **error);
char *binaryToString(DFBuffer *input);
DFBuffer *stringToBinary(const char *str);

#endif
