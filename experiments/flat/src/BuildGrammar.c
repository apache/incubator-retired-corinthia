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
#include "BuildGrammar.h"
#include "Util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAX_CHILDREN 64

typedef struct Builder Builder;

struct Builder {
    Grammar *gram;
    const char *input;
    int len;
};

static Expression *buildExpression(Builder *builder, Term *term);

static char *unescapeLiteral(const char *escaped)
{
    size_t escapedLen = strlen(escaped);
    size_t escapedPos = 0;
    size_t unescapedLen = 0;
    char *unescaped = (char *)malloc(escapedLen+1);

    while (escapedPos < escapedLen) {
        char c = escaped[escapedPos++];
        if ((c == '\\') && (escapedPos < escapedLen)) {
            c = escaped[escapedPos++];
            switch (c) {
                case 'n':
                    unescaped[unescapedLen++] = '\n';
                    break;
                case 'r':
                    unescaped[unescapedLen++] = '\r';
                    break;
                case 't':
                    unescaped[unescapedLen++] = '\t';
                    break;
                default:
                    unescaped[unescapedLen++] = c;
                    break;
            }
        }
        else {
            unescaped[unescapedLen++] = c;
        }
    }

    unescaped[unescapedLen] = '\0';
    return unescaped;
}

// Sanity checking functions
//
// These are just used to verify that a Term we're about to use is of the expected type.
// Since the format of the parse tree is likely to change in these early stages of development,
// this will catch cases where the code in this file has not been adapted to those changes
//
// The large number of assert statements in this file, most of which use these functions, will
// likely only be necessary in the short term. Ideally we should be able to get to a point where
// one can safely write code that consumes a parse tree without having so many sanity checks.

static int isTerm(Term *term, ExprKind kind, int count)
{
    return ((TermKind(term) == kind) && (TermCount(term) == count));
}

static int isSequence(Term *term, int count)
{
    return isTerm(term,SequenceExpr,count);
}

static int isIdent(Term *term, const char *name)
{
    return (isTerm(term,IdentExpr,1) && !strcmp(name,ExprIdentValue(TermType(term))));
}

// Extract a substring of the input based on a specific term, or one or more children of a term.
//
// Each Term object has a start and end field, which specify which portion of the input string
// the term covers (that is, the portion of the input consumed durin parsing of that term). In
// some cases we want all the text matched by a term, and in others we only want the text matched
// by specific children. An example of the latter is a "pseudo-terminal" like Identifier where
// we want all characters except the trailing whitespace.

static char *termString(Builder *builder, Term *term)
{
    assert(term->start >= 0);
    assert(term->end <= builder->len);
    int len = term->end - term->start;
    char *str = malloc(len+1);
    memcpy(str,&builder->input[term->start],len);
    str[len] = '\0';
    return str;
}

static char *identifierString(Builder *builder, Term *term)
{
    assert(isIdent(term,"Identifier"));
    Term *body = TermChildAt(term,0);
    assert(isSequence(body,2));
    Term *content = TermChildAt(body,0);
    Term *spacing = TermChildAt(body,1);
    assert(TermKind(content) == StringExpr);
    assert(isIdent(spacing,"Spacing"));
    return termString(builder,content);
}

// For Terms of type ChoiceExpr, determine which of the choices the corresponding term is

static int choiceIndex(Term *body)
{
    assert(TermKind(body) == ChoiceExpr);
    assert(TermCount(body) == 1);
    Term *choice = TermChildAt(body,0);
    int match = -1;
    int choiceCount = ExprChoiceCount(TermType(body));
    for (int i = 0; i < choiceCount; i++) {
        if (TermType(choice) == ExpressionChildAt(TermType(body),i))
            match = i;
    }
    return match;
}

// Expression building functions (build*)
//
// There is one of these for each type of expression that can be present in the grammar. The
// supplied Term object is, in all cases, of an IdentExpr type, whose single child is the body
// of the corresponding grammar rule.
//
// For example, buildPrimary is called with an ExpressionType of IdentExpr "Primary", and the
// body is a ChoiceExpr which can contain one of five possible child types, as given in the
// PEG grammar.
//
// To see the exact expression tree of a given rule, look at its definition in Builtin.c

