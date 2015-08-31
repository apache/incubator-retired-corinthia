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
    Rule **nextRule;
    Rule *ruleList;
};

Rule *RuleNew(const char *name, Expression *expr)
{
    Rule *rule = (Rule *)calloc(1,sizeof(Rule));
    rule->name = strdup(name);
    rule->expr = expr;
    return rule;
}

void RuleFree(Rule *rule)
{
    free(rule->name);
    ExpressionFree(rule->expr);
    free(rule);
}

Grammar *GrammarNew(void)
{
    Grammar *gram = (Grammar *)calloc(1,sizeof(Grammar));
    gram->nextRule = &gram->ruleList;
    return gram;
}

void GrammarFree(Grammar *gram)
{
    Rule *next;
    for (Rule *rule = gram->ruleList; rule != NULL; rule = next) {
        next = rule->next;
        RuleFree(rule);
    }
    free(gram);
}

void GrammarDefine(Grammar *gram, const char *name, Expression *expr)
{
    Rule *rule = RuleNew(name,expr);
    *gram->nextRule = rule;
    gram->nextRule = &rule->next;
}

Expression *GrammarLookup(Grammar *gram, const char *name)
{
    for (Rule *rule = gram->ruleList; rule != NULL; rule = rule->next) {
        if (!strcmp(rule->name,name))
            return rule->expr;
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
    for (Rule *rule = gram->ruleList; rule != NULL; rule = rule->next)
        GrammarResolveRecursive(gram,rule->expr,rule->name);
}

void GrammarPrint(Grammar *gram, int exprAsTree)
{
    int maxNameLen = 0;
    for (Rule *rule = gram->ruleList; rule != NULL; rule = rule->next) {
        int nameLen = strlen(rule->name);
        if (maxNameLen < nameLen)
            maxNameLen = nameLen;
    }

    char *prefix = malloc(maxNameLen+2);
    memset(prefix,' ',maxNameLen+1);
    prefix[maxNameLen+1] = '\0';

    for (Rule *def = gram->ruleList; def != NULL; def = def->next) {
        int nameLen = strlen(def->name);
        printf("%s",def->name);
        if (exprAsTree) {
            // Print the expression in tree format, with one line per node
            printf("\n");
            printf("\n");
            printf("    ");
            ExpressionPrintTree(def->expr,"    ",4);
            printf("\n");
        }
        else {
            // Print the expression in compact format, with one line per alternative
            for (int i = nameLen; i < maxNameLen+1; i++)
                printf(" ");
            printf(": ");
            ExpressionPrint(def->expr,0,prefix);
            printf(";\n");
        }
    }

    free(prefix);
}

const char *GrammarFirstRuleName(Grammar *gram)
{
    if (gram->ruleList == NULL)
        return NULL;
    else
        return gram->ruleList->name;
}
