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
#include "Builtin.h"
#include <stdarg.h>
#include <stdlib.h>

#define ref(name)       ExpressionNewIdent(name)
#define choice(...)     makeExpression(ExpressionNewChoice,__VA_ARGS__,NULL)
#define seq(...)        makeExpression(ExpressionNewSequence,__VA_ARGS__,NULL)
#define and(child)      ExpressionNewAnd(child)
#define not(child)      ExpressionNewNot(child)
#define opt(child)      ExpressionNewOpt(child)
#define star(child)     ExpressionNewStar(child)
#define plus(child)     ExpressionNewPlus(child)
#define lit(value)      ExpressionNewLit(value)
#define cls(...)        makeExpression(ExpressionNewClass,__VA_ARGS__,NULL)
#define dot()           ExpressionNewDot()
#define string(child)   ExpressionNewString(child)
#define range(lo,hi)    ExpressionNewRange(lo,hi)

static Expression *makeExpression(Expression *(*fun)(int,Expression **), ...)
{
    Expression *children[20];
    int count = 0;
    va_list ap;
    va_start(ap,fun);
    Expression *expr;
    while ((count < 20) && ((expr = va_arg(ap,Expression *)) != NULL))
        children[count++] = expr;
    va_end(ap);
    return fun(count,children);
}

