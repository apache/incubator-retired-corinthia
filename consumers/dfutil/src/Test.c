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

#include "Test.h"
#include "TextPackage.h"
#include "Commands.h"
#include "TestFunctions.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFCommon.h"
#include "DFUnitTest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           TestHarness                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int diffResults2(const char *from, const char *to, const char *tempDir, DFError **error)
{
    char *fromFilename = DFAppendPathComponent(tempDir,"from");
    char *toFilename = DFAppendPathComponent(tempDir,"to");
    int result = 0;
    if (!DFStringWriteToFile(from,fromFilename,error)) {
        DFErrorFormat(error,"%s: %s",fromFilename,DFErrorMessage(error));
    }
    else if (!DFStringWriteToFile(to,toFilename,error)) {
        DFErrorFormat(error,"%s: %s",toFilename,DFErrorMessage(error));
    }
    else {
        char *cmd = DFFormatString("diff -u %s/from %s/to",tempDir,tempDir);
        system(cmd);
        free(cmd);
        result = 1;
    }
    free(fromFilename);
    free(toFilename);
    return result;
}

static int diffResults(const char *from, const char *to, DFError **error)
{
    char *tempDir = createTempDir(error);
    if (tempDir == NULL)
        return 0;

    int result = diffResults2(from,to,tempDir,error);
    DFDeleteFile(tempDir,NULL);
    free(tempDir);
    return result;
}

static char *getCommandFromCode(const char *code, DFError **error)
{
    char *command = NULL;
    const char **lines = DFStringSplit(code,"\n",0);
    int count = 0;
    for (int i = 0; lines[i]; i++) {
        if (!DFStringIsWhitespace(lines[i]) && strncmp(lines[i],"//",2)) {
            if (command == NULL)
                command = strdup(lines[i]);
            count++;
        }
    }
    free(lines);

    if (count != 1) {
        DFErrorFormat(error,"%d commands found",count);
        free(command);
        return NULL;
    }
    else {
        return command;
    }
}

static const char **parseCommand(const char *command, DFError **error)
{
    const char *openbr = strchr(command,'(');
    const char *closebr = strrchr(command,')');

    if ((openbr == NULL) || (closebr == NULL)) {
        DFErrorFormat(error,"Malformed command: %s\n",command);
        return NULL;
    }

    size_t openpos = openbr - command;
    size_t closepos = closebr - command;
    size_t nameend = openpos;

    if ((openpos+2 <= closepos) && (command[openpos+1] == '{') && (command[closepos-1] == '}')) {
        openpos++;
        closepos--;
    }

    char *name = DFSubstring(command,0,nameend);
    char *arguments = DFSubstring(command,openpos+1,closepos);

    const char **components = DFStringSplit(arguments,",",0);
    DFArray *array = DFArrayNew((DFCopyFunction)strdup,free);
    DFArrayAppend(array,name);
    for (int i = 0; components[i]; i++) {
        char *trimmed = DFStringTrimWhitespace(components[i]);
        DFArrayAppend(array,trimmed);
        free(trimmed);
    }

    const char **result = DFStringArrayFlatten(array);

    free(components);
    free(name);
    free(arguments);
    DFArrayRelease(array);
    return result;
}

void TestRun(TestHarness *harness, const char *path)
{
//    @autoreleasepool {
        DFError *error = NULL;
        TextPackage *package = TextPackageNewWithFile(path,&error);
        int pass = 0;
        if (package != NULL) {
            DFHashTable *dict = DFHashTableCopy(package->items);

            char *code = DFStrDup(DFHashTableLookup(dict,""));
            DFHashTableRemove(dict,"");

            char *command = getCommandFromCode(code,&error);
            if (command == NULL) {
                printf("%s: %s\n",path,DFErrorMessage(&error));
                exit(1);
            }

            const char **arguments = parseCommand(command,&error);
            if (arguments == NULL) {
                printf("%s: %s\n",path,DFErrorMessage(&error));
                exit(1);
            }

            char *expectedRaw = DFStrDup(DFHashTableLookup(dict,"expected"));
            if (expectedRaw == NULL) {
                printf("%s: no expected output given\n",path);
                exit(1);
            }
            DFHashTableRemove(dict,"expected");

            char *parentPath = DFPathDirName(path);
            DFBuffer *outputBuf = DFBufferNew();
            utsetup(parentPath,dict,(int)(DFStringArrayCount(arguments)-1),&arguments[1],
                    outputBuf);
            runTest(arguments[0]);

            char *output = DFStringTrimWhitespace(outputBuf->data);
            char *expected = DFStringTrimWhitespace(expectedRaw);

            if (harness->showResults && !harness->showDiffs) {
                printf("%s",outputBuf->data);
            }
            else {
                pass = ((expected != NULL) && DFStringEquals(expected,output));
            }

            if (!pass && harness->showDiffs) {
                if (!diffResults(expected,output,&error))
                    printf("%s\n",DFErrorMessage(&error));
            }

            free(output);
            free(code);
            free(command);
            free(arguments);
            free(expectedRaw);
            free(parentPath);
            free(expected);
            utteardown();
            DFBufferRelease(outputBuf);
            DFHashTableRelease(dict);
        }
        else {
            if (harness->showResults || harness->showDiffs)
                printf("%s\n",DFErrorMessage(&error));
        }

        if (!harness->showResults) {
            if (pass) {
                printf("%-80s PASS\n",path);
                harness->passed++;
            }
            else {
                printf("%-80s FAIL\n",path);
                harness->failed++;
            }
        }
        TextPackageRelease(package);
//    }
}

void TestGetFilenamesRecursive(const char *path, DFArray *result)
{
    if (!DFFileExists(path))
        return;
    int isDirectory = DFIsDirectory(path);
    if (isDirectory) {
        DFError *error = NULL;
        const char **contents = DFContentsOfDirectory(path,0,&error);
        if (contents == NULL) {
            printf("%s: %s\n",path,DFErrorMessage(&error));
            DFErrorRelease(error);
            return;
        }

        DFSortStringsCaseInsensitive(contents);

        for (int i = 0; contents[i]; i++) {
            const char *filename = contents[i];
            char *childPath = DFAppendPathComponent(path,filename);
            TestGetFilenamesRecursive(childPath,result);
            free(childPath);
        }
        free(contents);
    }
    else {
        char *extension = DFPathExtension(path);
        if (DFStringEqualsCI(extension,"test"))
            DFArrayAppend(result,(void *)path);
        free(extension);
    }
}
