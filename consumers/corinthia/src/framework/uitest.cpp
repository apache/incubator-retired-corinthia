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
#include "CView.h"
#include "CString.h"

int fooInstances = 0;

class Foo : public CShared
{
public:
    Foo(int value) : x(value) {
        printf("    %p Foo::Foo()\n",this);
        fooInstances++;
    }
    virtual ~Foo() {
        printf("    %p Foo::~Foo()\n",this);
        fooInstances--;
    }
    int x;
    void print() {
        printf("x = %d\n",x);
    }
};

class StrongHolder
{
public:
    StrongHolder() { }
    StrongHolder(const CRef<Foo> &_ref) : ref(_ref) { }
    StrongHolder(Foo *ptr) : ref(ptr) { }
    CRef<Foo> ref;
};

class WeakHolder
{
public:
    WeakHolder() { }
    WeakHolder(const CWeakRef<Foo> &_ref) : ref(_ref) { }
    WeakHolder(Foo *ptr) : ref(ptr) { }
    CWeakRef<Foo> ref;
};

int main(int argc, const char **argv)
{
    printf("step 0\n");
    Foo *f = new Foo(4);
    printf("    f = %p\n",f);

    StrongHolder *s1 = new StrongHolder();
    StrongHolder *s2 = new StrongHolder();

    WeakHolder *w1 = new WeakHolder();
    WeakHolder *w2 = new WeakHolder();

    printf("step 1\n");
    printf("    instances=%d, strong=%d, weak=%d, w1=%p, w2=%p\n",
           fooInstances,f->refCount(),f->weakRefCount(),w1->ref.ptr(),w2->ref.ptr());
    s1->ref = f; // Should bring refCount to 1
    printf("    instances=%d, strong=%d, weak=%d, w1=%p, w2=%p\n",
           fooInstances,f->refCount(),f->weakRefCount(),w1->ref.ptr(),w2->ref.ptr());
    printf("step 2\n");
    s2->ref = f; // Should bring refCount to 2
    printf("    instances=%d, strong=%d, weak=%d, w1=%p, w2=%p\n",
           fooInstances,f->refCount(),f->weakRefCount(),w1->ref.ptr(),w2->ref.ptr());
    printf("step 3\n");
    w1->ref = f; // Should bring weakRefCount to 1
    printf("    instances=%d, strong=%d, weak=%d, w1=%p, w2=%p\n",
           fooInstances,f->refCount(),f->weakRefCount(),w1->ref.ptr(),w2->ref.ptr());
    printf("step 4\n");
    w2->ref = f; // Should bring weakRefCount to 2
    printf("    instances=%d, strong=%d, weak=%d, w1=%p, w2=%p\n",
           fooInstances,f->refCount(),f->weakRefCount(),w1->ref.ptr(),w2->ref.ptr());
    printf("step 5\n");
    delete s1; // Should bring refCount to 1
    printf("    instances=%d, strong=%d, weak=%d, w1=%p, w2=%p\n",
           fooInstances,f->refCount(),f->weakRefCount(),w1->ref.ptr(),w2->ref.ptr());
    printf("step 6\n");
    delete s2; // Should bring refCount to 0, deleting Foo, and clearing weak references
    printf("    instances=%d w1=%p, w2=%p\n",
           fooInstances,w1->ref.ptr(),w2->ref.ptr());
    printf("done\n");

    return 0;
}
