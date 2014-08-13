//
//  WordNotes.h
//  DocFormats
//
//  Created by Peter Kelly on 15/03/2014.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_WordNotes_h
#define DocFormats_WordNotes_h

#include "DFDOM.h"
#include "DFHashTable.h"

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

typedef struct WordNote WordNote;

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

typedef struct WordNoteGroup WordNoteGroup;

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