static Expression *buildIdentifier(Builder *builder, Term *term)
{
    char *str = identifierString(builder,term);
    Expression *result = ExpressionNewIdent(str);
    free(str);
    return result;
}

static Expression *buildLiteral(Builder *builder, Term *term)
{
    // The Literal rule in the built-in PEG grammar contains two choices - one for single quotes
    // and the other for double quotes - which otherwise have the same structure. There are four
    // children, where child 0 and child 2 represent the quotes themselves, child 1 represents the
    // escaped representation of a string, and child 3 is any trailing whitespace.
    //
    // All we're interested in is the unescaped string, so we get child 2 and convert any escape
    // sequences (e.g. \" or \n) to the characters they represent, and return a new literal
    // expression with the unescaped string as its value.

    assert(isIdent(term,"Literal"));
    Term *body = TermChildAt(term,0);
    assert(TermKind(body) == ChoiceExpr);
    int index = choiceIndex(body);
    assert((index == 0) || (index == 1));
    Term *choice = TermChildAt(body,0);
    assert(isSequence(choice,4));
    Term *content = TermChildAt(choice,1);
    char *escaped = termString(builder,content);
    char *unescaped = unescapeLiteral(escaped);
    Expression *result = ExpressionNewLit(unescaped);
    free(escaped);
    free(unescaped);
    return result;
}

static int decodeRangeChar(Builder *builder, Term *charTerm)
{
    // FIXME: Handle non-ASCII chars encoded as UTF-8, as well as numeric escape sequences
    // We don't need to allocate a string for unescaped here, we should just do it directly

    assert(isIdent(charTerm,"Char"));
    char *escaped = termString(builder,charTerm);
    char *unescaped = unescapeLiteral(escaped);
    int result;
    if (strlen(unescaped) == 0)
        result = '?';
    else if (unescaped[0] < 0) // Start of UTF-8 multibyte sequence
        result = '?';
    else
        result = unescaped[0];
    free(escaped);
    free(unescaped);
    return result;
}

static Expression *buildRange(Builder *builder, Term *term)
{
    // A range expression, one or more of which appear inside a [...] character class expression,
    // matches a character in a range between a start and end value. We use a single expression
    // type to represent both exact matches (where the minimum and maximum are the same), and
    // "true" ranges (which match two or more possible characters).
    //
    // Note that the representation we use for Range expressions is a (start,end) pair, where a
    // character match must satisfy the condition start <= c < end (that is, the range includes the
    // start, but does *not* include the end). This representation is for the convenience of other
    // code that works with ranges. The ExpressionNewRange function however takes the minimum and
    // maximum values - that is, a matching character c must satisfy min <= c <= max.

    assert(isIdent(term,"Range"));
    Term *body = TermChildAt(term,0);
    int index = choiceIndex(body);
    Term *choice = TermChildAt(body,0);
    assert((index == 0) || (index == 1));
    if (index == 0) {
        assert(isSequence(choice,3));
        Term *minChar = TermChildAt(choice,0);
        Term *maxChar = TermChildAt(choice,2);
        assert(isIdent(minChar,"Char"));
        assert(isIdent(maxChar,"Char"));
        int min = decodeRangeChar(builder,minChar);
        int max = decodeRangeChar(builder,maxChar);
        return ExpressionNewRange(min,max);
    }
    else {
        assert(isIdent(choice,"Char"));
        int value = decodeRangeChar(builder,choice);
        return ExpressionNewRange(value,value);
    }
}

static Expression *buildClass(Builder *builder, Term *term)
{
    // A character class expression is the same as a choice expression, except that it is only
    // supposed to contain range expressions.

    assert(isIdent(term,"Class"));
    Term *body = TermChildAt(term,0);
    assert(isSequence(body,4));
    Term *star = TermChildAt(body,1);
    assert(TermKind(star) == StarExpr);

    Expression *children[MAX_CHILDREN];
    int count = 0;
    for (TermList *item = TermChildren(star); (item != NULL) && (count < MAX_CHILDREN); item = item->next) {
        assert(isSequence(item->term,2));
        Term *rangeTerm = TermChildAt(item->term,1);
        Expression *rangeExpr = buildRange(builder,rangeTerm);
        children[count++] = rangeExpr;
    }

    return ExpressionNewClass(count,children);
}

