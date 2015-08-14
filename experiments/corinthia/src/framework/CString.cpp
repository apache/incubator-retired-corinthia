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

#include "CString.h"
#include <string.h>

class CStringImpl : public CShared
{
public:
    CStringImpl() { }
    ~CStringImpl() { delete chars; }
    CChar *chars;
    unsigned int len;
};

CString::CString()
{
    _impl = NULL;
}

CString::CString(const char *utf8)
{
    _impl = new CStringImpl();
    _impl->len = strlen(utf8);
    _impl->chars = new CChar[_impl->len];
    // FIXME: Do propert UTF-8 decoding here
    for (unsigned int i = 0; i < _impl->len; i++)
        _impl->chars[i] = utf8[i];
}

CString::CString(CChar *chars, unsigned int length)
{
    _impl = new CStringImpl();
    _impl->chars = new CChar[length];
    _impl->len = length;
}

CString::CString(const CString &other)
{
    _impl = other._impl;
}

CString::~CString()
{
}

CString &CString::operator=(const CString &other)
{
    _impl = other._impl;
    return *this;
}

unsigned int CString::length() const
{
    if (_impl.isNull())
        return 0;
    else
        return _impl->len;
}

CChar CString::charAt(int index) const
{
    if (_impl.isNull())
        return 0;
    else
        return _impl->chars[index];
}
