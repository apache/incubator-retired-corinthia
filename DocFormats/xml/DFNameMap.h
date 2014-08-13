//
//  DFNameMap.h
//  DocFormats
//
//  Created by Peter Kelly on 15/09/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFNameMap_h
#define DocFormats_DFNameMap_h

#include "DFXMLNamespaces.h"
#include "DFXMLNames.h"
#include <libxml/xmlstring.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFNameMap                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

/** TODO */
typedef struct DFNameMap DFNameMap;

DFNameMap *DFNameMapNew(void);
void DFNameMapFree(DFNameMap *map);

NamespaceID  DFNameMapFoundNamespace(DFNameMap *map, const xmlChar *URI, const xmlChar *prefix);
const NamespaceDecl *DFNameMapNamespaceForID(DFNameMap *map, NamespaceID nsId);
NamespaceID DFNameMapNamespaceCount(DFNameMap *map);

const TagDecl *DFNameMapNameForTag(DFNameMap *map, Tag tag);
Tag DFNameMapTagForName(DFNameMap *map, const xmlChar *URI, const xmlChar *localName);

const TagDecl *DFBuiltinMapNameForTag(Tag tag);
Tag DFBuiltinMapTagForName(const xmlChar *URI, const xmlChar *localName);

#endif
