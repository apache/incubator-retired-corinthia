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

#ifndef DocFormats_CSSStyle_h
#define DocFormats_CSSStyle_h

/**

 \file

 # %CSSStyle

 A CSSStyle object represents a set of rules in a CSS stylesheet which all have a common **base
 selector**, consisting of an element name and optional class name (e.g. `h1` or `h1.Appendix`). The
 base selector is used to identify the style in a CSSSheet object, and is passed to
 CSSSheetLookupSelector() to add or retrieve a particular style.

 Each CSSStyle object has one or more **rules** associated with it. A rule is identified by its
 **suffix**, which is the text that comes immediately after the base selector in the textual
 representation of the stylesheet. For example, a rule defined in a CSS stylesheet with a selector
 of `h1.Appendix::before` would have a base selector of `h1.Appendix`, and a suffix of `::before`.

 In the typical case, a style will have only a single rule with an empty suffix, and this rule will
 define all of the properties of that style. For example, a style with a base selector of `p.Warning`
 and properties defining the color and font size would appear in the textual representation of the
 stylesheet like this:

     p.Warning {
         color: red;
         font-size: 18pt;
     }

 In other cases, a style may have multiple rules, each with a different suffix. Each rule defines
 the properties to be applied to a certain aspect of that element, such as particular rows of a
 table. For example, style with a base selector of `table.Statistics` which defines a black border,
 and colors odd rows blue and odd rows red, would have three separate rules:

     table.Statistics {
         border: 1px solid black;
     }

     table.Statistics > * > tr:nth-of-type(odd) > td {
         color: blue;
     }

     table.Statistics > * > tr:nth-of-type(even) > td {
         color: red;
     }

 ## Creating styles

 You should not create CSSStyle objects directly --- instead, call CSSSheetLookupSelector() with
 the `add` parameter set to 1. This ensures that if there is already a style with that selector
 present in the stylesheet, you will not end up with a duplicate. CSSSheetLookupSelector() returns
 the new or existing style, and calls CSSStyleNew() internally if necessary.

 All though CSSStyle objects are reference-counted, you generally do not need to retain or release
 them yourself, as the CSSSheet object itself maintains a reference to all styles within it. When
 a CSSSheet object is freed, it releases all references to the styles it contains, so assuming there
 are no additional references to them, they will be freed at that time as well.

 ## Working with rules

 A rule is represented by a CSSProperties object. The main function for accessing rules is
 CSSStyleRuleForSuffix(), which requires the desired suffix to be specified. There are also several
 short-hand funcitons for obtaining certain commonly-used rules; CSSStyleRule() returns the default
 rule (the one with an empty suffix), while CSSStyleCell() and CSSStyleBefore() returns the rules
 for table cells and content displayed before an element, respectively.

 When you call any of these functions and the rule does not exist, it is created. When the
 stylesheet is converted to its textual representation using CSSSheetCopyCSSText(), empty rules
 (those with no properties) are excluded --- so it is never necessary to remove a rule as such.
 You can, however, clear all the properties on it, which will have the same effect.

 ## Style inheritance

 To support working with documents in formats like docx and ODF, which support style inheritance, we
 use the special `-uxwrite-parent` CSS property to record the base selector of the parent style, if
 any. This should not be set directly, however, as some characters in the style name may not be
 valid in CSS values. Instead, the parent value is stored as a quoted string, according to the
 escaping rules defined in the CSS specification. You should always use CSSStyleCopyParent() and
 CSSStyleSetParent() to access this property; as this handles quoting for you.

 Note that since CSSStyle objects do not contain direct references to each other or the stylesheet
 in which they are contained, if you want to obtain the actual CSSStyle object representing the
 parent style, you need to do so from the CSSSheet object. This can be done using
 CSSSheetParentOfStyle().

 ## Style for next paragraph

 Microsoft Word and OpenOffice both allow a style to specify which style should be used for the next
 paragraph, if the user presses enter within a paragraph of the given style. For example, it is
 typical for heading styles to specify "Normal" as the next style, so when a user types in a heading
 and presses enter, the program adds a normal paragraph, instead of another heading.

 As with parent styles, we use a special property to record this, `-uxwrite-next`. The UX Write
 editing code knows about this property, and uses it to determine what style to associate with
 newly-created paragraphs after enter is pressed. As with the `-uxwrite-parent` property, this
 needs to be quoted, and you should always use CSSStyleCopyNext() and CSSStyleSetNext() to access it.

 @see CSSSheet.h
 @see CSSProperties.h

 */

#include <DocFormats/DFXMLForward.h>
#include "CSSSelector.h"
#include "CSSProperties.h"
#include "DFCallback.h"
#include "DFTypes.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            CSSStyle                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define DFTableSuffixWholeTable ""
#define DFTableSuffixCell " > * > tr > td"
#define DFTableSuffixFirstRow " > * > tr:first-of-type > td"
#define DFTableSuffixLastRow " > * > tr:last-of-type > td"
#define DFTableSuffixFirstCol " > * > tr > td:first-of-type"
#define DFTableSuffixLastCol " > * > tr > td:last-of-type"
#define DFTableSuffixBand1Vert " > * > tr:nth-of-type(odd) > td"
#define DFTableSuffixBand2Vert " > * > tr:nth-of-type(even) > td"
#define DFTableSuffixBand1Horz " > * > tr > td:nth-of-type(odd)"
#define DFTableSuffixBand2Horz " > * > tr > td:nth-of-type(even)"
#define DFTableSuffixNWCell " > * > tr:first-of-type > td:first-of-type"
#define DFTableSuffixNECell " > * > tr:first-of-type > td:last-of-type"
#define DFTableSuffixSWCell " > * > tr:last-of-type > td:first-of-type"
#define DFTableSuffixSECell " > * > tr:last-of-type > td:last-of-type"

