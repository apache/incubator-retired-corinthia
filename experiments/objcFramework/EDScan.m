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

#import "EDScan.h"
#import "EDScanResults.h"
#import "EDEditor.h"
#import "EDJSInterface.h"
#import "EDOutline.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDScanParagraph                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDScanParagraph

- (EDScanParagraph *)initWithText:(NSString *)text sectionId:(NSString *)sectionId
{
    if (!(self = [super init]))
        return nil;
    _text = [text copy];
    _sectionId = [sectionId copy];
    return self;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDScanOperation                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDScanOperation
{
    BOOL _cancelled;
}

- (EDScanOperation *)initWithEditor:(EDEditor *)editor
{
    if (!(self = [super init]))
        return nil;
    _editor = editor;
    _results = [[EDScanResults alloc] init];
    return self;
}

- (void)start
{
    [super start];

    [_editor.js.scan reset];
    [self performSelector: @selector(next) withObject: nil afterDelay: 0.1];
}

- (void)cancel
{
    _cancelled = YES;
    [super cancel];
}

- (void)next
{
    if (self.status != FCOperationStatusActive)
        return;

    EDScanParagraph *paragraph = [_editor.js.scan next];
    if (paragraph == nil) {
        [self succeed: nil];
        return;
    }
    else {
        if (paragraph.sectionId != nil) {
            _lastSectionId = paragraph.sectionId;
            _currentSection = nil;
        }

        [self processParagraph: paragraph];
        [self performSelector: @selector(next) withObject: nil afterDelay: 0];
    }
}

- (void)includeCurrentSection
{
    if (_currentSection == nil) {
        EDOutlineItem *item = nil;
        if (_lastSectionId != nil)
            item = [_editor.outline lookupItem: _lastSectionId];
        if (item != nil)
            _currentSection = [[EDScanResultsSection alloc] initWithTitle: item.title];
        else
            _currentSection = [[EDScanResultsSection alloc] initWithTitle: nil];
        [_results.sections addObject: _currentSection];
    }
}

- (void)processParagraph:(EDScanParagraph *)paragraph
{
    // Default implementation does nothing
}

- (void)foundMatches:(NSArray *)newMatches
{
    for (EDScanMatch *match in newMatches)
        [_editor.js.scan showMatch: match.matchId];

    // We must add these *after* calling the JS showMatch method, as modifying the results will cause
    // ScanMode to go to the first one if there were no previous matches, and this will modify the document
    // structure, by adding a selection highlight around the match.
    for (EDScanMatch *match in newMatches) {
        [self.currentSection.matches addObject: match];
        [self.results addMatch: match];
    }
}

@end
