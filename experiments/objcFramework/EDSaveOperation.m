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

#import "EDSaveOperation.h"
#import "EDEditor.h"
#import "EDJSInterface.h"
#import "EDTiming.h"
#import "EDFileFormat.h"
#import <FileClient/FCError.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDSaveOperation                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDSaveOperation
{
    EDEditor *_editor;
    NSString *_path;
    NSMutableArray *_completions;
}

- (EDSaveOperation *)initWithEditor:(EDEditor *)editor path:(NSString *)path
{
    if (!(self = [super init]))
        return nil;
    _editor = editor;
    _path = [path copy];
    _completions = [NSMutableArray arrayWithCapacity: 0];
    return self;
}

- (NSUInteger)completionCount
{
    return _completions.count;
}

- (void)addCompletion:(EDSaveCompletion)completion;
{
    [_completions addObject: [completion copy]];
}

- (void)start
{
    [super start];

    assert(self == _editor.activeSave);
    _editor.delegate.editorIsSaving = YES;
    // Start the actual save in a separate iteration of the event loop, so that the "Saving" label
    // and animated activity indicator become visible
    [self performSelector: @selector(stage1) withObject: nil afterDelay: 0];
}

- (void)stage1
{
    if (!_editor.js.documentModified) {
        [self succeed: nil];
        return;
    }

    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *tempPath = [NSString stringWithFormat: @"%@.saving", _path];
    [_editor.saveTiming start];

    if (![_editor.js.main prepareForSave]) {
        [self fail: [NSError error: @"JS prepareForSave failed"]];
        return;
    }
    [_editor.saveTiming addEntry: @"JS prepare for save"];
    NSString *html = [_editor.js.main getHTML];

    // Save the document to a temporary file (so if it fails, we still have the original)
    [_editor.fileFormat save: tempPath html: html completion:^(NSError *error) {
        if (error != nil) {
            [self fail: error];
            return;
        }

        // Delete temp dir
        error = nil;
        if ([fm fileExistsAtPath: _editor.tempDir] && ![fm removeItemAtPath: _editor.tempDir error: &error]) {
            [self fail: error];
            return;
        }

        // Check if the file has actually changed - if not, avoid trigering an upload and updating the
        // modification time
        if ([fm contentsEqualAtPath: tempPath andPath: _path]) {
            [fm removeItemAtPath: tempPath error: nil];
            [self succeed: nil];
            return;
        }

        // Remove old version of file
        if ([fm fileExistsAtPath: _path] && ![fm removeItemAtPath: _path error: &error]) {
            [self fail: error];
            return;
        }

        // Rename temporary file
        if (![fm moveItemAtPath: tempPath toPath: _path error: &error]) {
            [self fail: error];
            return;
        }

        // Save succsesful. Tell the sync manager to start uploading the file (if it's on a remote
        // server), and to do so even if the app is in the background
        _editor.js.documentModified = NO;
        [_editor.delegate editorDidSaveFile];
        [_editor.saveTiming addEntry: @"Completed save"];
        [self succeed: nil];
    }];
}

- (void)cleanup
{
    for (EDSaveCompletion completion in _completions) {
        if (self.status == FCOperationStatusCancelled)
            completion(YES,nil);
        else if (self.status == FCOperationStatusFailed)
            completion(NO,self.privateError);
        else
            completion(NO,nil);
    }
    _completions = nil; // break retain cycles

    _editor.delegate.editorIsSaving = NO;

    assert(self == _editor.activeSave);

    if (_editor.pendingSave != nil) {
        _editor.activeSave = _editor.pendingSave;
        _editor.pendingSave = nil;
        [_editor.activeSave start];
    }
    else {
        _editor.activeSave = nil;
    }

    assert(self != _editor.activeSave);
    [_editor debugSaveStatus];
}

@end
