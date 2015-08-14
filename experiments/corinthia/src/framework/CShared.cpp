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

#include "CShared.h"
#include <assert.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             CShared                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

CShared::CShared()
    : _refCount(0)
{
}

CShared::~CShared()
{
    CWeakRefData *ref = _weakRefs.first;
    while (ref != NULL) {
        CWeakRefData *next = ref->next;
        ref->ptr = NULL;
        ref->prev = NULL;
        ref->next = NULL;
        ref = next;
    }
}

int CShared::weakRefCount() const
{
    int count = 0;
    for (CWeakRefData *d = _weakRefs.first; d != NULL; d = d->next)
        count++;
    return count;
}

void CShared::addWeakRef(CWeakRefData *ref)
{
    ref->ptr = this;

    assert(!ref->prev && !ref->next);
    if (_weakRefs.last) {
        ref->prev = _weakRefs.last;
        _weakRefs.last->next = ref;
        _weakRefs.last = ref;
    }
    else {
        _weakRefs.first = _weakRefs.last = ref;
    }
}

void CShared::removeWeakRef(CWeakRefData *ref)
{
    ref->ptr = NULL;

    assert(ref->prev || (_weakRefs.first == ref));
    assert(ref->next || (_weakRefs.last == ref));
    if (_weakRefs.first == ref)
        _weakRefs.first = ref->next;
    if (_weakRefs.last == ref)
        _weakRefs.last = ref->prev;
    if (ref->next)
        ref->next->prev = ref->prev;
    if (ref->prev)
        ref->prev->next = ref->next;
    ref->next = NULL;
    ref->prev = NULL;
}
