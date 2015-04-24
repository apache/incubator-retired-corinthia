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

#ifndef DocFormats_DFUnitTest_h
#define DocFormats_DFUnitTest_h

/*
 
 There are two types of tests: plain tests and data tests. The former are entirely self-contained;
 they consist of a C function that takes no input and produces no output; simply calling functions
 like utassert(), utexpect(), and utfail() to trigger success or failure. These are intended for
 testing functionality that requires no or only trivial amounts of input data.
 
 Data tests are for functions which need to be called multiple times with different input values,
 read from files. The input data comes from the *.test files included in the various modules, the
 first line of each indicating the name of the test function that should be called, as well as
 optional parameters. A data test retrieves these parameters and/or input data, and acts upon it.
 Typically, part of the input data for the test will include the expected output of the test, which
 can be checked by calling utexpect().
 
 The tests are triggered by the dftest function, which can be run in three modes:
 
 dftest plain
 dftest data <path>
 dftest all <path>
 
 When run in 'plain' mode, only the plain tests requiring no input files are executed. When run in
 'data' mode, the specified path will be searched for all files with the extension .test, and the
 functions indicated in the first line of those files will be called. A given function may be called
 multiple times, since each test file corresponds to one test execution. The 'all' mode does both
 plain and data tests.
 
 All test groups defined in the various source files in test/ directories must be explicitly
 referenced from main.c in dftest, as this is where it gets the references to the set of available
 groups. These are passed to utrun, so it can determine what tests it needs to execute based on
 the plain and data parameters.

 */

typedef enum {
    PlainTest,
    DataTest,
} TestType;

typedef struct TestCase TestCase;
typedef struct TestGroup TestGroup;

struct TestCase {
    const char *name;
    TestType type;
    void (*fun)(void);
};

struct TestGroup {
    const char *name;
    TestCase tests[];
};

// Functions to be called by the test harness

struct DFHashTable;
struct DFBuffer;

TestCase *utlookup(TestGroup **groups, const char *name);
void utsetup(const char *path, struct DFHashTable *data, int argc, const char **argv, struct DFBuffer *output);
void utteardown(void);
void utrun(TestGroup **groups, int plain, int data, const char **filenames);

// Functions to be called from within test cases

const char *utgetpath(void);
void *utgetdata(void);
int utgetargc(void);
const char **utgetargv(void);
void *utgetoutput(void);

void utassert(int condition, const char *description);
void utexpect(const char *actual, const char *expected);
void utfail(const char *reason);

#endif
