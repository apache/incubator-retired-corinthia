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

#include "Grammar.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef struct Rule Rule;

struct Rule {
    char *name;
    Expression *expr;
    Rule *next;
};

struct Grammar {
    Rule **nextDef;
    Rule *defList;
};

Rule *RuleNew(const char *name, Expression *expr)
{
    Rule *def = (Rule *)calloc(1,sizeof(Rule));
    def->name = strdup(name);
    def->expr = expr;
    return def;
}

void RuleFree(Rule *def)
{
    free(def->name);
    ExpressionFree(def->expr);
    free(def);
}

Grammar *GrammarNew(void)
{
    Grammar *gram = (Grammar *)calloc(1,sizeof(Grammar));
    gram->nextDef = &gram->defList;
    return gram;
}

void GrammarFree(Grammar *gram)
{
    Rule *next;
    for (Rule *def = gram->defList; def != NULL; def = next) {
        next = def->next;
        RuleFree(def);
    }
    free(gram);
}

void GrammarDefine(Grammar *gram, const char *name, Expression *expr)
{
    Rule *def = RuleNew(name,expr);
    *gram->nextDef = def;
    gram->nextDef = &def->next;
}

Expression *GrammarLookup(Grammar *gram, const char *name)
{
    for (Rule *def = gram->defList; def != NULL; def = def->next) {
        if (!strcmp(def->name,name))
            return def->expr;
    }
    return NULL;
}

static void GrammarResolveRecursive(Grammar *gram, Expression *expr, const char *ruleName)
{
    if (ExpressionKind(expr) == IdentExpr) {
        const char *targetName = ExprIdentValue(expr);
        Expression *targetExpr = GrammarLookup(gram,targetName);
        if (targetExpr == NULL) {
            fprintf(stderr,"%s: Cannot resolve reference %s\n",ruleName,targetName);
            exit(1);
        }
        ExprIdentSetTarget(expr,targetExpr);
    }
    else {
        for (int i = 0; i < ExpressionCount(expr); i++)
            GrammarResolveRecursive(gram,ExpressionChildAt(expr,i),ruleName);
    }
}

void GrammarResolve(Grammar *gram)
{
    for (Rule *def = gram->defList; def != NULL; def = def->next)
        GrammarResolveRecursive(gram,def->expr,def->name);
}

void GrammarPrint(Grammar *gram)
{
    int maxNameLen = 0;
    for (Rule *def = gram->defList; def != NULL; def = def->next) {
        int nameLen = strlen(def->name);
        if (maxNameLen < nameLen)
            maxNameLen = nameLen;
    }

    char *prefix = malloc(maxNameLen+3);
    memset(prefix,' ',maxNameLen+2);
    prefix[maxNameLen+2] = '\0';

    for (Rule *def = gram->defList; def != NULL; def = def->next) {
        int nameLen = strlen(def->name);
        printf("%s",def->name);
        for (int i = nameLen; i < maxNameLen+1; i++)
            printf(" ");
        printf("<- ");
        ExpressionPrint(def->expr,0,prefix);
        printf("\n");
    }

    free(prefix);
}