typedef struct CSSStyle CSSStyle;

struct CSSStyle {
    size_t retainCount;

    char *selector;
    char *elementName;
    char *className;
    Tag tag;
    int headingLevel; // 0 for all non-heading styles, 1-6 otherwise
    StyleFamily family;

    struct DFHashTable *rules;
    DFCallback *changeCallbacks;
    int latent;
    int dirty;
};

/**
 Create a new CSSStyle object, with the specified identifier. In addition to initialising the
`selector` field, this also sets the `elementName`, `className`, `tag`, `headingLevel`, and `family`
 fields based on the content of the selector.

 When working with stylesheets, you should never call this function yourself. Instead, call
 CSSSheetLookupSelector() with the `add` parameter set to 1. This way, if a style already exists
 in the stylesheet with the same name, you will not end up with a duplicate.
 */
CSSStyle *CSSStyleNew(const char *ident);
CSSStyle *CSSStyleRetain(CSSStyle *style);
void CSSStyleRelease(CSSStyle *style);

/**
 Change the selector associated with the style. Like CSSStyleNew(), this also sets the
 `elementName`, `className`, `tag`, `headingLevel`, and `family` fields appropriately.

 To maintain consistency with the hash table maintained by CSSStyle, you should remove the style
 from the stylesheet before changing the selector, and add it back again afterwards. This way, it
 will maintain the correct mapping from selectors to styles, so that subsequent calls to
 CSSSheetLookupSelector() with the new selector will work correctly --- i.e. by returning this
 object. Note that when removing the style from the stylesheet, the reference count will be
 decremented, so you should temporarily grab an additional reference to this object using
 CSSStyleRetain() and later release it with CSSStyleRelease(). See replaceSelectorsInSheet() for an
 example of where this is done.
 */
void CSSStyleSetSelector(CSSStyle *style, const char *selector);

char *CSSStyleCopyDisplayName(CSSStyle *style);
void CSSStyleSetDisplayName(CSSStyle *style, const char *newDisplayName);
char *CSSStyleCopyParent(CSSStyle *style);
void CSSStyleSetParent(CSSStyle *style, const char *newParent);
char *CSSStyleCopyNext(CSSStyle *style);
void CSSStyleSetNext(CSSStyle *style, const char *newNext);
int CSSStyleIsCustom(CSSStyle *style);

/**
 Get the default rule (the one with an empty prefix) for this style
 */
CSSProperties *CSSStyleRule(CSSStyle *style);
int CSSStyleIsEmpty(CSSStyle *style);

/**
 Get the rule whose properties apply to cells within a table, assuming this style is a table style.
 This is equivalent to calling CSSStyleRuleForSuffix() with a parameter of `" > * > tr > td"`.
 */
CSSProperties *CSSStyleCell(CSSStyle *style);

/**
 Get the rule whose properties define content and its appearance that should appear before elements
 associated with this style. This is equivalent to calling CSSStyleRuleForSuffix() with a parameter
 of `"::before"`.
 */
CSSProperties *CSSStyleBefore(CSSStyle *style);

/**
 Retrieve a NULL-terminated array of suffixes corresponding to the rules in this style. This array
 can still be used after changes to the style object, as contains its own copy of the suffixes.

 The returned array is newly-allocated, and must be freed by the caller. The strings are allocated
 in the same block of memory as the array itself, so a single call to `free` is sufficient.

 @see DFHashTableCopyKeys()
 */
const char **CSSStyleCopySuffixes(CSSStyle *style);

/**
 Get the rule associated with this style that has the specified suffix. The `suffix` parameter must
 be non-NULL, but you can pass the empty string to retrive the default rule.

 @see CSSStyleRule()
 @see CSSStyleCell()
 @see CSSStyleBefore()
 */
CSSProperties *CSSStyleRuleForSuffix(CSSStyle *style, const char *suffix);

/**
 Get the rule associated with a particular aspect of table formatting, such as the first row or
 column. The `tableComponent` parameter should be one of the macros defined near the top of this
 file, such as `DFTableSuffixFirstRow`.
 */
CSSProperties *CSSStyleRuleForTableComponent(CSSStyle *style, const char *tableComponent);

/**
 Set the default properties that are used by typical web browser for this style. With a regular HTML
 file, if you specify e.g. a `h1` element, the browser knows to display that in a larger, bold font.
 This is because browsers have internal defaults for certain built-in elements. However, when
 converting to other file formats such as docx and ODF, these built-in defaults are not present, so
 we need to make the properties explicit.

 The purpose of this function is to set those properties, so that a document converted from HTML to
 another format will have the defaults set explicitly. It should be called when you are adding a new
 style to an non-HTML document, and you want to make sure the apperance matches that of the HTML
 file as displayed by a browser.
 */
void CSSStyleAddDefaultHTMLProperties(CSSStyle *style);

void CSSStylePrint(CSSStyle *style, const char *indent);

#endif
