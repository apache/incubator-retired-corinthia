// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "WordLenses.h"
#include "DFCommon.h"

DFNode *WordContainerGet(WordGetData *get, WordLens *childLens, DFNode *abstract, DFNode *concrete)
{
    return BDTContainerGet(get,(DFLens *)childLens,abstract,concrete);
}

void WordContainerPut(WordPutData *put, WordLens *childLens, DFNode *abstract, DFNode *concrete)
{
    BDTContainerPut(put,(DFLens *)childLens,abstract,concrete,
                    (DFLookupConcreteFunction)WordConverterGetConcrete);
}
