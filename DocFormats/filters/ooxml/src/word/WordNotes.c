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

#include "DFPlatform.h"
#include "WordNotes.h"
#include "WordConverter.h"
#include "WordSheet.h"
#include "WordLenses.h"
#include "DFString.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         Lens functions                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *getActualRunStyleName(WordSheet *styles, DFNode *concrete)
{
    if (concrete->tag != WORD_R)
        return NULL;;

    DFNode *rPr = DFChildWithTag(concrete,WORD_RPR);
    if (rPr == NULL)
        return NULL;;

    DFNode *rStyle = DFChildWithTag(rPr,WORD_RSTYLE);
    if (rStyle == NULL)
        return NULL;;

    const char *styleId = DFGetAttribute(rStyle,WORD_VAL);
    return WordSheetNameForStyleId(styles,"character",styleId);
}

static WordNoteGroup *noteGroupForRun(WordGetData *get, DFNode *concrete)
{
    const char *actualName = getActualRunStyleName(get->conv->styles,concrete);

    if (DFStringEquals(actualName,get->conv->footnotes->refStyleName))
        return get->conv->footnotes;
    else if (DFStringEquals(actualName,get->conv->endnotes->refStyleName))
        return get->conv->endnotes;
    else
        return NULL; // This run is neither a footnote or endnote reference
}

static int trimLeadingWhitespace(DFNode *concrete)
{
    if (concrete->tag == DOM_TEXT) {
        char *value = DFStringTrimLeadingWhitespace(concrete->value);
        concrete->value = DFCopyString(concrete->doc,value);
        free(value);
        return 1;
    }
    else {
        for (DFNode *child = concrete->first; child != NULL; child = child->next) {
            if (trimLeadingWhitespace(child))
                return 1;
        }
        return 0;
    }
}

static void removeNoteMarker(WordSheet *styles, DFNode *concrete)
{
    DFNode *firstRun = DFChildWithTag(concrete,WORD_R);
    DFNode *rPr = DFChildWithTag(firstRun,WORD_RPR);
    DFNode *rStyle = DFChildWithTag(rPr,WORD_RSTYLE);
    const char *styleId = DFGetAttribute(rStyle,WORD_VAL);
    if (styleId == NULL)
        return;;

    WordStyle *style = WordSheetStyleForTypeId(styles,"character",styleId);
    if (style == NULL)
        return;

    if (DFStringEquals(style->name,"footnote reference") ||
        DFStringEquals(style->name,"endnote reference")) {
        DFRemoveNode(firstRun);
    }
    trimLeadingWhitespace(concrete);
}

DFNode *WordRunGetNote(WordGetData *get, DFNode *concrete)
{
    WordNoteGroup *group = noteGroupForRun(get,concrete);
    if (group == NULL)
        return NULL; // This run is neither a footnote or endnote reference

    // Check that the run contains *only* a <footnoteReference> or <endnoteReference>, in addition
    // to the <rPr> used to define the style name. If there's any other content in there, we don't
    // treat it as a properly-formed reference, and WordRunGet will handle it as a normal run.
    for (DFNode *child = concrete->first; child != NULL; child = child->next) {
        if ((child->tag != group->refTag) && (child->tag != WORD_RPR))
            return NULL;
    }

    // Look in footnotes.xml or endnotes.xml to get the content of the note this reference points to
    const char *noteIdStr = DFGetChildAttribute(concrete,group->refTag,WORD_ID);
    WordNote *note = (noteIdStr != NULL) ? WordNoteGroupGet(group,atoi(noteIdStr)) : NULL;
    if (note == NULL)
        return NULL;;

    // Create a HTML <span> element of class "footnote" or "endnote", with the plain text of note note
    // as content. The use of text-only content is a temporary measure; it ultimately needs to traverse
    // the element hierarchy to extract formatting and other details within the note.
    DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
    DFSetAttribute(span,HTML_CLASS,group->htmlClass);

    int paragraphCount = 0;
    for (DFNode *child = note->element->first; child != NULL; child = child->next) {
        if (child->tag == WORD_P)
            paragraphCount++;
    }

    for (DFNode *child = note->element->first; child != NULL; child = child->next) {
        if (child->tag == WORD_P) {
            removeNoteMarker(get->conv->styles,child);
            WordContainerGet(get,&WordParagraphContentLens,span,child);
        }
    }
    
    return span;
}

