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

#ifndef DocFormats_WordPPr_h
#define DocFormats_WordPPr_h

#include "WordConverter.h"

/** \file */

/**

 Converts paragraph formatting information represented in Word's XML format into a set of CSS
 properties. This function can be used for both named style definitions and direct formatting.

 Paragraph formatting information in Word documents is stored in a `<pPr>` element, which has zero
 or more child elements specifying different properties, such as text alignment and background
 color. This function looks through all the children of the `<pPr>` element, and for each child,
 converts its values to their equivalents in CSS. Some of Word's formatting properties have to
 equivalents in CSS and are not translated.

 The CSSProperties object passed into this function must be created by the caller, and may either be
 empty, or contain other properties obtained from another function, such as WordGetRPR(). This
 function simply adds any new properties that it finds without removing those that are already
 there.

 Example:

     DFNode *pPr = ...
     CSSProperties *properties = CSSPropertiesNew();
     const char *styleId = NULL;
     WordGetPPr(pPr,properties,&styleId,converter.mainSection);

 @param pPr A `<pPr>` element contained within a style definition (for named styles) or a paragraph
 element (for direct formatting).

 @param properties Used to pass back the translated CSS properties

 @param styleId Used to pass back the style name specified for the paragraph, via the val attribute of
 a `<pStyle>` child. If there is no `<pStyle>` child, then the value passed back will be NULL. If
 the style parameter itself is NULL, no value will be passed back. The style name is only relevant
 for `<pPr>` elements that are specified in the document content itself.

 @param section The section of the document in which the paragraph with this styling information is
 contained. Note that this use of the term "section" is completely unrelated to headings.

 \see WordPutPPR()

 */
void WordGetPPr(DFNode *pPr, CSSProperties *properties, const char **styleId, struct WordSection *section);

/**

 Updates formatting information stored in an existing `<pPr>` element in a Word document, based on
 CSS properties, a style name, and an outline level from the HTML document or CSS stylesheet. This
 function can be used for both named style definitions and direct formatting.

 The update is performed in a non-destructive manner. The function first calls WordGetPPr() to
 obtain the "old" CSS properties, and compares these to the "new" CSS properties supplied by the
 caller. Only in cases where these differ are changes made to the <pPr> element or its children.
 This guarantees that information that could not be losslessly translated into CSS is maintained if
 no changes have been made. An example is the different types of underlining that can be applied to
 text in Word, where CSS only supports a single type.

 @param pPr A `<pPr>` element contained within a style definition (for named styles) or a paragraph
 element (for direct formatting). This element should already exist in the document. If a new style
 or paragraph is being added to the Word document, the caller should create this element before
 calling the function. If, upon return, the <pPr> element has no children, the caller may safely
 remove it from the document.

 @param properties The CSS properties to set on the paragraph. As mentioned above, changes are only
 made in cases where the value of a property (or the presence or absence of that property) differs
 from that of the same property returned by WordGetPPr(). Properties which were present in the old
 set but absent in the new one cause the relevant information to be removed from the <pPr> element.

 @param newStyle The style name to be set on the paragraph. Applicable only to `<pPr>` elements
 residing directly in paragraphs.

 @param section The section of the document in which the paragraph with this styling information is
 contained. Note that this use of the term "section" is completely unrelated to headings.

 @param outlineLvl For named styles, the outline level that paragraphs with this style should have.
 This parameter should be in the range 0..5 for styles that cause their elements to appear in the
 outline, or -1 for styles whose elements should not appear in the outline. These values correspond
 to the` <h1>`..`<h6>` and `<p>` elements in HTML. This parameter should be set to -1 for direct
 formatting.

 \see WordGetPPr()

 */
void WordPutPPr(DFNode *pPr, CSSProperties *properties, const char *styleId, struct WordSection *section, int outlineLvl);

#endif
