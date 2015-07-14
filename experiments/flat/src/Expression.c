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
#include "Expression.h"
#include "Util.h"
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

const char *ExprKindAsString(ExprKind kind)
{
    switch (kind) {
        case ChoiceExpr:
            return "Choice";
        case SequenceExpr:
            return "Sequence";
        case AndExpr:
            return "And";
        case NotExpr:
            return "Not";
        case OptExpr:
            return "Opt";
        case StarExpr:
            return "Star";
        case PlusExpr:
            return "Plus";
        case IdentExpr:
            return "Ident";
        case LitExpr:
            return "Lit";
        case ClassExpr:
            return "Class";
        case DotExpr:
            return "Dot";
        case RangeExpr:
            return "Range";
        default:
            return "?";
    }
}

Expression *ExpressionNewChoice(int count, Expression **children)
{
    for (int i = 0; i < count; i++)
        assert(children[i] != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+count*sizeof(Expression *));
    expr->kind = ChoiceExpr;
    expr->count = count;
    memcpy(expr->children,children,count*sizeof(Expression *));
    return expr;
}

Expression *ExpressionNewSequence(int count, Expression **children)
{
    for (int i = 0; i < count; i++)
        assert(children[i] != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+count*sizeof(Expression *));
    expr->kind = SequenceExpr;
    expr->count = count;
    memcpy(expr->children,children,count*sizeof(Expression *));
    return expr;
}

Expression *ExpressionNewAnd(Expression *child)
{
    assert(child != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+1*sizeof(Expression *));
    expr->kind = AndExpr;
    expr->count = 1;
    expr->children[0] = child;
    return expr;
}

Expression *ExpressionNewNot(Expression *child)
{
    assert(child != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+1*sizeof(Expression *));
    expr->kind = NotExpr;
    expr->count = 1;
    expr->children[0] = child;
    return expr;
}

Expression *ExpressionNewOpt(Expression *child)
{
    assert(child != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+1*sizeof(Expression *));
    expr->kind = OptExpr;
    expr->count = 1;
    expr->children[0] = child;
    return expr;
}

Expression *ExpressionNewStar(Expression *child)
{
    assert(child != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+1*sizeof(Expression *));
    expr->kind = StarExpr;
    expr->count = 1;
    expr->children[0] = child;
    return expr;
}

Expression *ExpressionNewPlus(Expression *child)
{
    assert(child != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+1*sizeof(Expression *));
    expr->kind = PlusExpr;
    expr->count = 1;
    expr->children[0] = child;
    return expr;
}

Expression *ExpressionNewIdent(const char *ident)
{
    assert(ident != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+1*sizeof(Expression *));
    expr->kind = IdentExpr;
    expr->value = strdup(ident);
    expr->count = 1;
    expr->children[0] = NULL;
    return expr;
}

Expression *ExpressionNewLit(const char *value)
{
    assert(value != NULL);
    Expression *expr = (Expression *)calloc(1,sizeof(Expression));
    expr->kind = LitExpr;
    expr->value = strdup(value);
    expr->count = 0;
    return expr;
}

Expression *ExpressionNewClass(int count, Expression **children)
{
    for (int i = 0; i < count; i++) {
        assert(children[i] != NULL);
        assert(children[i]->kind == RangeExpr);
    }
    Expression *expr = (Expression *)calloc(1,sizeof(Expression)+count*sizeof(Expression *));
    expr->kind = ClassExpr;
    expr->count = count;
    memcpy(expr->children,children,count*sizeof(Expression *));
    return expr;
}