static Expression *buildDot(Builder *builder, Term *term)
{
    assert(isIdent(term,"DOT"));
    return ExpressionNewDot();
}

static Expression *buildPrimary(Builder *builder, Term *term)
{
    // A primary expression can be one of five possibilities: An identifier (reference to another
    // rule in the grammar), a parenthesised expresion, a literal, a character class, or a dot
    // (which matches any character).
    //
    // The type of the term is an Expression with kind == ChoiceExpr, and the term has exactly
    // one child. We use choiceIndex to determine which of the five possible expression types
    // the choice matches, and then call through to the relevant function to build the appropriate
    // type of Expression object.

    assert(isIdent(term,"Primary"));
    Term *body = TermChildAt(term,0);
    assert(TermKind(body) == ChoiceExpr);
    assert(TermCount(body) == 1);
    Term *choice = TermChildAt(body,0);

    switch (choiceIndex(body)) {
        case 0: {
            assert(isSequence(choice,2));
            Term *identifier = TermChildAt(choice,0);
            return buildIdentifier(builder,identifier);
        }
        case 1: {
            assert(isSequence(choice,4));
            Term *expression = TermChildAt(choice,2);
            Expression *result = buildExpression(builder,expression);
            return ExpressionNewString(result);
        }
        case 2: {
            assert(isSequence(choice,3));
            Term *expression = TermChildAt(choice,1);
            return buildExpression(builder,expression);
        }
        case 3:
            return buildLiteral(builder,choice);
        case 4:
            return buildClass(builder,choice);
        case 5:
            return buildDot(builder,choice);
        default:
            assert(!"Invalid choice for Primary");
            return NULL;
    }
}

static Expression *buildSuffix(Builder *builder, Term *term)
{
    // A suffix expression has two children. The second is optional, and is one of either QUESTION,
    // STAR, or PLUS - which indicate that the first child may occur zero or one times, zero or more
    // times, or one or more times. The first child is always present, and represents a primary
    // expression with no prefix or suffix.
    //
    // If a QUESTION, STAR, or PLUS suffix is given, we wrap the constructed primary expression
    // inside another Expression object of type OptExpr, StarExpr, or PlusExpr. Otherwise, we just
    // return the primary expression directly.

    assert(isIdent(term,"Suffix"));
    Term *body = TermChildAt(term,0);
    assert(isSequence(body,2));
    Term *primary = TermChildAt(body,0);
    Term *suffix = TermChildAt(body,1);
    assert(isIdent(primary,"Primary"));
    assert(TermKind(suffix) == OptExpr);

    Expression *primaryExpr = buildPrimary(builder,primary);

    assert((TermCount(suffix) == 0) || (TermCount(suffix) == 1));
    if (TermCount(suffix) == 1) {
        Term *suffixChild = TermChildAt(suffix,0);
        int index = choiceIndex(suffixChild);
        Term *choice = TermChildAt(suffixChild,0);
        switch (index) {
            case 0:
                assert(isIdent(choice,"QUESTION"));
                return ExpressionNewOpt(primaryExpr);
            case 1:
                assert(isIdent(choice,"STAR"));
                return ExpressionNewStar(primaryExpr);
            case 2:
                assert(isIdent(choice,"PLUS"));
                return ExpressionNewPlus(primaryExpr);
            default:
                assert(!"Invalid choice for Suffix");
                break;
        }
    }

    return primaryExpr;
}

