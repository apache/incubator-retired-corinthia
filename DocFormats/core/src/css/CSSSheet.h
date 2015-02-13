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

#ifndef DocFormats_CSSSheet_h
#define DocFormats_CSSSheet_h

/** \file

 A CSSSheet object represents a CSS stylesheet. It contains a series of styles, identified by
 their selector (element and optional class name), each of which is corresponds to a CSSStyle
 object.

 CSSSheet uses reference counting. When you create a new object using CSSSheetNew, it initially
 has a reference count of 1. You can increment and decrement the reference count by calling
 CSSSheetRetain and CSSSheetRelease, respectively. When the reference count drops to 0, the object
 will be freed from memory.

 Each style is identified by its **base selector**, or simply **selector** for short. This
 corresponds to the initial part of the selector used in all rules of a CSS stylesheet that are
 associated with that style. The selector consists of an element name (e.g. `h1`), plus an optional
 class name (e.g. `h1.class`).

 To add a new style or retrieve an existing style from the stylesheet, use @ref
 CSSSheetLookupSelector or @ref CSSSheetLookupElement. Both of these functions take an `add`
 parameter, which indicate whether the style should be added if it does not already exist. If you
 just want to get an existing file, pass in zero for the add parameter. If you want to add the style
 if it does not already exist, pass in a non-zero value. @ref CSSSheetLookupSelector takes a
 selector (element name and optional class name combined into a single string) as a parameter; @ref
 CSSSheetLookupElement takes the element name and class name as separate parameters.

 @see CSSStyle.h
 @see CSSProperties.h

 */

#include "CSSStyle.h"
#include "DFHashTable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSSheet                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CSSSheet CSSSheet;

CSSSheet *CSSSheetNew(void);
CSSSheet *CSSSheetRetain(CSSSheet *sheet);
void CSSSheetRelease(CSSSheet *sheet);

CSSProperties *CSSSheetPageProperties(CSSSheet *sheet);
CSSProperties *CSSSheetBodyProperties(CSSSheet *sheet);
int CSSSheetHeadingNumbering(CSSSheet *sheet);
void CSSSheetSetHeadingNumbering(CSSSheet *sheet, int enabled);
int CSSSheetIsNumberingUsed(CSSSheet *sheet);

/**
 Retrieve a NULL-terminated array containing the base selectors of styles in this stylesheet. A
 base selector consists of an element name and optional class name (e.g. `h1` or `h1.Appendix`),
 and is used to uniquely identify a style. You can pass these selectors to CSSSheetLookupSelector()
 to get the actual style objects.

 The returned array is newly-allocated, and must be freed by the caller. The strings are allocated
 in the same block of memory as the array itself, so a single call to `free` is sufficient.

 @see DFHashTableCopyKeys()
 */
const char **CSSSheetCopySelectors(CSSSheet *sheet);

/**
 Add a new style to the stylesheet, replacing any existing style with the same name, if present.

 Generally, you should not call this function directly, but instead use CSSSheetLookupSelector()
 with an `add` parameter of 1, so that if there's already a style with the same selector, it will
 remain.
 */
void CSSSheetAddStyle(CSSSheet *sheet, CSSStyle *style);

/**
 Remove a style from the stylesheet.

 Note that this causes the CSSSheet object to release its reference to the style, so if you want to
 ensure the CSSStyle object remains in memory after this function is called, you must retain your
 own reference to it. See replaceSelectorsInSheet() for an example.
 */
void CSSSheetRemoveStyle(CSSSheet *sheet, CSSStyle *style);

/**
 Retrieve the CSSStyle object associated with the specified selector, optionally adding it if it
 does not yet exist.

 @param add If non-zero, the style will be added if it does not already exist. If zero, NULL will
 be returned if the style does not exist.
 @param latent If non-zero, and a new style is added, it will be marked as latent. This result in it
 not appearing in the textual representation of a stylesheet. Normally this should be set to zero;
 it is only useful for definining built-in styles in an editing interface which are program defaults
 that are yet to be activated.
 */
CSSStyle *CSSSheetLookupSelector(CSSSheet *sheet, const char *selector, int add, int latent);

/**
 Retrieve the CSSStyle object associated with the specified element name and optional class name,
 optionally adding it if it does not yet exist. The element name *must* be supplied; the class name
 can be NULL. This function behaves in the same way as CSSSheetLookupSelector.

 @param add If non-zero, the style will be added if it does not already exist. If zero, NULL will
 be returned if the style does not exist.
 @param latent If non-zero, and a new style is added, it will be marked as latent. This result in it
 not appearing in the textual representation of a stylesheet. Normally this should be set to zero;
 it is only useful for definining built-in styles in an editing interface which are program defaults
 that are yet to be activated.
 */
CSSStyle *CSSSheetLookupElement(CSSSheet *sheet, const char *elementName, const char *className, int add, int latent);

