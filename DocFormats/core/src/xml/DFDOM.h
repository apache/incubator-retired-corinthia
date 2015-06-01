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

#ifndef DocFormats_DFDOM_h
#define DocFormats_DFDOM_h

/** \file

 # DocFormats Document Object Model (DOM)

 This file defines data structures and functions for manipulating parsed XML data, represented in
 memory as a tree. It is inspired by the Document Object Model (DOM) API commonly used for
 processing XML data, but does not follow the API strictly.

 The two primary classes are DFDocument and DFNode. A DFDocument represents a parsed XML or HTML
 document, and acts as a container for all of the nodes within the document. A DFNode object
 represents either an element (represented textually as `<element-name> ... </element-name>`) or a
 text node (containing literal text from the XML document, residing inside an element).

 Every DFDocument object has a root note, and every DFNode object has a doubly-linked list of
 children. You can traverse the tree of nodes by using the first and next fields defined in the
 DFNode struct, and determine the element or node type via the \ref tag field, and the textual
 content (for text nodes) via the \value object.

 ## Tags

 Naming of XML elements and attributes is considerably complicated by the use of namespaces.
 Conceptually, every element is identified by a (namespcae URI, local name) pair, which determines
 its semantic meaning. However, these pairs are not directly specified in source files, as namespace
 URIs can be quite long. Instead, the XML Namespace specification [1] specifies a mechanism to map
 *prefixes* to namespace URIs, and elements in the textual representation are named based on a
 (prefix, local name) combination. With this mechanism, a program reading an XML document must first
 look up the prefix mapping to determine the namespace URI, before it can correctly identify an
 element. Worse, the mapping between prefixes and namespace URI can differ between different parts
 of the document.

 DocFormats uses an in-memory representation of XML elements in which the name is replaced by a
 numeric *tag*. Each possible tag corresponds to a (namespace URI, local name) pair, which
 simplifies checking the types of elements to a simple integer comparison, rather than a complicated
 symbol resolution algorithm. Furthermore, these tags can optionally be declared as pre-defined
 constants, as in \ref XMLNames.h, enabling them to be used in switch statements --- which improves
 performance of code that has to check for many different element types.

 Each document has a \ref DFNameMap object which stores the information necessary to map between
 numeric tags and (namespace URI, local name) pairs, and also stores the default prefix to be used
 for ecah namespace URI. During parsing, as carried out DFParseXMLFile() and DFParseXMLString(),
 symbol resolution based on the textual names given in the source file is performed, resulting in
 the tags in the constructed tree corresponding to either pre-defined tags in the default name map,
 or other tags whose numeric values have been dynamically allocated during parsing. During
 serialisation, namespace mappings based on the default prefix for each namespace URI are set on the
 root element of the output file, and tags are translated into (prefix:local name) combinations for
 the actual start and end tags of elements.

 ## Memory allocation

 DFDocument objects are reference-counted, but the memory occupied by all DFNode objects is managed
 by their containing documents. As you create new nodes, the document itself takes care of
 allocating memory for them, and also keeping track of all the nodes it has allocated. When a
 document is freed --- that is, when its reference count drops to zero --- all nodes allocated by
 that document are also freed.

 This approach is for performance reasons. In an earlier version of the library, each DFNode object
 was allocated separately, as a reference-counted Objective C object. This implied large overheads
 both for incrementing and decrementing the reference count, and for traversing through the node
 tree releasing all the references individually. The approach used now, implemented by DFAllocator,
 makes only a few calls to `malloc` to allocate large chunks of memory, and then allocates portions
 of those memory blocks itself for the DFNode objects. When a DFDocument object is freed, those
 blocks are simply released in one go, without the need to individually inspect each node to check
 its reference count. The same memory blocks are used to allocate strings for attribute values,
 which also get freed along with the document.

 THe previous paragraph describes an implementation detail which you don't need to worry about
 when using documents. Just call DFDocumentNew() or DFDocumentNewWithRoot() to create a document,
 and DFDocumentRelease() when you are finished with it. All memory that has been allocated for
 nodes, attribute values, and text node values will automatically be freed when the document's
 reference count drops to zero, as it would after a simple new/release sequence.

 */

#include "DFXMLNamespaces.h"
#include "DFXMLNames.h"
#include <DocFormats/DFXMLForward.h>
#include "DFBuffer.h"
#include <stdarg.h>

#define DOM_DOCUMENT                 1
#define DOM_TEXT                     2
#define DOM_COMMENT                  3
#define DOM_CDATA                    4
#define DOM_PROCESSING_INSTRUCTION   5

#define MIN_ELEMENT_TAG              10