int WordRunPutNote(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    const char *className = DFGetAttribute(abstract,HTML_CLASS);
    int isFootnote = DFStringEquals(className,"footnote");
    int isEndnote = DFStringEquals(className,"endnote");
    if (!isFootnote && !isEndnote)
        return 0;;

    WordStyle *referenceStyle;
    WordStyle *paragraphStyle;

    if (isFootnote) {
        referenceStyle = WordSheetFootnoteReference(put->conv->styles);
        paragraphStyle = WordSheetFootnoteText(put->conv->styles);
    }
    else {
        referenceStyle = WordSheetEndnoteReference(put->conv->styles);
        paragraphStyle = WordSheetEndnoteText(put->conv->styles);
    }

    // Make sure we have a space at the start of the text
    // The current method for doing this (expecting the first child of the first span to be a text node)
    // is not reliable, as there could be other elements such as cross-references or hyperlinks. But this
    // works for plain-text notes (with or without formatting)
    DFNode *childSpan = DFChildWithTag(abstract,HTML_SPAN);
    if (childSpan != NULL) {
        if ((childSpan->first != NULL) && (childSpan->first->tag == DOM_TEXT)) {
            DFNode *childText = childSpan->first;
            char *newValue = DFFormatString(" %s",childText->value);
            childText->value = DFCopyString(abstract->doc,newValue);
            free(newValue);
        }
    }

    // Set the content of the note
    WordNoteGroup *group = isFootnote ? put->conv->footnotes : put->conv->endnotes;

    DFNode *reference = DFChildWithTag(concrete,group->refTag);
    const char *idStr = DFGetAttribute(reference,WORD_ID);
    WordNote *note = (idStr != NULL) ? WordNoteGroupGet(group,atoi(idStr)) : NULL;

    // Convert the HTML span to a paragraph
    DFNode *htmlP = DFCreateElement(abstract->doc,HTML_P);
    while (abstract->first != NULL) {
        DFAppendChild(htmlP,abstract->first);
    }

    DFNode *wordP = NULL;
    if (note != NULL) {
        wordP = DFChildWithTag(note->element,WORD_P);
        if (wordP == NULL)
            wordP = DFCreateChildElement(note->element,WORD_P);
    }
    else {
        note = WordNoteGroupAddNew(group);
        wordP = DFCreateChildElement(note->element,WORD_P);
    }

    // wordP and its descendants must go in footnotes.xml or endnotes.xml, instead of document.xml. Normally,
    // put->contentDoc refers to the latter, but we temporary swap it out so that new nodes created for the
    // footnote or endnote go in the format.
    DFDocument *savedContentDoc = put->contentDoc;
    put->contentDoc = note->element->doc;
    WordParagraphLens.put(put,htmlP,wordP);
    put->contentDoc = savedContentDoc;

    // Remove existing paragraph style information
    DFNode *pPr = DFChildWithTag(wordP,WORD_PPR);
    if (pPr != NULL)
        DFRemoveNode(pPr);

    // Set the paragraph style
    pPr = DFCreateElement(group->doc,WORD_PPR);
    DFNode *pStyle = DFCreateChildElement(pPr,WORD_PSTYLE);
    DFSetAttribute(pStyle,WORD_VAL,paragraphStyle->styleId);
    DFInsertBefore(wordP,pPr,wordP->first);

    // Add the note ref element at the start
    DFNode *markR = DFCreateElement(group->doc,WORD_R);
    DFNode *markRPr = DFCreateChildElement(markR,WORD_RPR);
    DFNode *markRStyle = DFCreateChildElement(markRPr,WORD_RSTYLE);
    DFSetAttribute(markRStyle,WORD_VAL,referenceStyle->styleId);
    if (isFootnote)
        DFCreateChildElement(markR,WORD_FOOTNOTEREF);
    else
        DFCreateChildElement(markR,WORD_ENDNOTEREF);
    DFInsertBefore(wordP,markR,pPr->next);

    // Make the word run a footnote reference
    while (concrete->first != NULL)
        DFRemoveNode(concrete->first);

    DFNode *refRPr = DFCreateChildElement(concrete,WORD_RPR);
    DFNode *refRStyle = DFCreateChildElement(refRPr,WORD_RSTYLE);
    DFSetAttribute(refRStyle,WORD_VAL,referenceStyle->styleId);

    DFNode *refNode = DFCreateChildElement(concrete,group->refTag);
    DFFormatAttribute(refNode,WORD_ID,"%d",note->noteId);

    return 1;
}

