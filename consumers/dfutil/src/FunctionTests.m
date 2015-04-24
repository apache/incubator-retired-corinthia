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

#import <Foundation/Foundation.h>
#include "DFPlatform.h"
#include "FunctionTests.h"
#include "DFString.h"
#include "DFFilesystem.h"

static NSString *NSStringFromC(const char *cstr)
{
    if (cstr == NULL)
        return NULL;
    else
        return [NSString stringWithUTF8String: cstr];
}

static void printResult(const char *input, const char *expected, const char *actual)
{
    char *quotedInput = DFQuote(input);
    char *quotedExpected = DFQuote(expected);
    char *quotedActual = DFQuote(actual);
    char *resultStr = !strcmp(expected,actual) ? "PASS": "FAIL";

    printf("%-30s %-30s %-30s %s\n",quotedInput,quotedExpected,quotedActual,resultStr);

    free(quotedInput);
    free(quotedExpected);
    free(quotedActual);
}

static void testDirName(const char *input)
{
    const char *expected = NSStringFromC(input).stringByDeletingLastPathComponent.UTF8String;
    char *actual = DFPathDirName(input);
    printResult(input,expected,actual);
    free(actual);
}

static void testBaseName(const char *input)
{
    const char *expected = NSStringFromC(input).lastPathComponent.UTF8String;
    char *actual = DFPathBaseName(input);
    printResult(input,expected,actual);
    free(actual);
}

static void testPathExtension(const char *input)
{
    const char *expected = NSStringFromC(input).pathExtension.UTF8String;
    char *actual = DFPathExtension(input);
    printResult(input,expected,actual);
    free(actual);
}

static void testPathWithoutExtension(const char *input)
{
    const char *expected = NSStringFromC(input).stringByDeletingPathExtension.UTF8String;
    char *actual = DFPathWithoutExtension(input);
    printResult(input,expected,actual);
    free(actual);
}

static const char *dirTests[] = {
    "/one/two/three",
    "/one/two/three/",
    "one/two/three",
    "one/two/three/",
    "something",
    "something/",
    "/something/",
    "/something",
    NULL,
};

static const char *extensionTests[] = {
    "filename.ext",
    "filename",
    "one/two/filename.ext",
    "one/two/filename",
    "/one/two/filename.ext",
    "/one/two/filename",
    "/one/two/three.ext/",
    "/one/two.ext/three/",
    "/one/two.ext/three",
    "/",
    "",
    NULL,
};

static struct { const char *path1; const char *path2; } appendTests[] = {
    { "one", "two" },
    { "one/", "two" },
    { "one", "/two" },
    { "one/", "/two" },
    { "one/two", "three" },
    { "one//two", "three" },
    { "one", "two/three" },
    { "one", "two//three" },
    { "/", "one" },
    { "/", "/one" },
    { "", "one" },
    { "", "/one" },
    { "one", "" },
    { "one", "/" },
    { "one", "two/" },
    { "one", "two//" },
    { NULL, NULL },
};

static void testAppendPathComponent(const char *input1, const char *input2)
{
    const char *expected = ([NSStringFromC(input1) stringByAppendingPathComponent: NSStringFromC(input2)]).UTF8String;
    char *actual = DFAppendPathComponent(input1,input2);
    char *quoteInput = DFFormatString("'%s' '%s'",input1,input2);
    printResult(quoteInput,expected,actual);
    free(actual);
    free(quoteInput);
}

void testPathFunctions(void)
{
    printf("DFDirName\n\n");
    for (int i = 0; 0 < dirTests[i]; i++)
        testDirName(dirTests[i]);

    printf("\nDFBaseName\n\n");
    for (int i = 0; 0 < dirTests[i]; i++)
        testBaseName(dirTests[i]);

    printf("\nDFPathExtension\n\n");
    for (int i = 0; extensionTests[i]; i++)
        testPathExtension(extensionTests[i]);

    printf("\nDFPathWithoutExtension\n\n");
    for (int i = 0; extensionTests[i]; i++)
        testPathWithoutExtension(extensionTests[i]);

    for (int i = 0; appendTests[i].path1; i++)
        testAppendPathComponent(appendTests[i].path1,appendTests[i].path2);
}
