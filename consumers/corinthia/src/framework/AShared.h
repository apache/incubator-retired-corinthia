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

#pragma once

#include <stddef.h>
#include <stdio.h>

class AShared;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          AWeakRefData                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct AWeakRefData
{
    AWeakRefData(void *_back) : ptr(NULL), prev(NULL), next(NULL), back(_back) { }
    AShared *ptr;
    AWeakRefData *prev;
    AWeakRefData *next;
    void *back;
};

struct AWeakRefDataList
{
    AWeakRefDataList() : first(NULL), last(NULL) { }
    AWeakRefData *first;
    AWeakRefData *last;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             AShared                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

class AShared
{
public:
    AShared();
    virtual ~AShared();

    void ref() { _refCount++; }
    void deref() { _refCount--; if (_refCount == 0) delete this; }
    int refCount() const { return _refCount; }

    void addWeakRef(AWeakRefData *ref);
    void removeWeakRef(AWeakRefData *ref);
    int weakRefCount() const;

private:
    int _refCount;
    AWeakRefDataList _weakRefs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                              ARef                                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class ARef
{
public:
    ARef() : _ptr(NULL) { }
    ARef(T *ptr) : _ptr(NULL) { setPtr(ptr); }
    ARef(const ARef<T> &other) : _ptr(NULL) { setPtr(other._ptr); }
    ~ARef() { setPtr(NULL); }

    ARef<T> &operator=(const ARef<T> &other) {
        setPtr(other._ptr);
        return *this;
    }

    T &operator*() const { return *_ptr; }
    T *operator->() const { return _ptr; }
    T *ptr() const { return _ptr; }
    bool isNull() const { return (_ptr == NULL); }

    void setPtr(T *newPtr) {
        T *oldPtr = _ptr;
        if (newPtr != NULL)
            newPtr->ref();
        if (oldPtr != NULL)
            oldPtr->deref();
        _ptr = newPtr;
    }

 private:
    T *_ptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            AWeakRef                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
class AWeakRef
{
public:
    AWeakRef() : _data(this) { }
    AWeakRef(T *ptr) : _data(this) { setPtr(ptr); }
    AWeakRef(const AWeakRef<T> &other) : _data(this) { setPtr(other._data.ptr); }
    ~AWeakRef() { setPtr(NULL); }

    AWeakRef &operator=(const AWeakRef<T> &other) {
        setPtr(other._data.ptr);
        return *this;
    }

    T &operator*() const { return *(static_cast<T*>(_data.ptr)); }
    T *operator->() const { return static_cast<T*>(_data.ptr); }
    T *ptr() const { return static_cast<T*>(_data.ptr); }
    bool isNull() const { return (_data.ptr == NULL); }

    void setPtr(AShared *newPtr) {
        AShared *oldPtr = _data.ptr;
        if (oldPtr != NULL)
            oldPtr->removeWeakRef(&_data);
        if (newPtr != NULL)
            newPtr->addWeakRef(&_data);
        _data.ptr = newPtr;
    }

private:
    AWeakRefData _data;
};