void WordNoteReferenceRemove(struct WordPutData *put, DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_FOOTNOTEREFERENCE: {
            const char *idStr = DFGetAttribute(concrete,WORD_ID);
            if (idStr != NULL)
                WordNoteGroupRemove(put->conv->footnotes,atoi(idStr));
            break;
        }
        case WORD_ENDNOTEREFERENCE: {
            const char *idStr = DFGetAttribute(concrete,WORD_ID);
            if (idStr != NULL)
                WordNoteGroupRemove(put->conv->endnotes,atoi(idStr));
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordNote                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordNote *WordNoteNew(DFNode *element, int noteId)
{
    WordNote *note = (WordNote *)calloc(1,sizeof(WordNote));
    note->retainCount = 1;
    note->element = element;
    note->noteId = noteId;
    return note;
}

WordNote *WordNoteRetain(WordNote *note)
{
    if (note != NULL)
        note->retainCount++;
    return note;
}

void WordNoteRelease(WordNote *note)
{
    if ((note == NULL) || (--note->retainCount > 0))
        return;

    free(note);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordNoteGroup                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static void findNotes(WordNoteGroup *group)
{
    for (DFNode *child = group->doc->root->first; child != NULL; child = child->next) {
        if (child->tag == group->noteTag) {
            const char *noteIdStr = DFGetAttribute(child,WORD_ID);
            if (noteIdStr != NULL) {
                int noteId = atoi(noteIdStr);
                WordNote *note = WordNoteNew(child,noteId);
                DFHashTableAddInt(group->notesById,noteId,note);
                WordNoteRelease(note);
            }
        }
    }
}

static WordNoteGroup *WordNoteGroupNew(DFDocument *doc, Tag noteTag, Tag refTag,
                                       const char *refStyleName, const char *htmlClass)
{
    assert(doc != NULL);
    assert((noteTag == WORD_FOOTNOTE) || (noteTag == WORD_ENDNOTE));

    WordNoteGroup *group = (WordNoteGroup *)calloc(1,sizeof(WordNoteGroup));
    group->retainCount = 1;
    group->notesById = DFHashTableNew((DFCopyFunction)WordNoteRetain,(DFFreeFunction)WordNoteRelease);
    group->doc = DFDocumentRetain(doc);
    group->noteTag = noteTag;
    group->refTag = refTag;
    group->refStyleName = refStyleName;
    group->htmlClass = htmlClass;

    findNotes(group);

    return group;
}

WordNoteGroup *WordNoteGroupNewFootnotes(DFDocument *doc)
{
    return WordNoteGroupNew(doc,
                            WORD_FOOTNOTE,
                            WORD_FOOTNOTEREFERENCE,
                            "footnote reference",
                            "footnote");
}

WordNoteGroup *WordNoteGroupNewEndnotes(DFDocument *doc)
{
    return WordNoteGroupNew(doc,
                            WORD_ENDNOTE,
                            WORD_ENDNOTEREFERENCE,
                            "endnote reference",
                            "endnote");
}


WordNoteGroup *WordNoteGroupRetain(WordNoteGroup *group)
{
    if (group != NULL)
        group->retainCount++;
    return group;
}

void WordNoteGroupRelease(WordNoteGroup *group)
{
    if ((group == NULL) || (--group->retainCount > 0))
        return;

    DFHashTableRelease(group->notesById);
    DFDocumentRelease(group->doc);
    free(group);
}

WordNote *WordNoteGroupGet(WordNoteGroup *group, int noteId)
{
    return (WordNote *)DFHashTableLookupInt(group->notesById,noteId);
}

WordNote *WordNoteGroupAddNew(WordNoteGroup *group)
{
    while (DFHashTableLookupInt(group->notesById,group->nextNoteId) != NULL)
        group->nextNoteId++;

    int noteId = group->nextNoteId++;
    DFNode *element = DFCreateChildElement(group->doc->root,group->noteTag);
    DFFormatAttribute(element,WORD_ID,"%d",noteId);
    WordNote *note = WordNoteNew(element,noteId);
    DFHashTableAddInt(group->notesById,noteId,note);
    WordNoteRelease(note); // The hash table retains a reference to the note
    return note;
}

void WordNoteGroupRemove(WordNoteGroup *group, int noteId)
{
    WordNote *note = DFHashTableLookupInt(group->notesById,noteId);
    if (note != NULL) {
        DFRemoveNode(note->element);
        DFHashTableRemoveInt(group->notesById,noteId);
    }
}
