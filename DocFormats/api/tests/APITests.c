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
#include <stddef.h>

static void test_api_one(void)
{
    utassert(1 == 2,"1 equals 2");
}

static void test_api_two(void)
{
    utexpect("failure","success");
}

static void test_api_three(void)
{
    utfail("Failed because I felt like it");
}

static void test_api_four(void)
{
    // Pass!
}

TestGroup APITests = {
    "api", {
        { "one", PlainTest, test_api_one },
        { "two", PlainTest, test_api_two },
        { "three", PlainTest, test_api_three },
        { "four", PlainTest, test_api_four },
        { NULL, PlainTest, NULL },
    }
};
