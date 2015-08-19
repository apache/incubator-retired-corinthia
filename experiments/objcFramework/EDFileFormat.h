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

typedef NSComparisonResult(^Comparator)(id,id);

@class EDEditor;
@protocol EDSystemDelegate;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          EDFileFormat                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface EDFileFormat : NSObject

@property (weak) EDEditor *editor;
@property (copy, readonly) NSString *fileLocalPath;
@property (copy, readonly) NSString *absoluteHTMLPath;
@property (assign, readonly) BOOL supportsBareStyles;
@property (copy, readonly) NSString *imagePath;
@property (readonly) Comparator styleComparator;

- (EDFileFormat *)init;
- (NSString *)prepareForLoad:(NSError **)error;
- (void)finishLoad;
- (void)save:(NSString *)filename html:(NSString *)html completion:(void (^)(NSError *error))completion;
- (void)setupInitialStyles;
- (CSSStyle *)setupTableStyle;
- (void)setupFigureStyle;
- (void)setupOutlineStyle;
- (NSString *)addImage:(NSData *)data extension:(NSString *)extension error:(NSError **)error;
- (NSString *)localizedStyleName:(CSSStyle *)style;

+ (BOOL)create:(NSString *)filename error:(NSError **)error;
+ (EDFileFormat *)formatForExtension:(NSString *)extension;
+ (BOOL)isExtensionSupported:(NSString *)extension;
+ (NSString *)findUniqueFilenameWithPrefix:(NSString *)prefix
                                 extension:(NSString *)extension
                             existingNames:(NSArray *)names;
@end
