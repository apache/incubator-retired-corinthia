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
#include "DFUnitTest.h"
#include "TextPackage.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFCommon.h"
#include "DFUnitTest.h"
#include "DFArray.h"
#include "DFBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern TestGroup APITests;
extern TestGroup CSSTests;
extern TestGroup HTMLTests;
extern TestGroup LibTests;
extern TestGroup XMLTests;
extern TestGroup LaTeXTests;
extern TestGroup ODFTests;
extern TestGroup WordTests;
extern TestGroup PlatformOSTests;
extern TestGroup PlatformWrapperTests;
extern TestGroup BDTTests;

TestGroup *allGroups[] = {
    &APITests,
    &BDTTests,
    &CSSTests,
    &HTMLTests,
    &LibTests,
    &XMLTests,
    &LaTeXTests,
    &ODFTests,
    &WordTests,
    &PlatformOSTests,
    &PlatformWrapperTests,
    NULL
};

typedef struct {
    int showResults;
    int showDiffs;
    int passed;
    int failed;
} TestHarness;

static int diffResults(const char *from, const char *to, DFError **error)
{
    const char *fromFilename = "dftest-diff-from.tmp";
    const char *toFilename = "dftest-diff-to.tmp";
    int result = 0;
    if (!DFStringWriteToFile(from,fromFilename,error)) {
        DFErrorFormat(error,"%s: %s",fromFilename,DFErrorMessage(error));
    }
    else if (!DFStringWriteToFile(to,toFilename,error)) {
        DFErrorFormat(error,"%s: %s",toFilename,DFErrorMessage(error));
    }
    else {
        char *cmd = DFFormatString("diff -u %s %s",fromFilename,toFilename);
        system(cmd);
        free(cmd);
        result = 1;
    }
    DFDeleteFile(fromFilename,NULL);
    DFDeleteFile(toFilename,NULL);
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

    if ((openbr == NULL) && (closebr == NULL)) {
        DFArray *array = DFArrayNew((DFCopyFunction)strdup,free);
        DFArrayAppend(array,(void *)command);
        const char **result = DFStringArrayFlatten(array);
        DFArrayRelease(array);
        return result;
    }

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

static void TestRun(TestHarness *harness, const char *path)
{
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

        TestCase *tc = utlookup(allGroups,arguments[0]);
        if (tc != NULL) {
            tc->fun();
        }
        else {
            DFBufferFormat(utgetoutput(),"Unknown test: %s\n",arguments[0]);
        }

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
}

static void TestGetFilenamesRecursive(const char *path, DFArray *result)
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

void runTests(int argc, const char **argv, int diff)
{
    DFArray *tests = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (int i = 0; i < argc; i++) {
        const char *path = argv[i];
        if (!DFFileExists(path)) {
            fprintf(stderr,"%s: No such file or directory\n",path);
            exit(1);
        }
        TestGetFilenamesRecursive(path,tests);
    }

    TestHarness harness;
    bzero(&harness,sizeof(TestHarness));
    harness.showResults = (DFArrayCount(tests) == 1);
    harness.showDiffs = diff;

    if (DFArrayCount(tests) == 1) {
        TestRun(&harness,(const char *)DFArrayItemAt(tests,0));
    }
    else {
        for (size_t i = 0; i < DFArrayCount(tests); i++) {
            const char *test = DFArrayItemAt(tests,i);
            TestRun(&harness,test);
        }
        printf("Passed: %d\n",harness.passed);
        printf("Failed: %d\n",harness.failed);
    }
    DFArrayRelease(tests);
}

int main(int argc, const char **argv)
{
    // Ensure that if a segfault occurs half-way through printing a line,
    // we still get the partial output.
    setbuf(stdout,NULL);

    if ((argc >= 2) && !strcmp(argv[1],"-plain")) {
      if (argc == 2)
        utrun(allGroups, 1, 0, NULL);
      else {
        // Arg[2] == testgroup to run
        TestGroup *singleGroup[] = { NULL, NULL };
        int        i = 0;

        for (; allGroups[i] && strcmp(argv[2], allGroups[i]->name); i++) ;
        if (allGroups[i]) {
          singleGroup[0] = allGroups[i];
          utrun(singleGroup, 1, 0, NULL);
        }
        else
          printf("\n function group \"%s\" does not exist!\n\n", argv[2]);
      }
    }
    else if ((argc >= 3) && !strcmp(argv[1],"-diff")) {
        runTests(argc-2,&argv[2],1);
    }
    else if (argc >= 2) {
        runTests(argc-1,&argv[1],0);
    }
    else {
        // Usage
        printf("Usage:\n"
               "\n"
               "dftest path1 path2 ...\n"
               "\n"
               "    Run a series of automated tests, consisting of all the .test files in the\n"
               "    specified path(s). If only one file is found, the test is run and the\n"
               "    result is printed to standard output. If multiple files are found, they are\n"
               "    all run, and their pass/fail status is printed.\n"
               "\n"
               "dftest -diff path1 path2 ...\n"
               "\n"
               "    As above, but for each test that fails, print out a diff between the expected\n"
               "    and actual results.\n"
               "\n"
               "dftest -plain\n"
               "\n"
               "    Run all tests functions of type PlainTest, which don't use any data files\n"
               "\n");
    }
    return 0;
}
