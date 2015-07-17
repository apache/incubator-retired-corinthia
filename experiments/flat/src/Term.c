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

#include "Common.h"
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

void TermPrint(Term *term, const char *input, const char *indent)
{
    switch (ExpressionKind(term->type)) {
        case IdentExpr:
            printf("%s %s\n",ExprKindAsString(ExpressionKind(term->type)),ExprIdentValue(term->type));
            break;
        case LitExpr:
        case RangeExpr:
        case DotExpr: {
            int start = term->start;
            int end = term->end;
            int inputLen = strlen(input);
            if ((start >= 0) && (start <= inputLen) && (end >= 0) && (end <= inputLen) && (start <= end)) {
                char *temp = (char*)malloc(end-start+1);
                memcpy(temp,&input[start],end-start);
                temp[end-start] = '\0';

                printf("%s ",ExprKindAsString(ExpressionKind(term->type)));
                printLiteral(temp);
                printf("\n");

                free(temp);
            }
            else {
                printf("%s\n",ExprKindAsString(ExpressionKind(term->type)));
            }
            break;
        }
        default:
            printf("%s\n",ExprKindAsString(ExpressionKind(term->type)));
            break;
    }

    int indentLen = strlen(indent);
    char *nextIndent = (char *)malloc(indentLen+5);

    for (TermList *child = term->children; child != NULL; child = child->next) {
        if (child->next != NULL) {
            printf("%s|-- ",indent);
            snprintf(nextIndent,indentLen+5,"%s|   ",indent);
            TermPrint(child->term,input,nextIndent);
        }
        else {
            printf("%s\\-- ",indent);
            snprintf(nextIndent,indentLen+5,"%s    ",indent);
            TermPrint(child->term,input,nextIndent);
        }
    }

    free(nextIndent);
}
