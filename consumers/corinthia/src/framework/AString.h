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

class AStringImpl;

#include <stdint.h>
#include "AShared.h"

typedef uint32_t AChar;

class AString
{
public:
    AString();
    AString(const char *utf8);
    AString(AChar *chars, unsigned int length);
    AString(const AString &other);
    ~AString();

    AString &operator=(const AString &other);

    unsigned int length() const;
    AChar charAt(int index) const;

    /*
    int compare(const AString &other) const;
    bool hasPrefix(const AString &other) const;
    bool hasSuffix(const AString &other) const;
    AString substring(int start, int end) const;
    AString substringTo(int start) const;
    AString substringFrom(int end) const;
    AString lowerCase() const;
    AString upperCase() const;
    */

 private:
    ARef<AStringImpl> _impl;
};