/**
 Creates a new CSSStyle object whose rules are all "flattened" --- that is, they include all properties
 defined on the style and all of its ancestors.

 Formats like Word and ODF support **style inheritance**, whereby each style can optionally have a
 parent style associated with it. This causes the style to inherit all properties from its parent,
 in the same way that a subclass inherits fields and methods from its superclass in an object-oriented
 programming language. The parent may itself have a parent, and so forth, so there may be any number
 of "ancestors" of the style.

 The process of flattening a style involves starting with a style itself, collecting all properties
 directly defined there, and then repeatedly going through the chain of ancestors adding all properties
 they define as well. The result is that the flattened style contains all inherited properties.

 For example, suppose you had the following three styles defined:

     p.One {
         font-weight: bold;
         font-size: 12pt;
     }

     p.Two {
         -uxwrite-parent: "p.One";
         color: red;
         font-size: 14pt;
     }

     p.Three {
         -uxwrite-parent: "p.Two";
         font-size: 18pt;
         font-style: italic;
     }

 The collapsed style would have the following properties (note that font-size is overridden, so
 we use the definition closest to the original style in preference to the value defined by its
 ancestors):

     p.Three {
         -uxwrite-parent: "p.Two";
         font-weight: bold;        // From p.One
         color: red;               // From p.Two
         font-size: 18pt;          // From p.Three (overriden)
         font-style: italic;       // From p.Three
     }

 The fact that the properties have been flattened like this mean they will display as expected
 in a web browser (and UX Write itself, since it displays documents in the same way as a web
 browser). Without flattening, we would only have the `font-size` and `font-weight` properties
 defined; browsers don't know about the special `-uxwrite-parent` property and cannot infer that
 the style is inherited.

 Unlike all other CSSSheet methods, the CSSStyle object returned by this function is newly
 allocated, and the caller must call CSSStyleRelease on the object when finished with it.
 */
CSSStyle *CSSSheetFlattenedStyle(CSSSheet *sheet, CSSStyle *orig);

/**
 Builds a hash table of rules to be included in the textual representation of the stylesheet. This
 corresponds directly to what is produced by CSSSheetFlattenedStyle(), but is in the form of a hash
 table rather than a string, so that it can be easily manipulated in code.

 Each key in the hash table is a full selector (i.e. base selector + suffix). Each value is another
 hash table, whose keys are property names and whose values are property names.

 The only reason this function is exposed separately from CSSSheetFlattenedStyle() is that UX Write
 needs to pass this information to the javascript portion of the editing code, which uses it for
 things like determining the style to apply to new paragraphs. In general you would normally just
 call CSSSheetFlattenedStyle() instead.

 The returned hash table is newly allocated and must be released by the caller.
 */
DFHashTable *CSSSheetRules(CSSSheet *sheet);

/**
 Converts a stylesheet into its textual representation. All rules are collapsed, all styles are
 flattened, and all CSS declarations which have the same set of proeprties are merged. The result
 can be used as the text content of a HTML `style` element to define the stylesheet for a document,
 or written to an external stylesheet in a `.css` file.

 The string returned by this function is newly-allocated, and must be freed by the caller.
 */
char *CSSSheetCopyCSSText(CSSSheet *sheet);

/**
 Similar to CSSSheetCopyCSSText(), but uses an alternative textual format that lists all rules
 without flattening inherited styles. It is here only for testing purposes; some of the automated
 tests rely on it.

 The string returned by this function is newly-allocated, and must be freed by the caller.
 */
char *CSSSheetCopyText(CSSSheet *sheet);

/**
 Replaces all styles with those defined in the supplied CSS text.

 You can use this function to create a new stylesheet based on the text content of a HTML `style`
 element, or the contents of an external stylesheet in a `.css` file. First call CSSSheetNew, and
 then CSSSheetUpdateFromCSSText with the string representing the CSS text:

     char *cssText = ... // get from HTML <style> element or .css file
     CSSSheet *sheet = CSSSheetNew();
     CSSSheetUpdateFromCSSText(sheet,cssText);

 This function is the inverse of CSSSheetCopyCSSText. If you call CSSSheetCopyCSSText and then
 pass the result to CSSSheetUpdateFromCSSText, you will get the same stylesheet.
 */
void CSSSheetUpdateFromCSSText(CSSSheet *sheet, const char *cssText);

CSSStyle *CSSSheetDefaultStyleForFamily(CSSSheet *sheet, StyleFamily family);
void CSSSheetSetDefaultStyle(CSSSheet *sheet, CSSStyle *style, StyleFamily family);
int CSSStyleIsNumbered(CSSStyle *style);
CSSStyle *CSSSheetGetStyleParent(CSSSheet *sheet, CSSStyle *style);

/**
 Retrieve the parent of a given style, if it has one.

 The CSSStyle object itself records the parent's base selector name in the `-uxwrite-parent`
 property, but in order to actually retrieve that style, it is necessary to have access to the
 CSSSheet object, since this is the only way to get access to the other CSSStyle objects. This
 function checks to see if `-uxwrite-parent` is set, and it so, looks up that selector in the
 stylesheet.

 If the style does not specify a parent, or it does specify a parent but that parent does not exist,
 this function returns NULL.
 */
CSSStyle *CSSSheetParentOfStyle(CSSSheet *sheet, CSSStyle *style);
int CSSIsBuiltinSelector(const char *selector);

#endif
