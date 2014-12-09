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
