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

#ifndef DocFormats_DFCallback_h
#define DocFormats_DFCallback_h

typedef void (*DFCallbackFunction)(void *ctx, void *object, void *data);

typedef struct DFCallback DFCallback;

struct DFCallback {
    DFCallbackFunction fun;
    void *ctx;
    DFCallback *next;
};

void DFCallbackAdd(DFCallback **list, DFCallbackFunction fun, void *ctx);
void DFCallbackRemove(DFCallback **list, DFCallbackFunction fun, void *ctx);
void DFCallbackInvoke(DFCallback *list, void *object, void *data);

#endif
