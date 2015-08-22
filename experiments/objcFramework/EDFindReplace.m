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

// This file comes from the portion of the UX Write editor that
// works on both Apple platforms (that is, it can run on either
// OS X or iOS). It's in the repository for illustrative purposes
// only, to assist with the creation of the framework for the
// Corinthia editor UI. The code does not compile independently in
// its present form.

#import "EDFindReplace.h"
#import "EDEditor.h"
#import "EDScanResults.h"
#import "EDJSInterface.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDFindOperation                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDFindOperation

- (EDFindOperation *)initWithEditor:(EDEditor *)editor
                           findTerm:(NSString *)findTerm
                        replaceTerm:(NSString *)replaceTerm
                      caseSensitive:(BOOL)caseSensitive
                              regex:(NSRegularExpression *)regex
{
    if (!(self = [super initWithEditor: editor]))
        return nil;
    _findTerm = [findTerm copy];
    _replaceTerm = [replaceTerm copy];
    _caseSensitive = caseSensitive;
    _regex = regex;
    return self;
}

- (void)processParagraph:(EDScanParagraph *)paragraph
{
    NSString *text = paragraph.text;

    NSMutableArray *newMatches = [NSMutableArray arrayWithCapacity: 0];

    NSStringCompareOptions compareOptions = _caseSensitive ? 0 : NSCaseInsensitiveSearch;
    NSUInteger index = 0;
    while (true) {
        NSRange remainder = NSMakeRange(index,text.length-index);
        NSRange range;
        if (_regex != nil)
            range = [_regex rangeOfFirstMatchInString: text options: 0 range: remainder];
        else
            range = [text rangeOfString: _findTerm options: compareOptions range: remainder];
        if (range.location == NSNotFound)
            break;
        index = range.location + range.length;

        [self includeCurrentSection];

        int matchId = [self.editor.js.scan addMatchStart: (int)range.location end: (int)(range.location + range.length)];
        NSString *matchText = [text substringWithRange: range];
        [newMatches addObject: [[EDScanMatch alloc] initWithMatchId: matchId text: matchText]];
    }

    [self foundMatches: newMatches];
}

@end
