//
//  DFChanges.h
//  dfutil
//
//  Created by Peter Kelly on 6/11/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef dfutil_DFChanges_h
#define dfutil_DFChanges_h

#include "DFDOM.h"

void DFComputeChanges(DFNode *root1, DFNode *root2, Tag idAttr);
char *DFChangesToString(DFNode *root);

#endif
