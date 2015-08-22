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

#import "EDOutline.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          EDOutlineItem                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDOutlineItem

- (EDOutlineItem *)initWithItemId:(NSString *)itemId type:(OutlineItemType)type
{
    if (!(self = [super init]))
        return nil;

    _itemId = [itemId copy];
    _type = type;

    return self;
}

- (NSString *)description
{
    switch (_type) {
        case OutlineItemTypeSection: {
            if (_number.length > 0)
                return [NSString stringWithFormat: @"%@ %@", _number, _title];
            else
                return _title;
        }
        case OutlineItemTypeFigure:
            return [NSString stringWithFormat: L10NOutlineFigureCaptionPrefix, _number];
        case OutlineItemTypeTable:
            return [NSString stringWithFormat: L10NOutlineTableCaptionPrefix, _number];
    }
    return nil;
}

- (NSString *)detail
{
    switch (_type) {
        case OutlineItemTypeSection:
            return nil;
        case OutlineItemTypeFigure:
            return _title;
        case OutlineItemTypeTable:
            return _title;
    }
    return nil;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDOutlineCategory                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDOutlineCategory
{
    NSMutableArray *_expandedItems;
}

- (EDOutlineCategory *)initWithTitle:(NSString *)title sectionIndex:(int)sectionIndex
{
    if (!(self = [super init]))
        return nil;
    _title = [title copy];
    _sectionIndex = sectionIndex;
    _rootItems = [NSMutableArray arrayWithCapacity: 0];
    _expandedItems = [NSMutableArray arrayWithCapacity: 0];
    return self;
}

- (void)buildExpandedItems:(EDOutlineItem *)item
{
    [_expandedItems addObject: item];
    if (item.expanded) {
        for (EDOutlineItem *child in item.children)
            [self buildExpandedItems: child];
    }
}

- (void)updateExpandedItems
{
    _expandedItems = [NSMutableArray arrayWithCapacity: 0];
    for (EDOutlineItem *item in _rootItems)
        [self buildExpandedItems: item];
}

- (void)setRootItems:(NSArray *)newRootItems
{
    _rootItems = newRootItems;
    [self updateExpandedItems];
}

- (NSUInteger)countExpandedDescendants:(EDOutlineItem *)parent
{
    NSUInteger result = 0;
    for (EDOutlineItem *child in parent.children) {
        result++;
        if (child.expanded)
            result += [self countExpandedDescendants: child];
    }
    return result;
}

- (NSArray *)indexPathsForExpandedDescendants:(EDOutlineItem *)item
{
    NSUInteger extra = [self countExpandedDescendants: item];
    NSUInteger itemIndex = [_expandedItems indexOfObject: item];
    assert(itemIndex < _expandedItems.count);
    NSMutableArray *indexPaths = [NSMutableArray arrayWithCapacity: extra];
    for (NSUInteger i = 0; i < extra; i++) {
        NSUInteger indexes[2];
        indexes[0] = _sectionIndex;
        indexes[1] = itemIndex + i + 1;
        [indexPaths addObject: [NSIndexPath indexPathWithIndexes: indexes length: 2]];
    }
    return indexPaths;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            EDOutline                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDOutline
{
    NSMutableDictionary *_itemsById;
}

- (EDOutline *)init
{
    if (!(self = [super init]))
        return nil;
    _sections = [[EDOutlineCategory alloc] initWithTitle: L10NOutlineGrpSections sectionIndex: 0];
    _figures = [[EDOutlineCategory alloc] initWithTitle: L10NOutlineGrpFigures sectionIndex: 1];
    _tables = [[EDOutlineCategory alloc] initWithTitle: L10NOutlineGrpTables sectionIndex: 2];
    _categories = [NSArray arrayWithObjects: _sections, _figures, _tables, nil];
    _itemsById = [NSMutableDictionary dictionaryWithCapacity: 0];
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    EDOutline *copy = [[EDOutline allocWithZone: zone] init];
    for (EDOutlineItem *item in _itemsById.allValues) {
        EDOutlineItem *itemCopy = [[EDOutlineItem alloc] initWithItemId: item.itemId type: item.type];
        itemCopy.title = item.title;
        [copy->_itemsById setObject: itemCopy forKey: itemCopy.itemId];
    }
    copy.json = _json;
    return copy;
}

- (NSArray *)addItems:(NSArray *)items parent:(EDOutlineItem *)parent type:(OutlineItemType)type
{
    NSMutableArray *result = [NSMutableArray arrayWithCapacity: 0];

    for (NSDictionary *dict in items) {
        NSString *itemId = [dict objectForKey: @"id"];
        NSString *number = [dict objectForKey: @"number"];
        NSArray *children = [dict objectForKey: @"children"];

        EDOutlineItem *item = [_itemsById objectForKey: itemId];
        assert(item != nil);
        item.number = number;
        item.parent = parent;
        item.children = [self addItems: children parent: item type: type];
        [result addObject: item];
    }

    return result;
}

- (void)setJson:(NSDictionary *)newJson
{
    _json = [newJson copy];

    NSArray *jsonSections = [_json objectForKey: @"sections"];
    NSArray *jsonFigures = [_json objectForKey: @"figures"];
    NSArray *jsonTables = [_json objectForKey: @"tables"];

    // Add/update items from jsonOutline
    _sections.rootItems = [self addItems: jsonSections parent: nil type: OutlineItemTypeSection];
    _figures.rootItems = [self addItems: jsonFigures parent: nil type: OutlineItemTypeFigure];
    _tables.rootItems = [self addItems: jsonTables parent: nil type: OutlineItemTypeTable];
}

- (void)addItem:(NSString *)itemId type:(NSString *)type title:(NSString *)title
{
    assert([_itemsById objectForKey: itemId] == nil);
    OutlineItemType itemType;
    if ([type isEqualToString: @"figure"])
        itemType = OutlineItemTypeFigure;
    else if ([type isEqualToString: @"table"])
        itemType = OutlineItemTypeTable;
    else
        itemType = OutlineItemTypeSection;
    EDOutlineItem *item = [[EDOutlineItem alloc] initWithItemId: itemId type: itemType];
    item.title = title;
    [_itemsById setObject: item forKey: itemId];
}

- (void)updateItem:(NSString *)itemId title:(NSString *)title
{
    assert([_itemsById objectForKey: itemId] != nil);
    EDOutlineItem *item = [_itemsById objectForKey: itemId];
    item.title = title;
}

- (void)removeItem:(NSString *)itemId
{
    assert([_itemsById objectForKey: itemId] != nil);
    [_itemsById removeObjectForKey: itemId];
}

- (EDOutlineItem *)lookupItem:(NSString *)itemId
{
    return [_itemsById objectForKey: itemId];
}

- (void)expandItem:(EDOutlineItem *)item category:(EDOutlineCategory *)category
{
    if (!item.expanded) {
        item.expanded = YES;
        [category updateExpandedItems];
        [_delegate outlineItemExpanded: item category: category];
    }
}

- (void)collapseItem:(EDOutlineItem *)item category:(EDOutlineCategory *)category
{
    if (item.expanded) {
        item.expanded = NO;
        [category updateExpandedItems];
        [_delegate outlineItemCollapsed: item category: category];
    }
}

@end
