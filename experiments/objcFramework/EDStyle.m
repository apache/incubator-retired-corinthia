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

#import "EDStyle.h"
#import "EDUtil.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             EDStyle                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDStyle

- (void)dealloc
{
    CSSStyleRelease(_cssStyle);
}

- (BOOL)isEqual:(id)object
{
    if ((object == nil) || ![object isKindOfClass: [EDStyle class]])
        return NO;
    EDStyle *otherStyle = (EDStyle *)object;
    return (_cssStyle == otherStyle.cssStyle);
}

+ (EDStyle *)styleWithCSSStyle:(CSSStyle *)cssStyle
{
    if (cssStyle == NULL)
        return nil;
    EDStyle *style = [[EDStyle alloc] init];
    style->_cssStyle = CSSStyleRetain(cssStyle);
    return style;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            EDCascade                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDCascade

- (EDCascade *)init
{
    if (!(self = [super init]))
        return nil;
    return self;
}

- (void)dealloc
{
    CSSPropertiesRelease(_defaultProperties);
    CSSPropertiesRelease(_docProperties);
    CSSPropertiesRelease(_styleProperties);
    CSSPropertiesRelease(_directProperties);
}

- (void)setPropertiesPtr:(CSSProperties **)ptr value:(CSSProperties *)value
{
    if (*ptr != value) {
        CSSPropertiesRelease(*ptr);
        *ptr = CSSPropertiesRetain(value);
    }
}

- (void)setDefaultProperties:(CSSProperties *)defaultProperties
{
    [self setPropertiesPtr: &_defaultProperties value: defaultProperties];
}

- (void)setDocProperties:(CSSProperties *)docProperties
{
    [self setPropertiesPtr: &_docProperties value: docProperties];
}

- (void)setStyleProperties:(CSSProperties *)styleProperties
{
    [self setPropertiesPtr: &_styleProperties value: styleProperties];
}

- (void)setDirectProperties:(CSSProperties *)directProperties
{
    [self setPropertiesPtr: &_directProperties value: directProperties];
}

- (NSString *)get:(NSString *)name includeDirect:(BOOL)includeDirect
{
    CSSProperties *cascade[4] = { _directProperties, _styleProperties, _docProperties, _defaultProperties };
    size_t count = 4;
    size_t first = includeDirect ? 0 : 1;
    for (size_t i = first; i < count; i++) {
        if (cascade[i] != NULL) {
            const char *value = CSSGet(cascade[i],name.UTF8String);
            if (value != NULL)
                return [NSString stringWithUTF8String: value];
        }
    }
    return nil;
}

- (NSString *)get:(NSString *)name
{
    return [self get: name includeDirect: YES];
}

- (void)set:(NSString *)name value:(NSString *)value
{
    if (_directProperties == NULL)
        return;

    // If the value is unchanged, don't call CSSPut; this avoids triggering an unnecessary change notification
    NSString *oldValue = [self get: name includeDirect: YES];
    if (EDStringEquals(oldValue,value))
        return;

    // If the value being set is the same as the value inherited from the style, document, or detauls, remove
    // the direct formatting property. Otherwise, set it to override the inherited value.
    NSString *parentValue = [self get: name includeDirect: NO];
    if (EDStringEquals(parentValue,value))
        CSSPut(_directProperties,name.UTF8String,NULL);
    else
        CSSPut(_directProperties,name.UTF8String,value.UTF8String);
}

@end
