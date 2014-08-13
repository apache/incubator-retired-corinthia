//
//  DFAllocator.h
//  DocFormats
//
//  Created by Peter Kelly on 2/03/13.
//  Copyright (c) 2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_DFAllocator_h
#define DocFormats_DFAllocator_h

#include "DFTypes.h"

typedef struct DFAllocator DFAllocator;

DFAllocator *DFAllocatorNew(void);
void DFAllocatorFree(DFAllocator *alc);
void *DFAllocatorAlloc(DFAllocator *alc, size_t size);

#endif
