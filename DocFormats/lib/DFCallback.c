//
//  DFCallback.c
//  DocFormats
//
//  Created by Peter Kelly on 25/11/2013.
//  Copyright (c) 2014 UX Productivity. All rights reserved.
//

#include "DFCallback.h"
#include "DFCommon.h"

void DFCallbackAdd(DFCallback **list, DFCallbackFunction fun, void *ctx)
{
    DFCallback *item = (DFCallback *)calloc(1,sizeof(DFCallback));
    item->fun = fun;
    item->ctx = ctx;
    item->next = *list;
    *list = item;
}

void DFCallbackRemove(DFCallback **list, DFCallbackFunction fun, void *ctx)
{
    while (*list != NULL) {
        DFCallback *item = *list;
        if ((item->fun == fun) && (item->ctx == ctx)) {
            *list = item->next;
            free(item);
            return;
        }
    }
    assert(!"Callback not found");
}

void DFCallbackInvoke(DFCallback *list, void *object, void *data)
{
    for (DFCallback *item = list; item != NULL; item = item->next)
        item->fun(item->ctx,object,data);
}
