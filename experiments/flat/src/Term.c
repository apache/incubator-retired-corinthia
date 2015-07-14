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

#include "Term.h"
#include "Util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

Term *TermNew(Expression *type, int start, int end, TermList *children)
{
    assert(type != NULL);
    Term *term = (Term *)calloc(1,sizeof(Term));
    term->type = type;
    term->start = start;
    term->end = end;
    term->children = children;
    return term;
}

TermList *TermListNew(Term *term, TermList *next)
{
    TermList *list = (TermList *)calloc(1,sizeof(TermList));
    list->term = term;
    list->next = next;
    return list;
}

void TermListPtrAppend(TermList ***listPtr, Term *term)
{
    assert(term != NULL);
    assert(listPtr != NULL);
    assert(*listPtr != NULL);
    assert(**listPtr == NULL);
    **listPtr = TermListNew(term,NULL);
    *listPtr = &(**listPtr)->next;
}
