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

#include "Parser.h"
#include "Util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef struct Parser Parser;

struct Parser {
    Grammar *gram;
    const char *input;
    int start;
    int end;
    int pos;
};

static Term *parseExpr(Parser *p, Expression *expr)
{
    int startPos = p->pos;
    switch (ExpressionKind(expr)) {
        case ChoiceExpr:
            for (int i = 0; i < ExprChoiceCount(expr); i++) {
                Term *term = parseExpr(p,ExprChoiceChildAt(expr,i));

                // If parsing of a choice succeeds, we return immediately.
                if (term != NULL)
                    return TermNew(expr,startPos,p->pos,TermListNew(term,NULL)); // Success

                // If parsing of a choice fails, we reset the current position, and continue on with
                // the next choice (if any). If there are no more choices, the loop complets and
                // we return NULL.
                p->pos = startPos;
            }

            // If we get here, none of the choices have matched, so evaluation of the
            // choice expression as a whole fails.
            return NULL; // Failure
        case SequenceExpr: {
            TermList *list = NULL;
            TermList **listEnd = &list;
            for (int i = 0; i < ExprSequenceCount(expr); i++) {
                Term *term = parseExpr(p,ExprSequenceChildAt(expr,i));

                // If parsing of a sequence item fails, the sequence as a whole fails. We reset
                // the current position to the start.
                if (term == NULL) {
                    p->pos = startPos;
                    return NULL; // Failure
                }

                // If parsing of a sequence item succeeds, we append it to the list of
                // accumulated child terms, and continue with the next item.
                TermListPtrAppend(&listEnd,term);
            }

            // If we get here, all items in the sequence have matched, and evaluation succeeds,
            // returning a term with children comprising the parse results of all items.
            return TermNew(expr,startPos,p->pos,list); // Success
        }
        case AndExpr: {
            // Evaluate the child expression to see if it succeeds, but reset the position since
            // this is a predicate which is not supposed to consume any input. AndExpr is a
            // positive lookahead assertion, which means it succeeds if and only if the child
            // evaluation succeeds.
            Term *term = parseExpr(p,ExprAndChild(expr));
            p->pos = startPos;
            if (term != NULL)
                return TermNew(expr,startPos,startPos,NULL); // Success
            else
                return NULL; // Failure
        }
        case NotExpr: {
            // Evaluate the child expression to see if it succeeds, but reset the position since
            // this is a predicate which is not supposed to consume any input. NotExpr is a
            // *negative* lookahead assertion, which is the opposite of the above; it succeeds
            // if and only if the child evaluation *fails*.
            Term *term = parseExpr(p,ExprNotChild(expr));
            p->pos = startPos;
            if (term != NULL)
                return NULL; // Failure
            else
                return TermNew(expr,startPos,startPos,NULL); // Success
        }
        case OptExpr: {
            // An optional expression (? operator) succeeds regardless of whether or not the child
            // expression succeeds. If the child did succeed, we return its result as a child of the
            // OptExpr result.
            Term *term = parseExpr(p,ExprOptChild(expr));
            TermList *children;
            if (term != NULL)
                children = TermListNew(term,NULL);
            else
                children = NULL;
            return TermNew(expr,startPos,p->pos,children); // Success
        }
        case StarExpr: {
            // A zero-or-more expression (* operator) repeatedly matches is child as many times
            // as possible, suceeding regardless of the number of matches.
            TermList *list = NULL;
            TermList **listEnd = &list;
            for (;;) {
                Term *term = parseExpr(p,ExprStarChild(expr));
                if (term == NULL)
                    break;
                TermListPtrAppend(&listEnd,term);
            }
            return TermNew(expr,startPos,p->pos,list);
        }
        case PlusExpr: {
            // A one-or-more expression (+ operator) operates like a zero-or-match, but fails if
            // there is not at least one match
            TermList *list = NULL;
            TermList **listEnd = &list;

            // First make sure we have at least one match. If this parse fails then we have zero
            // matches, and thus the plus expression as a whole fails.
            Term *term = parseExpr(p,ExprPlusChild(expr));
            if (term == NULL)
                return NULL; // Failure
            TermListPtrAppend(&listEnd,term);

            // Now parse any following matches
            for (;;) {
                Term *term = parseExpr(p,ExprPlusChild(expr));
                if (term == NULL)
                    break;
                TermListPtrAppend(&listEnd,term);
            }
            return TermNew(expr,startPos,p->pos,list); // Success
        }
        case IdentExpr: {
            Term *term = parseExpr(p,ExprIdentTarget(expr));
            if (term != NULL)
                return TermNew(expr,startPos,p->pos,TermListNew(term,NULL));
            else
                return NULL;
        }
        case LitExpr: {
            const char *value = ExprLitValue(expr);
            int len = (int)strlen(value);
            if ((p->pos + len <= p->end) && !memcmp(&p->input[p->pos],value,len)) {
                p->pos += len;
                return TermNew(expr,startPos,p->pos,NULL);
            }
            else {
                return NULL;
            }
        }
        case ClassExpr:
            // Actually identical to ChoiceExpr; we should really merge the two
            for (int i = 0; i < ExprClassCount(expr); i++) {
                Term *term = parseExpr(p,ExprClassChildAt(expr,i));

                // If parsing of a choice succeeds, we return immediately.
                if (term != NULL)
                    return TermNew(expr,startPos,p->pos,TermListNew(term,NULL)); // Success

                // If parsing of a choice fails, we reset the current position, and continue on with
                // the next choice (if any). If there are no more choices, the loop complets and
                // we return NULL.
                p->pos = startPos;
            }

            // If we get here, none of the choices have matched, so evaluation of the
            // choice expression as a whole fails.
            return NULL; // Failure
        case DotExpr: {
            size_t offset = p->pos;
            int c = UTF8NextChar(p->input,&offset); // FIXME: Should use uint32_t here
            if (c == 0)
                return NULL;
            p->pos = offset;
            return TermNew(expr,startPos,p->pos,NULL);
        }
        case RangeExpr: {
            size_t offset = p->pos;
            int c = UTF8NextChar(p->input,&offset); // FIXME: Should use uint32_t here
            if (c == 0)
                return NULL;
            if ((c >= ExprRangeStart(expr)) && (c < ExprRangeEnd(expr))) {
                p->pos = offset;
                return TermNew(expr,startPos,p->pos,NULL);
            }
            else {
                return NULL;
            }
        }
    }
    assert(!"unknown expression type");
    return NULL;
}

Term *parse(Grammar *gram, const char *rule, const char *input, int start, int end)
{
    Expression *rootExpr = GrammarLookup(gram,rule);
    if (rootExpr == NULL) {
        fprintf(stderr,"%s: No such rule\n",rule);
        exit(1);
    }

    Parser *p = (Parser *)calloc(1,sizeof(Parser));
    p->gram = gram;
    p->input = input;
    p->start = start;
    p->end = end;
    p->pos = start;
    Term *result = parseExpr(p,rootExpr);
    free(p);
    return result;
}
