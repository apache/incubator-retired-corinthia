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

#ifndef DocFormats_DFUnitTest_h
#define DocFormats_DFUnitTest_h

typedef struct TestCase TestCase;
typedef struct TestGroup TestGroup;

struct TestCase {
    const char *name;
    void (*fun)(void);
};

struct TestGroup {
    const char *name;
    TestCase tests[];
};

// Functions to be called by the test harness

void utrun(TestGroup **groups);

// Functions to be called from within test cases

void utassert(int condition, const char *description);
void utexpect(const char *actual, const char *expected);
void utfail(const char *reason);

#endif
