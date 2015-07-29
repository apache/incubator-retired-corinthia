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

class CStringImpl;

#include <stdint.h>
#include "CShared.h"

typedef uint32_t CChar;

class CString
{
public:
    CString();
    CString(const char *utf8);
    CString(CChar *chars, unsigned int length);
    CString(const CString &other);
    ~CString();

    CString &operator=(const CString &other);

    unsigned int length() const;
    CChar charAt(int index) const;

    /*
    int compare(const CString &other) const;
    bool hasPrefix(const CString &other) const;
    bool hasSuffix(const CString &other) const;
    CString substring(int start, int end) const;
    CString substringTo(int start) const;
    CString substringFrom(int end) const;
    CString lowerCase() const;
    CString upperCase() const;
    */

 private:
    CRef<CStringImpl> _impl;
};