Expression *ExpressionNewDot(void)
{
    Expression *expr = (Expression *)calloc(1,sizeof(Expression));
    expr->kind = DotExpr;
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
    if (expr == NULL)
        return;
    free(expr->value);
    // Don't free children of IdentExpr, since these are expressions referenced by grammar
    // rules, which will be freed separately. We can't use reference counting here as there
    // will generally by cycles.
    if (expr->kind != IdentExpr) {
        for (int i = 0; i < expr->count; i++)
            ExpressionFree(expr->children[i]);
    }
    free(expr);
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
            for (int i = 0; i < ExprChoiceCount(expr); i++) {
                if (i > 0) {
                    if (indent != NULL)
                        printf("\n%s/ ",indent);
                    else
                        printf(" / ");
                }
                ExpressionPrint(ExprChoiceChildAt(expr,i),highestPrecedence,NULL);
            }
            break;
        case SequenceExpr: {
            for (int i = 0; i < ExprSequenceCount(expr); i++) {
                if (i > 0) {
                    printf(" ");
                }
                ExpressionPrint(ExprSequenceChildAt(expr,i),highestPrecedence,NULL);
            }
            break;
        }
        case AndExpr:
            printf("&");
            ExpressionPrint(ExprAndChild(expr),highestPrecedence,NULL);
            break;
        case NotExpr:
            printf("!");
            ExpressionPrint(ExprNotChild(expr),highestPrecedence,NULL);
            break;
        case OptExpr:
            ExpressionPrint(ExprOptChild(expr),highestPrecedence,NULL);
            printf("?");
            break;
        case StarExpr:
            ExpressionPrint(ExprStarChild(expr),highestPrecedence,NULL);
            printf("*");
            break;
        case PlusExpr:
            ExpressionPrint(ExprPlusChild(expr),highestPrecedence,NULL);
            printf("+");
            break;
        case IdentExpr:
            printf("%s",ExprIdentValue(expr));
            break;
        case LitExpr:
            printLiteral(ExprLitValue(expr));
            break;
        case ClassExpr:
            printf("[");
            for (int i = 0; i < ExprClassCount(expr); i++) {
                ExpressionPrint(ExprClassChildAt(expr,i),highestPrecedence,NULL);
            }
            printf("]");
            break;
        case DotExpr:
            printf(".");
            break;
        case RangeExpr: {
            int start = ExprRangeStart(expr);
            int end = ExprRangeEnd(expr);
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

ExprKind ExpressionKind(Expression *expr)
{
    return expr->kind;
}

int ExpressionCount(Expression *expr)
{
    return expr->count;
}

Expression *ExpressionChildAt(Expression *expr, int index)
{
    if ((index < 0) || (index >= expr->count))
        return NULL;
    return expr->children[index];
}

// Choice

int ExprChoiceCount(Expression *expr)
{
    assert(expr->kind == ChoiceExpr);
    return expr->count;
}

Expression *ExprChoiceChildAt(Expression *expr, int index)
{
    assert(expr->kind == ChoiceExpr);
    assert(index >= 0);
    assert(index < expr->count);
    return expr->children[index];
}

// Sequence

int ExprSequenceCount(Expression *expr)
{
    assert(expr->kind == SequenceExpr);
    return expr->count;
}

Expression *ExprSequenceChildAt(Expression *expr, int index)
{
    assert(expr->kind == SequenceExpr);
    assert(index >= 0);
    assert(index < expr->count);
    return expr->children[index];
}

// And, Not, Opt, Star, Plus

Expression *ExprAndChild(Expression *expr)
{
    assert(expr->kind == AndExpr);
    assert(expr->count == 1);
    assert(expr->children[0] != NULL);
    return expr->children[0];
}

Expression *ExprNotChild(Expression *expr)
{
    assert(expr->kind == NotExpr);
    assert(expr->count == 1);
    assert(expr->children[0] != NULL);
    return expr->children[0];
}

Expression *ExprOptChild(Expression *expr)
{
    assert(expr->kind == OptExpr);
    assert(expr->count == 1);
    assert(expr->children[0] != NULL);
    return expr->children[0];
}

Expression *ExprStarChild(Expression *expr)
{
    assert(expr->kind == StarExpr);
    assert(expr->count == 1);
    assert(expr->children[0] != NULL);
    return expr->children[0];
}

Expression *ExprPlusChild(Expression *expr)
{
    assert(expr->kind == PlusExpr);
    assert(expr->count == 1);
    assert(expr->children[0] != NULL);
    return expr->children[0];
}

// Ident, Lit

const char *ExprIdentValue(Expression *expr)
{
    assert(expr->kind == IdentExpr);
    return expr->value;
}

Expression *ExprIdentTarget(Expression *expr)
{
    assert(expr->kind == IdentExpr);
    assert(expr->count == 1);
    assert(expr->children[0] != NULL);
    return expr->children[0];
}

void ExprIdentSetTarget(Expression *expr, Expression *target)
{
    assert(expr->kind == IdentExpr);
    assert(expr->count == 1);
    assert(expr->children[0] == NULL);
    expr->children[0] = target;
}

const char *ExprLitValue(Expression *expr)
{
    assert(expr->kind == LitExpr);
    return expr->value;
}

// Class

int ExprClassCount(Expression *expr)
{
    assert(expr->kind == ClassExpr);
    return expr->count;
}

Expression *ExprClassChildAt(Expression *expr, int index)
{
    assert(expr->kind == ClassExpr);
    assert(index >= 0);
    assert(index < expr->count);
    return expr->children[index];
}

// Range

int ExprRangeStart(Expression *expr)
{
    assert(expr->kind == RangeExpr);
    return expr->start;
}

int ExprRangeEnd(Expression *expr)
{
    assert(expr->kind == RangeExpr);
    return expr->end;
}
