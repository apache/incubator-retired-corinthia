//
//  FunctionTests.m
//  dfutil
//
//  Created by Peter Kelly on 5/01/2014.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
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
}
