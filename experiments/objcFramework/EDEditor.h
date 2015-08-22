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

#import <Foundation/Foundation.h>
#import <Editor/DocFormats.h>
#import "EDGeometry.h"
#import "EDSaveOperation.h"

@class EDFileFormat;
@class JSInterface;
@class EDOutline;
@class EDSelectionFormatting;
@class EDTimingInfo;
@class EDSaveOperation;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDSystemDelegate                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@protocol EDSystemDelegate

- (void)runInBackground:(void (^)(void))block completion:(void (^)(void))completion;
- (void)runCommandInBackground:(BOOL (^)(NSError **commandError))command
                    completion:(void (^)(NSError *completionError))completion;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDEditorDelegate                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@protocol EDEditorDelegate

@property (nonatomic) BOOL editorIsSaving;
@property (nonatomic, readonly) NSString *editorPath;

- (void)editorDidUpdateCSS;
- (void)editorDidUpdateOutline;
- (void)editorDidSaveFile;
- (void)editorShowResizeHandles:(EDItemGeometry *)geometry vertical:(BOOL)vertical;
- (void)editorHideResizeHandles;

- (NSString *)saveResource:(NSData *)data prefix:(NSString *)prefix extension:(NSString *)extension
                     error:(NSError **)error;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            EDEditor                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface EDEditor : NSObject <EDResizeDelegate>

@property (weak) NSObject<EDEditorDelegate> *delegate;
@property (weak) NSObject<EDSystemDelegate> *system;
@property (strong, readonly) EDFileFormat *fileFormat;
@property (strong, readonly) NSString *generator;
@property (strong, readonly) NSString *tempDir;
@property (strong, readwrite) JSInterface *js;
@property (assign, readonly) BOOL jsInitOk;
@property (strong, readonly) EDOutline *outline;
@property (strong, readwrite, nonatomic) EDSelectionFormatting *formatting;
@property (copy, nonatomic) NSString *paragraphStyleId;
@property (assign, readonly) CSSSheet *styleSheet;
@property (strong, readonly) EDTimingInfo *loadTiming;
@property (strong, readonly) EDTimingInfo *saveTiming;
@property (strong) EDSaveOperation *activeSave;
@property (strong) EDSaveOperation *pendingSave;
@property (strong, readonly) NSString *origGenerator;
@property (copy, nonatomic) NSString *locale;

- (EDEditor *)initWithFileFormat:(EDFileFormat *)fileFormat generator:(NSString *)generator tempDir:(NSString *)tempDir;
- (void)updateFormatting;
- (void)updateCSS;
- (void)retrieveStyles;
- (void)makeStyleNonLatent:(const char *)ident;
- (void)setOutlineDirty;
- (void)loadAndInitSucceeded;
- (void)dumpHTML;
- (void)saveTo:(NSString *)path completion:(EDSaveCompletion)completion;
- (void)debugSaveStatus;
- (void)undo;
- (void)redo;

@end
