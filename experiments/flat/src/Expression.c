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

#include "Expression.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

struct Expression {
    ExprKind kind;
    int count;
    char *value;
    int start;
    int end;
    Expression *children[];
};

Expression *ExpressionNew(ExprKind kind, int count, Expression **children)
{
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+count*sizeof(Expression *));
    expr->kind = kind;
    expr->value = NULL;
    expr->count = count;
    memcpy(expr->children,children,count*sizeof(Expression *));
    return expr;
}

Expression *ExpressionNewValue(ExprKind kind, const char *value)
{
    Expression *expr = (Expression *)calloc(1,sizeof(Expression));
    expr->kind = kind;
    expr->value = strdup(value);
    expr->count = 0;
    return expr;
}

Expression *ExpressionNewRange(int lo, int hi)
{
    Expression *expr = (Expression *)calloc(1,sizeof(Expression));
    expr->kind = RangeExpr;
    expr->start = lo;
    expr->end = hi+1;
    return expr;
}

void ExpressionFree(Expression *expr)
{
    free(expr->value);
    for (int i = 0; i < expr->count; i++)
        ExpressionFree(expr->children[i]);
    free(expr);
}

ExprKind ExpressionKind(Expression *expr)
{
    return expr->kind;
}

Expression *ExpressionChild(Expression *expr, int index)
{
    if ((index < 0) || (index >= expr->count))
        return NULL;
    return expr->children[index];
}

int ExpressionCount(Expression *expr)
{
    return expr->count;
}

const char *ExpressionValue(Expression *expr)
{
    return expr->value;
}

int ExpressionStart(Expression *expr)
{
    return expr->start;
}

int ExpressionEnd(Expression *expr)
{
    return expr->end;
}

static int ExprKindPrecedence(ExprKind kind)
{
    switch (kind) {
        case ChoiceExpr:
            return 1;
        case SequenceExpr:
            return 2;
        case AndExpr:
        case NotExpr:
            return 3;
        case OptExpr:
        case StarExpr:
        case PlusExpr:
            return 4;
        case IdentExpr:
        case LitExpr:
        case ClassExpr:
        case DotExpr:
            return 5;
        case RangeExpr:
            return 6;
    }
    return 0;
}

static void printEscapedRangeChar(char c)
{
    switch (c) {
        case '[':
            printf("\\[");
            break;
        case ']':
            printf("\\]");
            break;
        case '\\':
            printf("\\\\");
            break;
        default:
            printf("%c",c);
            break;
    }
}

static void printLiteral(const char *value)
{
    printf("'");
    for (int i = 0; value[i] != '\0'; i++) {
        switch (value[i]) {
            case '\r':
                printf("\\r");
                break;
            case '\n':
                printf("\\n");
                break;
            case '\t':
                printf("\\t");
                break;
            case '\'':
                printf("\\'");
                break;
            case '\\':
                printf("\\\\");
                break;
            default:
                printf("%c",value[i]);
        }
    }
    printf("'");
}

void ExpressionPrint(Expression *expr, int highestPrecedence, const char *indent)
{
    int exprPrecedence = ExprKindPrecedence(expr->kind);
    int brackets = (highestPrecedence > exprPrecedence); // e.g. a choice inside a sequence
    if (highestPrecedence < exprPrecedence)
        highestPrecedence = exprPrecedence;

    if ((expr->kind == ClassExpr) || (expr->kind == RangeExpr))
        brackets = 0;

    if (brackets) {
        printf("(");
        highestPrecedence = 1;
    }
    switch (expr->kind) {
        case ChoiceExpr:
            for (int i = 0; i < ExpressionCount(expr); i++) {
                if (i > 0) {
                    if (indent != NULL)
                        printf("\n%s/ ",indent);
                    else
                        printf(" / ");
                }
                ExpressionPrint(ExpressionChild(expr,i),highestPrecedence,NULL);
            }
            break;
        case SequenceExpr: {
            for (int i = 0; i < ExpressionCount(expr); i++) {
                if (i > 0) {
                    printf(" ");
                }
                ExpressionPrint(ExpressionChild(expr,i),highestPrecedence,NULL);
            }
            break;
        }
        case AndExpr:
            assert(ExpressionCount(expr) == 1);
            printf("&");
            ExpressionPrint(ExpressionChild(expr,0),highestPrecedence,NULL);
            break;
        case NotExpr:
            assert(ExpressionCount(expr) == 1);
            printf("!");
            ExpressionPrint(ExpressionChild(expr,0),highestPrecedence,NULL);
            break;
        case OptExpr:
            assert(ExpressionCount(expr) == 1);
            ExpressionPrint(ExpressionChild(expr,0),highestPrecedence,NULL);
            printf("?");
            break;
        case StarExpr:
            assert(ExpressionCount(expr) == 1);
            ExpressionPrint(ExpressionChild(expr,0),highestPrecedence,NULL);
            printf("*");
            break;
        case PlusExpr:
            assert(ExpressionCount(expr) == 1);
            ExpressionPrint(ExpressionChild(expr,0),highestPrecedence,NULL);
            printf("+");
            break;
        case IdentExpr:
            assert(ExpressionValue(expr) != NULL);
            printf("%s",ExpressionValue(expr));
            break;
        case LitExpr:
            assert(ExpressionValue(expr) != NULL);
            printLiteral(ExpressionValue(expr));
            break;
        case ClassExpr:
            printf("[");
            for (int i = 0; i < ExpressionCount(expr); i++) {
                assert(ExpressionKind(ExpressionChild(expr,i)) == RangeExpr);
                ExpressionPrint(ExpressionChild(expr,i),highestPrecedence,NULL);
            }
            printf("]");
            break;
        case DotExpr:
            printf(".");
            break;
        case RangeExpr: {
            int start = ExpressionStart(expr);
            int end = ExpressionEnd(expr);
            if (start+1 == end) {
                printEscapedRangeChar(start);
            }
            else {
                printEscapedRangeChar(start);
                printf("-");
                printEscapedRangeChar(end-1);
            }
            break;
        }
    }
    if (brackets)
        printf(")");
}