typedef struct {
    Tag tag;
    char *value;
} DFAttribute;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFNode                                             //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

/** Documentation for DFNode */
struct DFNode {
    Tag tag;
    DFNode *parent;
    DFNode *first;
    DFNode *last;
    DFNode *next;
    DFNode *prev;
    unsigned int seqNo;
    struct DFDocument *doc;
    void *js;
    int changed;
    int childrenChanged;
    DFNode *seqNoHashNext;
    DFAttribute *attrs;
    unsigned int attrsCount;
    unsigned int attrsAlloc;
    char *target;
    char *value;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           DFDocument                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#define SEQNO_HASH_SIZE 3571

/**

 The DFDocument class represents an XML or HTML document in memory which has either been parsed from
 a file, or has been newly-created and is destined to be serialised to a file.

 */
struct DFDocument {
    size_t retainCount;
    struct DFAllocator *allocator;
    DFNode *seqNoHashBins[SEQNO_HASH_SIZE];
    struct DFHashTable *nodesByIdAttr;
    DFNode **nodes;
    size_t nodesCount;
    size_t nodesAlloc;

    struct DFNameMap *map;
    DFNode *docNode;
    DFNode *root;
    unsigned int nextSeqNo;
};

/**
 * Create a new DFDocument
 *
 * This document has no root node!
 *
 */
DFDocument *DFDocumentNew(void);
/**
 * Create a new DFDocument with a root element of rootTag type
 */
DFDocument *DFDocumentNewWithRoot(Tag rootTag);
/**
 * Increment the document reference count
 */
DFDocument *DFDocumentRetain(DFDocument *doc);
/**
 * Decrement the document reference count and free if zero
 */
void DFDocumentRelease(DFDocument *doc);

/**
 * This restarts the sequence numbers associated with the
 * document node.
 *
 * Not clear what these are for or when they are used
 */
void DFDocumentReassignSeqNos(DFDocument *doc);

/**
 * Find a DFNode given its sequence number
 *
 * Returns NULL if not found
 */
DFNode *DFNodeForSeqNo(DFDocument *doc, unsigned int seqNo);
/**
 * A DFDocument has an associated list of nodes indexed by the HTML_ID Attribute
 *
 * This function gets the DFNode that has HTML_ID attrbute value of idAttr.
 *
 * Returns NULL if not found.
 */
DFNode *DFElementForIdAttr(DFDocument *doc, const char *idAttr);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                               DOM                                              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Strings

/**
 * Allocate memory within the document and write the string to it
 */
char *DFCopyString(DFDocument *doc, const char *str);

// Document methods

/**
 * Create a tagged DFNode within the document
 *
 * Note this node is floating in that it is not part of the
 * document tree until inserted or appended somewhere.
 */
DFNode *DFCreateElement(DFDocument *doc, Tag tag);
/**
 * Create a text DFNode within the document with the data supplied
 *
 * Note this node is floating in that it is not part of the
 * document tree until inserted or appended somewhere.
 *
 * Q. Are we relying on the allocated contents of the node being set to zero
 * And there for the first, next, prev, and last pointers being NULL?
 *
 * Will this always be true?
 */
DFNode *DFCreateTextNode(DFDocument *doc, const char *data);
/**
 * Create a DOM comment DFNode within the document with the data supplied
 *
 * Note this node is floating in that it is not part of the
 * document tree until inserted or appended somewhere.
 */
DFNode *DFCreateComment(DFDocument *doc, const char *data);
/**
 * Create a CDATA DFNode within the document with the data supplied
 *
 * Note this node is floating in that it is not part of the
 * document tree until inserted or appended somewhere.
 */
DFNode *DFCreateCDATASection(DFDocument *doc, char *data);
/**
 * Create a Processing Instruction DFNode within the document with the data supplied
 *
 * Note this node is floating in that it is not part of the
 * document tree until inserted or appended somewhere.
 */
DFNode *DFCreateProcessingInstruction(DFDocument *doc, const char *target, const char *content);

// Node methods

/**
 * Insert the newChild DFNode before refChild below parent.
 *
 * There are a bunch of internal assertion checks here.
 * Just don't mess up and all will be well, the newChild will be correctly
 * inserted into the parent's doubly-linked list of children before refChild.
 *
 * If refChild is NULL and the newChild is appended to the list.
 */
void DFInsertBefore(DFNode *parent, DFNode *newChild, DFNode *refChild);
/**
 * Append newChild to the parent list.
 */
void DFAppendChild(DFNode *parent, DFNode *newChild);
/**
 * Create a tagged DFNode and append it to the parent
 */
DFNode *DFCreateChildElement(DFNode *parent, Tag tag);
/**
 * Create a text DFNode and append it to the parent
 */
DFNode *DFCreateChildTextNode(DFNode *parent, const char *data);
/**
 * Remove the node the the list in which it is contained.
 *
 * Looks like the node can be reused and does not need to be freed.
 * If not reused directly is it allocated later ?
 */
void DFRemoveNode(DFNode *node);
/**
 * Bubble up the children of node to its parent.
 *
 * The first child will be located where node was in its parent's list.
 *
 * node is can then be reused.
 */
void DFRemoveNodeButKeepChildren(DFNode *node);
/**
 * Copy the input value to node.
 *
 * If a node already has a value is it leaked?
 */
void DFSetNodeValue(DFNode *node, const char *value);

// Element methods

/**
 * A DFNode has an internal array of attributes.
 * An attribute is an TAG value pair.
 *
 * This function gets the value of tag attrbute.
 *
 * Returns NULL if not found.
 */
const char *DFGetAttribute(DFNode *node, Tag tag);
/**
 * Get the attrTag value from parent's childTag node.
 *
 * Returns NULL if not found
 */
const char *DFGetChildAttribute(DFNode *parent, Tag childTag, Tag attrTag);
/**
 * Set or add the tag : value pair of the element DFNode
 *
 * If the tag is HTML_ID then correct or add an entry in the internal map of id attributes
 *
 * If value is NULL remove the attribute from the DFNode
 *
 */
void DFSetAttribute(DFNode *element, Tag tag, const char *value);
/**
 * Set the elements tag attrbute to the variable formatted value.
 *
 * So set with a printf like value.
 */
void DFVFormatAttribute(DFNode *element, Tag tag, const char *format, va_list ap);
/**
 * Set the tag attrbute of the element node to the variable formatted value.
 *
 * So set with a printf like value.
 */
void DFFormatAttribute(DFNode *element, Tag tag, const char *format, ...) ATTRIBUTE_FORMAT(printf,3,4);
/**
 * Remove the tag attribute of the element node
 */
void DFRemoveAttribute(DFNode *element, Tag tag);
/**
 * Function name says it all.
 */
void DFRemoveAllAttributes(DFNode *element);

// Tree traversal

/**
 * This function along with DFNextNodeAfter and DFNextNode
 * provide a means of walking through the tree of DFNode elements
 *
 * Repeatedly calling DFPrevNode will perform a reverse walk of the elements under node
 */
DFNode *DFPrevNode(DFNode *node);
/**
 * Move to the next node. Which may be a child
 * the next sibling
 * or NULL
 */
DFNode *DFNextNodeAfter(DFNode *node);
/**
 * The next from node is either the first child
 * or the sibling to the right in the tree
 * or NULL if we are at the end of the list
 *
 * So repeatedly calling DFNextNode will walk the tree below node until the return value is NULL
 */
DFNode *DFNextNode(DFNode *node);

// Names

/**
 * To be explained
 */
Tag DFLookupTag(DFDocument *doc, const char *URI, const char *name);
/**
 * For this doc return the name associated with tag
 */
const char *DFTagName(DFDocument *doc, Tag tag);
/**
 * To be explained
 */
const char *DFTagURI(DFDocument *doc, Tag tag);
/**
 * Return the name of the node
 *
 * or "#document", "#text", "#comment", "#cdata-section";
 *
 */
const char *DFNodeName(DFNode *node);
/**
 * return the namespace URI of the node
 */
const char *DFNodeURI(DFNode *node);

// Misc

/**
 * Get all of the text or CDATA below node into buf.
 * One big lump...
 */
void DFNodeTextToBuffer(DFNode *node, DFBuffer *buf);
/**
 * Return all if the text or CDATA below node.
 * Calle will have to free it.
 */
char *DFNodeTextToString(DFNode *node);
/**
 * Remove the HTML_ID tags from all elements below node
 */
void DFStripIds(DFNode *node);
/**
 * Get the first child DFNode of parent of type tag
 *
 * Returns NULL if not found
 */
DFNode *DFChildWithTag(DFNode *parent, Tag tag);
/**
 * Remove all whitespace child elements below node
 */
void DFRemoveWhitespaceNodes(DFNode *node);
/**
 * Name says it all
 */
int DFIsWhitespaceNode(DFNode *node);
/**
 * Work to be done on this one as except is ignored at the moment.
 *
 * Currently will compare the attribute of two nodes for an exact match
 */
int identicalAttributesExcept(DFNode *first, DFNode *second, Tag except);
/**
 * Trim the text nodes below (elements removed is all whitespace)
 */
void DFStripWhitespace(DFNode *node);

#endif