Grammar *GrammarNewBuiltin(void)
{
    Grammar *gram = GrammarNew();

    // Grammar    <- Spacing Definition+ EndOfFile
    GrammarDefine(gram,"Grammar",
                    seq(ref("Spacing"),
                        plus(ref("Definition")),
                        ref("EndOfFile")));

    // Definition <- Identifier LEFTARROW Expression
    GrammarDefine(gram,"Definition",
                    seq(ref("Identifier"),
                        ref("LEFTARROW"),
                        ref("Expression")));
    // Expression <- Sequence (SLASH Sequence)*
    GrammarDefine(gram,"Expression",
                    seq(ref("Sequence"),
                        star(seq(ref("SLASH"),ref("Sequence")))));

    // Sequence   <- Prefix*
    GrammarDefine(gram,"Sequence",
                    star(ref("Prefix")));

    // Prefix     <- (AND / NOT)? Suffix
    GrammarDefine(gram,"Prefix",
                    seq(opt(choice(ref("AND"),
                                   ref("NOT"))),
                        ref("Suffix")));

    // Suffix     <- Primary (QUESTION / STAR / PLUS)?
    GrammarDefine(gram,"Suffix",
                    seq(ref("Primary"),
                        opt(choice(ref("QUESTION"),
                                   ref("STAR"),
                                   ref("PLUS")))));

    // Primary    <- Identifier !LEFTARROW
    //             / OPEN Expression CLOSE
    //             / Literal
    //             / Class
    //             / DOT
    GrammarDefine(gram,"Primary",
                    choice(seq(ref("Identifier"),not(ref("LEFTARROW"))),
                           seq(ref("DOLLAR"),ref("OPEN"),ref("Expression"),ref("CLOSE")),
                           seq(ref("OPEN"),ref("Expression"),ref("CLOSE")),
                           ref("Literal"),
                           ref("Class"),
                           ref("DOT")));

    // Identifier <- IdentStart IdentCont* Spacing
    GrammarDefine(gram,"Identifier",
                    seq(string(seq(ref("IdentStart"),
                                   star(ref("IdentCont")))),
                        ref("Spacing")));

    // IdentStart <- [a-zA-Z_]
    GrammarDefine(gram,"IdentStart",
                    cls(range('a','z'),
                        range('A','Z'),
                        range('_','_')));

    // IdentCont  <- IdentStart
    //             / [0-9]
    GrammarDefine(gram,"IdentCont",
                    choice(ref("IdentStart"),
                        cls(range('0','9'))));

    // Literal    <- ['] (!['] Char)* ['] Spacing
    //             / ["] (!["] Char)* ["] Spacing
    GrammarDefine(gram,"Literal",
                    choice(seq(cls(range('\'','\'')),
                               star(seq(not(cls(range('\'','\''))),
                                        ref("Char"))),
                               cls(range('\'','\'')),
                               ref("Spacing")),
                           seq(cls(range('"','"')),
                               star(seq(not(cls(range('"','"'))),
                                        ref("Char"))),
                               cls(range('"','"')),
                               ref("Spacing"))));

    // Class      <- '[' (!']' Range)* ']' Spacing
    GrammarDefine(gram,"Class",
                    seq(lit("["),
                        star(seq(not(lit("]")),
                                 ref("Range"))),
                        lit("]"),
                        ref("Spacing")));
    // Range      <- Char '-' Char
    //             / Char
    GrammarDefine(gram,"Range",
                    choice(seq(ref("Char"),
                               lit("-"),
                               ref("Char")),
                           ref("Char")));

    // Char       <- '\\' [nrt'"\[\]\\]
    //             / '\\' [0-2] [0-7] [0-7]
    //             / '\\' [0-7] [0-7]?
    //             / !'\\' .
    GrammarDefine(gram,"Char",
                    choice(seq(lit("\\"),
                               cls(range('n','n'),
                                   range('r','r'),
                                   range('t','t'),
                                   range('\'','\''),
                                   range('"','"'),
                                   range('[','['),
                                   range(']',']'),
                                   range('\\','\\'))),
                           seq(lit("\\"),
                               cls(range('0','2')),
                               cls(range('0','7')),
                               cls(range('0','7'))),
                           seq(lit("\\"),
                               cls(range('0','7')),
                               opt(cls(range('0','7')))),
                           seq(not(lit("\\")),dot())));

    // LEFTARROW  <- '<-' Spacing
    GrammarDefine(gram,"LEFTARROW",seq(lit("<-"),ref("Spacing")));

    // SLASH      <- '/' Spacing
    GrammarDefine(gram,"SLASH",seq(lit("/"),ref("Spacing")));

    // AND        <- '&' Spacing
    GrammarDefine(gram,"AND",seq(lit("&"),ref("Spacing")));

    // NOT        <- '!' Spacing
    GrammarDefine(gram,"NOT",seq(lit("!"),ref("Spacing")));

    // QUESTION   <- '?' Spacing
    GrammarDefine(gram,"QUESTION",seq(lit("?"),ref("Spacing")));

    // STAR       <- '*' Spacing
    GrammarDefine(gram,"STAR",seq(lit("*"),ref("Spacing")));

    // PLUS       <- '+' Spacing
    GrammarDefine(gram,"PLUS",seq(lit("+"),ref("Spacing")));

    // OPEN       <- '(' Spacing
    GrammarDefine(gram,"OPEN",seq(lit("("),ref("Spacing")));

    // CLOSE      <- ')' Spacing
    GrammarDefine(gram,"CLOSE",seq(lit(")"),ref("Spacing")));

    // DOT        <- '.' Spacing
    GrammarDefine(gram,"DOT",seq(lit("."),ref("Spacing")));

    // DOLLAR     <- '$' Spacing
    GrammarDefine(gram,"DOLLAR",seq(lit("$"),ref("Spacing")));

    // Spacing    <- (Space / Comment)*
    GrammarDefine(gram,"Spacing",string(star(choice(ref("Space"),ref("Comment")))));

    // Comment    <- '#' (!EndOfLine .)* EndOfLine
    GrammarDefine(gram,"Comment",
                    seq(lit("#"),
                        star(seq(not(ref("EndOfLine")),
                                 dot())),
                        ref("EndOfLine")));

    // Space      <- ' '
    //            / '\t'
    //            / EndOfLine
    GrammarDefine(gram,"Space",
                    choice(lit(" "),
                           lit("\t"),
                           ref("EndOfLine")));
    // EndOfLine  <- '\r\n'
    //             / '\n'
    //             / '\r'
    GrammarDefine(gram,"EndOfLine",
                    choice(lit("\r\n"),
                           lit("\n"),
                           lit("\r")));

    // EndOfFile  <- !.
    GrammarDefine(gram,"EndOfFile",not(dot()));

    GrammarResolve(gram);

    return gram;
}
