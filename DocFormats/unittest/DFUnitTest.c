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

#include "DFUnitTest.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct {
    int failed;
    char *reason;
} curtest;

int totalPasses = 0;
int totalFailures = 0;

static void runTest(TestCase *test)
{
    memset(&curtest,0,sizeof(curtest));
    printf("%-60s ",test->name);
    test->fun();
    if (curtest.failed)
        printf("FAIL");
    else
        printf("PASS");
    if (curtest.reason != NULL)
        printf(" %s",curtest.reason);
    printf("\n");
    free(curtest.reason);
}

static void runGroup(TestGroup *group)
{
    printf("Test group: %s\n",group->name);
    for (int i = 0; group->tests[i].name; i++) {
        if (group->tests[i].type == PlainTest)
            runTest(&group->tests[i]);
    }
}

void utrun(TestGroup **groups, int plain, int data, const char **filenames)
{
    for (int i = 0; groups[i]; i++)
        runGroup(groups[i]);
}

void utassert(int condition, const char *description)
{
    if (!condition) {
        curtest.failed = 1;
        curtest.reason = strdup(description);
    }
}

void utexpect(const char *actual, const char *expected)
{
    if (strcmp(actual,expected)) {
        curtest.failed = 1;
        curtest.reason = strdup("Output mismatch");
    }
}

void utfail(const char *reason)
{
    curtest.failed = 1;
    curtest.reason = strdup(reason);
}

TestCase *utlookup(TestGroup **groups, const char *name)
{
    for (int gi = 0; groups[gi]; gi++) {
        TestGroup *group = groups[gi];
        if (!strncmp(name,group->name,strlen(group->name)) &&
            (name[strlen(group->name)] == '.')) {
            const char *testName = &name[strlen(group->name)+1];
            for (int ti = 0; group->tests[ti].name; ti++) {
                TestCase *test = &group->tests[ti];
                if (!strcmp(test->name,testName)) {
                    return test;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

static const char *testPath = NULL;
static struct DFHashTable *testData = NULL;
static int testArgc = 0;
static const char **testArgv = NULL;
static struct DFBuffer *testOutput = NULL;

void utsetup(const char *path, struct DFHashTable *data, int argc, const char **argv, struct DFBuffer *output)
{
    testPath = path;
    testData = data;
    testArgc = argc;
    testArgv = argv;
    testOutput = output;
}

void utteardown(void)
{
    testPath = NULL;
    testData = NULL;
    testArgc = 0;
    testArgv = NULL;
    testOutput = NULL;
}

const char *utgetpath(void)
{
    return testPath;
}

void *utgetdata(void)
{
    return testData;
}

int utgetargc(void)
{
    return testArgc;
}

const char **utgetargv(void)
{
    return testArgv;
}

void *utgetoutput(void)
{
    return testOutput;
}
