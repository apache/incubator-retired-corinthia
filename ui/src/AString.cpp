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

#include "AString.h"
#include <string.h>

class AStringImpl : public AShared
{
public:
    AStringImpl() { }
    ~AStringImpl() { delete chars; }
    AChar *chars;
    unsigned int len;
};

AString::AString()
{
    _impl = NULL;
}

AString::AString(const char *utf8)
{
    _impl = new AStringImpl();
    _impl->len = strlen(utf8);
    _impl->chars = new AChar[_impl->len];
    // FIXME: Do propert UTF-8 decoding here
    for (unsigned int i = 0; i < _impl->len; i++)
        _impl->chars[i] = utf8[i];
}

AString::AString(AChar *chars, unsigned int length)
{
    _impl = new AStringImpl();
    _impl->chars = new AChar[length];
    _impl->len = length;
}

AString::AString(const AString &other)
{
    _impl = other._impl;
}

AString::~AString()
{
}

AString &AString::operator=(const AString &other)
{
    _impl = other._impl;
    return *this;
}

unsigned int AString::length() const
{
    if (_impl.isNull())
        return 0;
    else
        return _impl->len;
}

AChar AString::charAt(int index) const
{
    if (_impl.isNull())
        return 0;
    else
        return _impl->chars[index];
}
