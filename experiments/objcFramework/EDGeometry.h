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
#import <CoreGraphics/CoreGraphics.h>

typedef enum {
    EDSizeTypeNone = 0,
    EDSizeTypeTopLeft = 0x01,
    EDSizeTypeTopRight = 0x02,
    EDSizeTypeBottomLeft = 0x04,
    EDSizeTypeBottomRight = 0x08,
    EDSizeTypeInnerTop = 0x10,
    EDSizeTypeInnerBottom = 0x20,
} EDSizeType;

typedef enum {
    EDResizeEdgeTop = EDSizeTypeTopLeft | EDSizeTypeTopRight,
    EDResizeEdgeBottom = EDSizeTypeBottomLeft | EDSizeTypeBottomRight,
    EDResizeEdgeLeft = EDSizeTypeTopLeft | EDSizeTypeBottomLeft,
    EDResizeEdgeRight = EDSizeTypeTopRight | EDSizeTypeBottomRight,
    EDResizeEdgeInner = EDSizeTypeInnerTop | EDSizeTypeInnerBottom,
} EDResizeEdge;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDItemGeometry                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface EDItemGeometry : NSObject

@property (assign, readonly) CGRect contentRect; // Doc's rect for figure or table (view is slightly bigger, for resize handles)
@property (assign, readonly) CGRect fullRect; // Includes the caption
@property (assign, readonly) CGRect parentRect; // Rect of the figure or table's containing block (usually the body)
@property (assign, readonly) BOOL hasCaption;

// columnWidths contains NSNumber (double) objects.
// No specific value range; calculated relative to total.
// This property should be nil for figures, in which case no inner handles will be shown
@property (strong, readonly) NSArray *columnWidths;
@property (strong, readonly) NSArray *columnOffsets;

+ (EDItemGeometry *)fromDict:(NSDictionary *)dict;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        EDResizeDelegate                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@protocol EDResizeDelegate <NSObject>

- (void)resizedWidthPct:(CGFloat)widthPct;
- (void)resizedColumns:(NSArray *)widthPcts;

@end
