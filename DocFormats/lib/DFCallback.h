//
//  DFCallback.h
//  DocFormats
//
//  Created by Peter Kelly on 25/11/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#ifndef DocFormats_DFCallback_h
#define DocFormats_DFCallback_h

typedef void (*DFCallbackFunction)(void *ctx, void *object, void *data);

typedef struct DFCallback DFCallback;

struct DFCallback {
    DFCallbackFunction fun;
    void *ctx;
    DFCallback *next;
};

void DFCallbackAdd(DFCallback **list, DFCallbackFunction fun, void *ctx);
void DFCallbackRemove(DFCallback **list, DFCallbackFunction fun, void *ctx);
void DFCallbackInvoke(DFCallback *list, void *object, void *data);

#endif
