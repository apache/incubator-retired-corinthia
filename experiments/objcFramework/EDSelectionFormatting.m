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

#import "EDSelectionFormatting.h"
#import "EDUtil.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                      EDSelectionFormatting                                     //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDSelectionFormatting

- (EDSelectionFormatting *)init
{
    return [self initWithProperties: @{ }];
}

- (EDSelectionFormatting *)initWithProperties:(NSDictionary *)properties
{
    if (!(self = [super init]))
        return nil;
    _cssProperties = CSSPropertiesNew();

    DFHashTable *hashTable = HashTableFromNSDictionary(properties);
    CSSPropertiesUpdateFromRaw(_cssProperties,hashTable);
    DFHashTableRelease(hashTable);

    return self;
}

- (void)dealloc
{
    CSSPropertiesRelease(_cssProperties);
}

- (BOOL)shift { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-shift")); }
- (BOOL)inBrackets { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-brackets")); }
- (BOOL)inQuotes { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-quotes")); }
- (BOOL)inImage { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-image")); }
- (BOOL)inFigure { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-figure")); }
- (BOOL)inTable { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-table")); }
- (BOOL)inReference { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-reference")); }
- (BOOL)inLink { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-link")); }
- (BOOL)inTOC { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-toc")); }
- (BOOL)inUL { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-ul")); }
- (BOOL)inOL { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-ol")); }
- (BOOL)inTT { return DFStringEquals("true",CSSGet(_cssProperties,"-uxwrite-in-tt")); }
- (NSString *)inItemTitle { return NSStringFromC(CSSGet(_cssProperties,"-uxwrite-in-item-title")); }

@end
