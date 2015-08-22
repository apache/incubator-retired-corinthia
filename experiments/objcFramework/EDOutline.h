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

#define L10NOutlineTitle                             NSLocalizedString(@"OutlineTitle",nil)
#define L10NOutlineGrpSections                       NSLocalizedString(@"OutlineGrpSections",nil)
#define L10NOutlineGrpFigures                        NSLocalizedString(@"OutlineGrpFigures",nil)
#define L10NOutlineGrpTables                         NSLocalizedString(@"OutlineGrpTables",nil)
#define L10NOutlineBtnAddNumber                      NSLocalizedString(@"OutlineBtnAddNumber",nil)
#define L10NOutlineBtnRemoveNumber                   NSLocalizedString(@"OutlineBtnRemoveNumber",nil)
#define L10NOutlineFigureCaptionPrefix               NSLocalizedString(@"OutlineFigureCaptionPrefix",nil)
#define L10NOutlineTableCaptionPrefix                NSLocalizedString(@"OutlineTableCaptionPrefix",nil)
#define L10NOutlineDelSectionTitle                   NSLocalizedString(@"OutlineDelSectionTitle",nil)
#define L10NOutlineDelSectionHaveSub                 NSLocalizedString(@"OutlineDelSectionHaveSub",nil)
#define L10NOutlineDelSectionNoSub                   NSLocalizedString(@"OutlineDelSectionNoSub",nil)
#define L10NOutlineNone                              NSLocalizedString(@"OutlineNone",nil)

@class EDOutlineItem;
@class EDOutlineCategory;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDOutlineDelegate                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@protocol EDOutlineDelegate

- (void)outlineMoveItem:(EDOutlineItem *)item parent:(EDOutlineItem *)parent next:(EDOutlineItem *)next;
- (void)outlineDeleteItem:(NSString *)itemId;
- (void)outlineChanged;
- (void)outlineItemSelected:(NSString *)itemId;
- (void)outlineItemExpanded:(EDOutlineItem *)item category:(EDOutlineCategory *)category;
- (void)outlineItemCollapsed:(EDOutlineItem *)item category:(EDOutlineCategory *)category;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          EDOutlineItem                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    OutlineItemTypeSection,
    OutlineItemTypeFigure,
    OutlineItemTypeTable
} OutlineItemType;

@interface EDOutlineItem : NSObject

@property (copy, readonly) NSString *itemId;
@property (assign, readonly) OutlineItemType type;
@property (assign, nonatomic) BOOL expanded;
@property (weak, nonatomic) EDOutlineItem *parent;
@property (copy, nonatomic) NSString *title;
@property (copy, nonatomic) NSString *number;
@property (strong, nonatomic) NSArray *children;

@property (strong, readonly) NSString *description;
@property (strong, readonly) NSString *detail;

- (EDOutlineItem *)initWithItemId:(NSString *)itemId type:(OutlineItemType)type;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDOutlineCategory                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface EDOutlineCategory : NSObject

@property (copy, readonly) NSString *title;
@property (assign, readonly) int sectionIndex;
@property (strong, nonatomic) NSArray *rootItems;
@property (strong, readonly) NSArray *expandedItems;

- (EDOutlineCategory *)initWithTitle:(NSString *)title sectionIndex:(int)sectionIndex;
- (void)updateExpandedItems;
- (NSArray *)indexPathsForExpandedDescendants:(EDOutlineItem *)item;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            EDOutline                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface EDOutline : NSObject <NSCopying>

@property (strong, readonly) EDOutlineCategory *sections;
@property (strong, readonly) EDOutlineCategory *figures;
@property (strong, readonly) EDOutlineCategory *tables;
@property (strong, readonly) NSArray *categories;
@property (weak) id<EDOutlineDelegate> delegate;
@property (strong, readonly) NSDictionary *itemsById;
@property (copy, nonatomic) NSDictionary *json;

- (EDOutline *)init;
- (void)addItem:(NSString *)itemId type:(NSString *)type title:(NSString *)title;
- (void)updateItem:(NSString *)itemId title:(NSString *)title;
- (void)removeItem:(NSString *)itemId;
- (EDOutlineItem *)lookupItem:(NSString *)itemId;
- (void)expandItem:(EDOutlineItem *)item category:(EDOutlineCategory *)category;
- (void)collapseItem:(EDOutlineItem *)item category:(EDOutlineCategory *)category;

@end
