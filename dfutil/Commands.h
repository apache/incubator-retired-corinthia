//
//  Commands.h
//  dfutil
//
//  Created by Peter Kelly on 25/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef dfutil_Commands_h
#define dfutil_Commands_h

#include "DFError.h"
#include "DFBuffer.h"

int prettyPrintFile(const char *filename, DFError **error);
int fromPlain(const char *inFilename, const char *outFilename, DFError **error);
int normalizeFile(const char *filename, DFError **error);
int convertFile(const char *inFilename, const char *outFilename, DFError **error);
int resaveOPCFile(const char *filename, DFError **error);
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
