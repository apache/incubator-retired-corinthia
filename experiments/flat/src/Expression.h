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

#pragma once

/**
 * A tree of Expression objects represents the abstract syntax tree that comprises part of a
 * PEG (Parsing Expression Grammar). Each grammar consists of one or more named rules, each
 * of which has an expression tree associated with it. The format of the grammars is described
 * in the following paper:
 *
 *     "Parsing Expression Grammars: A Recognition-Based Syntactic Foundation".
 *     Bryan Ford, POPL, January 2004. http://bford.info/pub/lang/peg.pdf
 *
 * An Expression can be used by a code generator to produce the code for a parser, or alternatively,
 * used directly to implement an interpreter to produce a parse tree based on the rules of the
 * grammar.
 *
 * The types are as follows:
 *
 * ChoiceExpr - matches exactly one of the child expressions against the input. Each expression
 * is tested in order, until a match is found, with the result being that of the child parse.
 * Evaluation succeeds if one of the children matched.
 *
 * SequenceExpr - matches each of the child expressions, in order, following consecutively in the
 * input. Evaluation succeeds if all children matched.
 *
 * AndExpr - Positive lookahead assertion. The single child expression is tested against the input,
 * and evaluation succeeds if there is a match. Afterwards, the current position moved back to the
 * location in the input where the lookahead test began.
 *
 * NotExpr - Negative lookahead assertion. The single child expression is tested against the input,
 * and evaluation succeeds if the child *fails* to match. Afterwards, the current position moved
 * back to the location in the input where the lookahead test began.
 *
 * OptExpr - The input is consumed if it matches, otherwise the position is left unchanged. In
 * either case, the evaluation succeeds.
 *
 * StarExpr - The child expression is repeatedly evaluated against the input until no more matches
 * occur. The evaluation always suceeds. The result is a list of 1 or more nodes.
 *
 * PlusExpr - The child expression is repeatedly evaluated against the input until no more matches
 * occur. The evaluation always suceeds if at least one match was made. The result is a list of
 * 1 or more nodes.
 *
 * IdentExpr - Reference to another rule defined in the grammar. Always has a value, representing
 * the name of the rule, and when the grammar resolution algorithm (not yet implemented) is run,
 * will have a single child pointing to the expression associated with the referenced rule.
 *
 * LitExpr - Literal match against a sequence of characters. Always has a value, representing
 * the string to be matched, and has no child nodes.
 *
 * Class - Character class expression. Matches against any characters given in the children,
 * all of which are range expressions.
 *
 * RangeExpr - Matches again a character that is between a specified minimum and maximum. Evaluation
 * succeeds if the next input character c is such that start <= c < end.
 *
 * StringExpr - Instructs the parser to construct a single node in the parse tree representing
 * everything within the child expression. Doesn't have any effect on what is or isn't accepted
 * by the parser.
 *
 * LabelExpr - Instructs the parser to construct a label node in the parse tree, indicating
 * semantically important information. Many nodes in the parse tree are there simply because
 * they formed part of the call tree used during parsing, but do not necessarily have any
 * use for subsequent analysis of the tree. Label expressions are intended to, in the future,
 * allow us to have simpler trees which only contain the information necessary for analysing
 * a syntax tree. Like String expressions, Label expressions do not have any effect on what is or
 * isn't accepted by the parser.
 */

typedef enum {
    ChoiceExpr,
    SequenceExpr,
    AndExpr,
    NotExpr,
    OptExpr,
    StarExpr,
    PlusExpr,
    IdentExpr,
    LitExpr,
    ClassExpr,
    DotExpr,
    RangeExpr,
    StringExpr,
    LabelExpr,
} ExprKind;

typedef struct Expression Expression;

const char *ExprKindAsString(ExprKind kind);

Expression *ExpressionNewChoice(int count, Expression **children);
Expression *ExpressionNewSequence(int count, Expression **children);
Expression *ExpressionNewAnd(Expression *child);
Expression *ExpressionNewNot(Expression *child);
Expression *ExpressionNewOpt(Expression *child);
Expression *ExpressionNewStar(Expression *child);
Expression *ExpressionNewPlus(Expression *child);
Expression *ExpressionNewIdent(const char *ident);
Expression *ExpressionNewLit(const char *value);
Expression *ExpressionNewClass(int count, Expression **children);
Expression *ExpressionNewDot(void);
Expression *ExpressionNewRange(int lo, int hi);
Expression *ExpressionNewString(Expression *child);
Expression *ExpressionNewLabel(const char *label, Expression *child);
void ExpressionFree(Expression *expr);
void ExpressionPrint(Expression *expr, int highestPrecedence, const char *indent);
void ExpressionPrintTree(Expression *expr, const char *indent, int startCol);

ExprKind ExpressionKind(Expression *expr);
int ExpressionCount(Expression *expr);
Expression *ExpressionChildAt(Expression *expr, int index);

// Choice

int ExprChoiceCount(Expression *expr);
Expression *ExprChoiceChildAt(Expression *expr, int index);

// Sequence

int ExprSequenceCount(Expression *expr);
Expression *ExprSequenceChildAt(Expression *expr, int index);

// And, Not, Opt, Star, Plus

Expression *ExprAndChild(Expression *expr);
Expression *ExprNotChild(Expression *expr);
Expression *ExprOptChild(Expression *expr);
Expression *ExprStarChild(Expression *expr);
Expression *ExprPlusChild(Expression *expr);

// Ident, Lit

const char *ExprIdentValue(Expression *expr);
Expression *ExprIdentTarget(Expression *expr);
void ExprIdentSetTarget(Expression *expr, Expression *target);
const char *ExprLitValue(Expression *expr);

// Class

int ExprClassCount(Expression *expr);
Expression *ExprClassChildAt(Expression *expr, int index);

// Range

int ExprRangeStart(Expression *expr);
int ExprRangeEnd(Expression *expr);

// String

Expression *ExprStringChild(Expression *expr);

// Label

const char *ExprLabelIdent(Expression *expr);
Expression *ExprLabelChild(Expression *expr);