static Expression *buildPrefix(Builder *builder, Term *term)
{
    // A prefix expression has two children. The first is optional, and is one of either AND or
    // NOT - which indicate a positive or negative lookahead assertion. The second is not optional,
    // and represents a primary expression with an optional suffix.
    //
    // If an AND or NOT prefx is given, we wrap the constructed primary expression inside another
    // Expression object of type AndExpr or NotExpr. Otherwise, we just return the primary
    // expression directly.

    assert(isIdent(term,"Prefix"));
    Term *body = TermChildAt(term,0);
    assert(isSequence(body,2));
    Term *prefix = TermChildAt(body,0);
    Term *suffix = TermChildAt(body,1);
    assert(TermKind(prefix) == OptExpr);
    assert(isIdent(suffix,"Suffix"));

    Expression *suffixExpr = buildSuffix(builder,suffix);

    assert((TermCount(prefix) == 0) || (TermCount(prefix) == 1));
    if (TermCount(prefix) == 1) {
        Term *prefixChild = TermChildAt(prefix,0);
        int index = choiceIndex(prefixChild);
        Term *choice = TermChildAt(prefixChild,0);
        switch (index) {
            case 0:
                assert(isIdent(choice,"AND"));
                return ExpressionNewAnd(suffixExpr);
            case 1:
                assert(isIdent(choice,"NOT"));
                return ExpressionNewNot(suffixExpr);
            default:
                assert(!"Invalid choice for Prefix");
                break;
        }
    }

    return suffixExpr;
}

static Expression *buildSequence(Builder *builder, Term *term)
{
    // An Sequence consists of one or more expressions, each of which has an optional prefix
    // and suffix
    assert(isIdent(term,"Sequence"));
    Term *body = TermChildAt(term,0);
    assert(TermKind(body) == StarExpr);

    Expression *children[MAX_CHILDREN];
    int count = 0;
    for (TermList *item = TermChildren(body); (item != NULL) && (count < MAX_CHILDREN); item = item->next) {
        children[count] = buildPrefix(builder,item->term);
        count++;
    }
    if (count == 1)
        return children[0];
    else
        return ExpressionNewSequence(count,children);
}

static Expression *buildExpression(Builder *builder, Term *term)
{
    // An Expression consists of one or more choices
    assert(isIdent(term,"Expression"));
    Term *body = TermChildAt(term,0);
    assert(isSequence(body,2));

    Term *child0 = TermChildAt(body,0);
    Term *child1 = TermChildAt(body,1);

    assert(isIdent(child0,"Sequence"));
    assert(TermKind(child1) == StarExpr);

    Expression *initial = buildSequence(builder,child0);
    if (TermCount(child1) == 0)
        return initial;

    Expression *children[MAX_CHILDREN];
    children[0] = initial;
    int count = 1;
    for (TermList *item = TermChildren(child1); (item != NULL) && (count < MAX_CHILDREN); item = item->next) {
        assert(isSequence(item->term,2));
        children[count] = buildSequence(builder,TermChildAt(item->term,1));
        count++;
    }
    return ExpressionNewChoice(count,children);
}

static void buildGrammar(Builder *builder, Term *term)
{
    assert(isSequence(term,3));
    Term *plus = TermChildAt(term,1);
    assert(TermKind(plus) == PlusExpr);

    for (TermList *plusItem = plus->children; plusItem != NULL; plusItem = plusItem->next) {
        Term *defIdent = plusItem->term;
        assert(isIdent(defIdent,"Definition"));
        Term *defSeq = TermChildAt(defIdent,0);
        assert(isSequence(defSeq,3));
        Term *identTerm = TermChildAt(defSeq,0);
        Term *exprTerm = TermChildAt(defSeq,2);
        assert(isIdent(identTerm,"Identifier"));
        assert(isIdent(exprTerm,"Expression"));

        char *ruleName = identifierString(builder,identTerm);
        Expression *ruleExpr = buildExpression(builder,exprTerm);
        GrammarDefine(builder->gram,ruleName,ruleExpr);
        free(ruleName);
    }
}

// This function creates a new Grammar object from the result of parsing a file usin the built-in
// PEG grammar. The resulting Grammar object can then be used to parse other files which are written
// in another language that is accepted by that grammar. The Term objects are constructed by the
// parse function defined in Parser.c.

Grammar *grammarFromTerm(Term *term, const char *input)
{
    Grammar *gram = GrammarNew();

    Builder *builder = (Builder *)calloc(1,sizeof(Builder));
    builder->gram = gram;
    builder->input = input;
    builder->len = strlen(input);

    buildGrammar(builder,term);

    free(builder);

    return gram;
}
