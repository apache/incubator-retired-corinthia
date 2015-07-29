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

#include <stdio.h>
#include "AView.h"
#include "AString.h"

class Foo : public AShared
{
public:
    Foo(int value) : x(value) { printf("%p Foo::Foo()\n",this); }
    virtual ~Foo() { printf("%p Foo::~Foo()\n",this); }
    int x;
    void print() {
        printf("x = %d\n",x);
    }
};

class StrongHolder
{
public:
    StrongHolder() { }
    StrongHolder(const ARef<Foo> &_ref) : ref(_ref) { }
    StrongHolder(Foo *ptr) : ref(ptr) { }
    ARef<Foo> ref;
};

class WeakHolder
{
public:
    WeakHolder() { }
    WeakHolder(const AWeakRef<Foo> &_ref) : ref(_ref) { }
    WeakHolder(Foo *ptr) : ref(ptr) { }
    AWeakRef<Foo> ref;
};

int main(int argc, const char **argv)
{
    printf("Hello World\n");
    Foo *f = new Foo(4);
    printf("f = %p\n",f);

    StrongHolder *s1 = new StrongHolder();
    /*
    StrongHolder *s2 = new StrongHolder();
    StrongHolder *s3 = new StrongHolder();
    WeakHolder *w1 = new WeakHolder();
    WeakHolder *w2 = new WeakHolder();
    WeakHolder *w3 = new WeakHolder();
    */
    printf("s1 = %p\n",s1);

    printf("s1->ref.ptr() = %p\n",s1->ref.ptr());
    printf("before\n");
    s1->ref = f;
    printf("after\n");
    printf("s1->ref.ptr() = %p\n",s1->ref.ptr());
    //    printf("s1->ref.ptr()->x = %d\n",s1->ref.ptr()->x);

    return 0;
}
