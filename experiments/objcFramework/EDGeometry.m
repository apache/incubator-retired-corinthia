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

#import "EDGeometry.h"

static CGRect CGRectFromNumberDict(NSDictionary *dict)
{
    if ((dict == nil) || ![dict isKindOfClass: [NSDictionary class]])
        return CGRectZero;

    NSNumber *x = [dict objectForKey: @"x"];
    NSNumber *y = [dict objectForKey: @"y"];
    NSNumber *width = [dict objectForKey: @"width"];
    NSNumber *height = [dict objectForKey: @"height"];

    if ((x == nil) || ![x isKindOfClass: [NSNumber class]] ||
        (y == nil) || ![y isKindOfClass: [NSNumber class]] ||
        (width == nil) || ![width isKindOfClass: [NSNumber class]] ||
        (height == nil) || ![height isKindOfClass: [NSNumber class]])
        return CGRectZero;

    return CGRectMake(x.doubleValue,y.doubleValue,width.doubleValue,height.doubleValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDItemGeometry                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDItemGeometry

- (EDItemGeometry *)initWithContentRect:(CGRect)contentRect
                             fullRect:(CGRect)fullRect
                           parentRect:(CGRect)parentRect
                           hasCaption:(BOOL)hasCaption
                         columnWidths:(NSArray *)columnWidths
{
    if (!(self = [super init]))
        return nil;
    _contentRect = contentRect;
    _fullRect = fullRect;
    _parentRect = parentRect;
    _hasCaption = hasCaption;
    _columnWidths = [columnWidths copy];
    if (_columnWidths != nil)
        _columnOffsets = [self.class computeOffsets: _columnWidths];
    return self;
}

static BOOL isArrayOfType(NSArray *array, Class cls)
{
    if ((array == nil) || ![array isKindOfClass: [NSArray class]])
        return NO;
    for (id obj in array) {
        if (![obj isKindOfClass: cls])
            return NO;
    }
    return YES;
}

+ (EDItemGeometry *)fromDict:(NSDictionary *)dict
{
    CGRect contentRect = CGRectFromNumberDict([dict objectForKey: @"contentRect"]);
    CGRect fullRect = CGRectFromNumberDict([dict objectForKey: @"fullRect"]);
    CGRect parentRect = CGRectFromNumberDict([dict objectForKey: @"parentRect"]);
    BOOL hasCaption = ([dict objectForKey: @"hasCaption"] != nil);
    NSArray *columnWidths = [dict objectForKey: @"columnWidths"];
    if (CGRectIsEmpty(contentRect))
        return nil;
    if (CGRectIsEmpty(fullRect))
        return nil;
    if (CGRectIsEmpty(parentRect))
        return nil;
    if ((columnWidths != nil) && !isArrayOfType(columnWidths,[NSNumber class]))
        return nil;
    return [[EDItemGeometry alloc] initWithContentRect: contentRect
                                            fullRect: fullRect
                                          parentRect: parentRect
                                          hasCaption: hasCaption
                                        columnWidths: columnWidths];
}

+ (NSArray *)computeOffsets:(NSArray *)widths
{
    if (widths == nil)
        return nil;
    NSMutableArray *offsets = [NSMutableArray arrayWithCapacity: 0];
    CGFloat total = 0;
    for (NSNumber *pct in widths) {
        [offsets addObject: [NSNumber numberWithDouble: total]];
        total += pct.doubleValue;
    }
    [offsets addObject: [NSNumber numberWithDouble: total]];
    return offsets;
}

@end
