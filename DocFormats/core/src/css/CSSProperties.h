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

#ifndef DocFormats_CSSProperties_h
#define DocFormats_CSSProperties_h

/** @file

 A CSSProperties object represents a set of name/value pairs, corresponding to the properties
 defined in a CSS rule.

 When dealing with CSS rules, we make a distinction between **collapsed** and **expanded**
 properties. Collapsed properties use shorthand notation to define multiple expanded properties in
 a single go; for example, the `border-width` shorthand property actually defines
 `border-left-width`, `border-right-width`, `border-top-width`, and `border-bottom-width`
 properties. In the expanded representation, each of the latter properties four is represented
 individually. The functions CSSExpandProperties and CSSCollapseProperties convert between the
 collapsed and expanded representations.

 In addition to CSS shorthand rules, we also expand certain properties like `text-decoration`. In
 CSS, the `text-decoration` property consists of zero or more decoration values, namely `underline`,
 `overline`, and `line-through`. In order to facilitate setting or clearing these values
 individually, CSSProperties uses an expanded representation in which `text-decoration` is split
 into three distinct properties - `text-decoration-underline`, `text-decoration-overline`, and
 `text-decoration-line-through`. When the properties are collapsed using CSSCollapseProperties, the
 values of these properties (where set) are all merged into a single `text-decoration` property.

 @see CSSSheet.h
 @see CSSStyle.h

 */

#include "DFCallback.h"
#include "DFHashTable.h"

typedef struct CSSProperties CSSProperties;

struct CSSProperties {
    int retainCount;
    DFCallback *changeCallbacks;
    DFHashTable *hashTable;
    int dirty;
};

/**
 Returns a textual representation of the properties, in collapsed form. The resulting string can be
 used as the value of a HTML `style` attribute, to specify direct formatting for an element.

 The string returned by this function is newly-allocated, and must be freed by the caller.
 */
char *CSSPropertiesCopyDescription(CSSProperties *properties);

/**
 Retrieve a NULL-terminated array of all the property names set on this object. This array can still
 be used safely after changes to the object, as it contains its own copy of the names.

 The returned array is newly-allocated, and must be freed by the caller. The strings are allocated
 in the same block of memory as the array itself, so a single call to `free` is sufficient.

 @see DFHashTableCopyKeys()
 */
const char **CSSPropertiesCopyNames(CSSProperties *properties);

int CSSPropertiesIsEmpty(CSSProperties *properties);

/**
 Retrieve the value of the property with the specified name. If the property is not set, this
 function returns NULL.
 */
const char *CSSGet(CSSProperties *properties, const char *name);

/**
 Set or clear a property. If the value is non-NULL, then the property is set, replacing any previous
 value that might have been present. If the value is NULL, then the property is removed.
 */
void CSSPut(CSSProperties *properties, const char *name, const char *value);

/**
 Replace all properties in this object with those from the `raw` hash table, which contains
 collapsed properties. This function first expands the properties, as described above, and then
 sets the individual expanded properties on this object.
 */
void CSSPropertiesUpdateFromRaw(CSSProperties *properties, DFHashTable *raw);

int CSSGetBold(CSSProperties *properties);
int CSSGetItalic(CSSProperties *properties);
int CSSGetUnderline(CSSProperties *properties);
int CSSGetLinethrough(CSSProperties *properties);
int CSSGetOverline(CSSProperties *properties);
int CSSGetDefault(CSSProperties *properties);

void CSSSetBold(CSSProperties *properties, int value);
void CSSSetItalic(CSSProperties *properties, int value);
void CSSSetUnderline(CSSProperties *properties, int value);
void CSSSetLinethrough(CSSProperties *properties, int value);
void CSSSetOverline(CSSProperties *properties, int value);
void CSSSetDefault(CSSProperties *properties, int value);

void CSSPropertiesPrint(CSSProperties *properties, const char *indent);

CSSProperties *CSSPropertiesNewWithExtra(CSSProperties *orig, const char *string);
CSSProperties *CSSPropertiesNewWithRaw(DFHashTable *raw);
CSSProperties *CSSPropertiesNewWithString(const char *string);
CSSProperties *CSSPropertiesNew(void);

CSSProperties *CSSPropertiesRetain(CSSProperties *properties);
void CSSPropertiesRelease(CSSProperties *properties);

#endif
