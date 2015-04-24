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

#ifndef DocFormats_WordNotes_h
#define DocFormats_WordNotes_h

#include "DFDOM.h"
#include "DFHashTable.h"
#include "OOXMLTypedefs.h"

struct WordGetData;
struct WordPutData;
struct WordConverter;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         Lens functions                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFNode *WordRunGetNote(struct WordGetData *get, DFNode *concrete);
int WordRunPutNote(struct WordPutData *put, DFNode *abstract, DFNode *concrete);
void WordNoteReferenceRemove(struct WordPutData *put, DFNode *concrete);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordNote                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordNote {
    size_t retainCount;
    DFNode *element;
    int noteId;
};

WordNote *WordNoteNew(DFNode *element, int noteId);
WordNote *WordNoteRetain(WordNote *note);
void WordNoteRelease(WordNote *note);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          WordNoteGroup                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct WordNoteGroup {
    size_t retainCount;
    DFHashTable *notesById;
    int nextNoteId;

    /* XML document containing all notes of a particular type. Generally named footnotes.xml or
       endnotes.xml */
    DFDocument *doc;

    /* Type of element used to store the content of the note.
       Either WORD_FOOTNOTE or WORD_ENDNOTE. */
    Tag noteTag;

    /* Type of element used to store a reference to a note.
       Either WORD_FOOTNOTEREFERENCE OR WORD_ENDNOTEREFERENCE. */
    Tag refTag;

    /* Name of style used for the runs that contain references to this type of note. This is *not*
       the same as the styleId (which can differ between languages), but is instead the name property
     of the style as given in styles.xml. Either "footnote reference" or "endnote reference". */
    const char *refStyleName;

    /* Class name to use on HTML <span> elements for this kind of note. In HTML, the content is stored
       in-line, not as a reference. */
    const char *htmlClass;
};

WordNoteGroup *WordNoteGroupNewFootnotes(DFDocument *doc);
WordNoteGroup *WordNoteGroupNewEndnotes(DFDocument *doc);
WordNoteGroup *WordNoteGroupRetain(WordNoteGroup *group);
void WordNoteGroupRelease(WordNoteGroup *group);

WordNote *WordNoteGroupGet(WordNoteGroup *group, int noteId);
WordNote *WordNoteGroupAddNew(WordNoteGroup *group);
void WordNoteGroupRemove(WordNoteGroup *group, int noteId);

#endif
