//
//  DFMarkupCompatibility.h
//  DocFormats
//
//  Created by Peter Kelly on 2/10/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFMarkupCompatibility_h
#define DocFormats_DFMarkupCompatibility_h

#include "DFXMLNamespaces.h"
#include "DFXMLNames.h"
#include "DFNameMap.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                      DFMarkupCompatibility                                     //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    MCActionDefault,
    MCActionIgnore,
    MCActionProcessContent,
    MCActionMustUnderstand,
} MCAction;

typedef struct DFMarkupCompatibility DFMarkupCompatibility;

DFMarkupCompatibility *DFMarkupCompatibilityNew(void);
void DFMarkupCompatibilityFree(DFMarkupCompatibility *mc);
void DFMarkupCompatibilityPush(DFMarkupCompatibility *mc, int nb_namespaces, const xmlChar **namespaces, DFNameMap *map);
void DFMarkupCompatibilityPop(DFMarkupCompatibility *mc);
MCAction DFMarkupCompatibilityLookup(DFMarkupCompatibility *mc, NamespaceID nsId, Tag tag, int isElement);
void DFMarkupCompatibilityProcessAttr(DFMarkupCompatibility *mc, Tag attr, const char *value, DFNameMap *map);

#endif
