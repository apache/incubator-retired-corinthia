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

#ifndef DocFormats_OPC_h
#define DocFormats_OPC_h

#include <DocFormats/DFXMLForward.h>
#include <DocFormats/DFError.h>
#include "DFBuffer.h"
#include <DocFormats/DFStorage.h>
#include "OOXMLTypedefs.h"

// Essentials of the Open Packaging Conventions
// http://msdn.microsoft.com/en-us/library/ee361919(office.11).aspx

/*

 When opening the file, OPCPackage traverses the part/relationship graph and creates OPCPart
 and OPCRelationship objects for every part and relationship. This is safe because these comprise
 only a small amount of information, and we can reduce the cases where errors need to be handled
 to package open/close and part read/write. Once a package is open, you can therefore access all
 the parts and relationships without having to check for errors.

 */

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         OPCRelationship                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct OPCRelationship {
    size_t retainCount;
    char *rId;
    char *type;
    char *target;
    int external;
    int needsRemoveCheck;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       OPCRelationshipSet                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

OPCRelationshipSet *OPCRelationshipSetNew(void);
void OPCRelationshipSetFree(OPCRelationshipSet *set);

const char **OPCRelationshipSetAllIds(OPCRelationshipSet *set);
OPCRelationship *OPCRelationshipSetLookupById(OPCRelationshipSet *set, const char *rId);
OPCRelationship *OPCRelationshipSetLookupByType(OPCRelationshipSet *set, const char *type);
OPCRelationship *OPCRelationshipSetAddId(OPCRelationshipSet *set,
                                         const char *rId, const char *type,
                                         const char *target, int external);
OPCRelationship *OPCRelationshipSetAddType(OPCRelationshipSet *set,
                                           const char *type, const char *target, int external);
void OPCRelationshipSetRemove(OPCRelationshipSet *set, OPCRelationship *rel);
DFDocument *OPCRelationshipSetToDocument(OPCRelationshipSet *set);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             OPCPart                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct OPCPart {
    size_t retainCount;
    char *URI;
    char *contentType;
    OPCRelationshipSet *relationships;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         OPCContentTypes                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

void OPCContentTypesSetDefault(OPCContentTypes *ct, const char *extension, const char *type);
void OPCContentTypesSetOverride(OPCContentTypes *ct, const char *partName, const char *type);
void OPCContentTypesRemoveOverride(OPCContentTypes *ct, const char *partName);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           OPCPackage                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

struct OPCPackage {
    DFStorage *storage;
    int isOpen;
    OPCContentTypes *contentTypes;
    OPCRelationshipSet *relationships;
    DFBuffer *errors;
    struct DFHashTable *partsByName;
};

OPCPackage *OPCPackageOpenNew(DFStorage *storage, DFError **error);
OPCPackage *OPCPackageOpenFrom(DFStorage *storage, DFError **error);
void OPCPackageFree(OPCPackage *pkg);
int OPCPackageSaveToDir(OPCPackage *pkg);
int OPCPackageSave(OPCPackage *pkg, DFError **error);

OPCPart *OPCPackagePartWithURI(OPCPackage *pkg, const char *URI);
OPCPart *OPCPackageAddRelatedPart(OPCPackage *pkg, const char *URI, const char *contentType,
                                  const char *relType, OPCPart *source);
void OPCPackageRemoveRelatedPart(OPCPackage *pkg, const char *URI, const char *relType, OPCPart *source);
DFBuffer *OPCPackageReadPart(OPCPackage *pkg, OPCPart *part, DFError **error);
int OPCPackageWritePart(OPCPackage *pkg, const char *data, size_t len, OPCPart *part, DFError **error);
int OPCPackageDeletePart(OPCPackage *pkg, OPCPart *part, DFError **error);
void OPCPackageReadRelationships(OPCPackage *pkg, OPCRelationshipSet *rels, const char *partURI, DFDocument *relDoc);

#endif
