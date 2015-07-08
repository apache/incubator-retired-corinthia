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

#include "Grammar.h"

/**
 * The GrammarNewBuiltin() function creates the following Grammar, which comes
 * from Ford's original PEG paper:
 *
 *     Grammar    <- Spacing Definition+ EndOfFile
 *     Definition <- Identifier LEFTARROW Expression
 *     Expression <- Sequence (SLASH Sequence)*
 *     Sequence   <- Prefix*
 *     Prefix     <- (AND / NOT)? Suffix
 *     Suffix     <- Primary (QUESTION / STAR / PLUS)?
 *     Primary    <- Identifier !LEFTARROW
 *                 / OPEN Expression CLOSE
 *                 / Literal
 *                 / Class
 *                 / DOT
 *     Identifier <- IdentStart IdentCont* Spacing
 *     IdentStart <- [a-zA-Z_]
 *     IdentCont  <- IdentStart
 *                 / [0-9]
 *     Literal    <- ['] (!['] Char)* ['] Spacing
 *                 / ["] (!["] Char)* ["] Spacing
 *     Class      <- '[' (!']' Range)* ']' Spacing
 *     Range      <- Char '-' Char
 *                 / Char
 *     Char       <- '\\' [nrt'"\[\]\\]
 *                 / '\\' [0-2] [0-7] [0-7]
 *                 / '\\' [0-7] [0-7]?
 *                 / !'\\' .
 *     LEFTARROW  <- '<-' Spacing
 *     SLASH      <- '/' Spacing
 *     AND        <- '&' Spacing
 *     NOT        <- '!' Spacing
 *     QUESTION   <- '?' Spacing
 *     STAR       <- '*' Spacing
 *     PLUS       <- '+' Spacing
 *     OPEN       <- '(' Spacing
 *     CLOSE      <- ')' Spacing
 *     DOT        <- '.' Spacing
 *     Spacing    <- (Space / Comment)*
 *     Comment    <- '#' (!EndOfLine .)* EndOfLine
 *     Space      <- ' '
 *                 / '\t'
 *                 / EndOfLine
 *     EndOfLine  <- '\r\n'
 *                 / '\n'
 *                 / '\r'
 *     EndOfFile  <- !.
 */
Grammar *GrammarNewBuiltin(void);
