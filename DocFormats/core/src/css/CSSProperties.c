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

#include "CSSProperties.h"
#include "CSS.h"
#include "DFString.h"
#include "DFHashTable.h"
#include "DFCommon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *CSSPropertiesCopyDescription(CSSProperties *properties)
{
    DFHashTable *collapsed = CSSCollapseProperties(properties);
    char *result = CSSSerializeProperties(collapsed);
    DFHashTableRelease(collapsed);
    return result;
}

const char **CSSPropertiesCopyNames(CSSProperties *properties)
{
    return DFHashTableCopyKeys(properties->hashTable);
}

int CSSPropertiesIsEmpty(CSSProperties *properties)
{
    return (DFHashTableCount(properties->hashTable) == 0);
}

const char *CSSGet(CSSProperties *properties, const char *name)
{
    if (properties == NULL)
        return NULL;
    assert(properties->retainCount > 0);
    return DFHashTableLookup(properties->hashTable,name);
}

void CSSPut(CSSProperties *properties, const char *name, const char *value)
{
    if (properties == NULL)
        return;
    assert(properties->retainCount > 0);
    if (value == NULL)
        DFHashTableRemove(properties->hashTable,name);
    else
        DFHashTableAdd(properties->hashTable,name,(void *)value);
    if (!properties->dirty) // Minimise KVO notifications
        properties->dirty = 1;
    DFCallbackInvoke(properties->changeCallbacks,properties,NULL);
}

void CSSPropertiesUpdateFromRaw(CSSProperties *properties, DFHashTable *raw)
{
    DFHashTableRelease(properties->hashTable);
    properties->hashTable = DFHashTableCopy(raw);
    CSSExpandProperties(properties->hashTable);
    if (!properties->dirty) // Minimise KVO notifications
        properties->dirty = 1;
    DFCallbackInvoke(properties->changeCallbacks,properties,NULL);
}

int CSSGetBold(CSSProperties *properties)
{
    return DFStringEquals(CSSGet(properties,"font-weight"),"bold");
}

int CSSGetItalic(CSSProperties *properties)
{
    return DFStringEquals(CSSGet(properties,"font-style"),"italic");
}

int CSSGetUnderline(CSSProperties *properties)
{
    return (CSSGet(properties,"text-decoration-underline") != NULL);
}

int CSSGetLinethrough(CSSProperties *properties)
{
    return (CSSGet(properties,"text-decoration-line-through") != NULL);
}

int CSSGetOverline(CSSProperties *properties)
{
    return (CSSGet(properties,"text-decoration-overline") != NULL);
}

int CSSGetDefault(CSSProperties *properties)
{
    return DFStringEquals(CSSGet(properties,"-uxwrite-default"),"true");
}

void CSSSetBold(CSSProperties *properties, int value)
{
    CSSPut(properties,"font-weight",value ? "bold" : NULL);
}

void CSSSetItalic(CSSProperties *properties, int value)
{
    CSSPut(properties,"font-style",value ? "italic" : NULL);
}

void CSSSetUnderline(CSSProperties *properties, int value)
{
    CSSPut(properties,"text-decoration-underline",value ? "underline" : NULL);
}

void CSSSetLinethrough(CSSProperties *properties, int value)
{
    CSSPut(properties,"text-decoration-line-through",value ? "line-through" : NULL);
}

void CSSSetOverline(CSSProperties *properties, int value)
{
    CSSPut(properties,"text-decoration-overline",value ? "overline" : NULL);
}

void CSSSetDefault(CSSProperties *properties, int value)
{
    CSSPut(properties,"-uxwrite-default",value ? "true" : NULL);
}

void CSSPropertiesPrint(CSSProperties *properties, const char *indent)
{
    const char **allNames = CSSPropertiesCopyNames(properties);
    DFSortStringsCaseInsensitive(allNames);
    for (int nameIndex = 0; allNames[nameIndex]; nameIndex++) {
        const char *name = allNames[nameIndex];
        const char *value = CSSGet(properties,name);
        printf("%s%s = %s\n",indent,name,value);
    }
    free(allNames);
}

CSSProperties *CSSPropertiesNewWithExtra(CSSProperties *orig, const char *string)
{
    DFHashTable *extra = CSSParseProperties(string);
    CSSExpandProperties(extra);

    CSSProperties *result = (CSSProperties *)calloc(1,sizeof(CSSProperties));
    result->retainCount = 1;
    result->hashTable = DFHashTableNew((DFCopyFunction)strdup,free);
    const char **names = DFHashTableCopyKeys(orig->hashTable);
    for (int i = 0; names[i]; i++) {
        const char *value = DFHashTableLookup(orig->hashTable,names[i]);
        DFHashTableAdd(result->hashTable,names[i],(void *)value);
    }
    free(names);

    const char **keys = DFHashTableCopyKeys(extra);
    for (int i = 0; keys[i]; i++) {
        const char *key = keys[i];
        const char *value = DFHashTableLookup(extra,key);
        DFHashTableAdd(result->hashTable,key,value);
    }
    free(keys);

    DFHashTableRelease(extra);
    return result;
}

CSSProperties *CSSPropertiesNewWithRaw(DFHashTable *raw)
{
    CSSProperties *result = (CSSProperties *)calloc(1,sizeof(CSSProperties));
    result->retainCount = 1;

    result->hashTable = DFHashTableNew((DFCopyFunction)strdup,free);
    if (raw != NULL)
        CSSPropertiesUpdateFromRaw(result,raw);

    return result;
}

CSSProperties *CSSPropertiesNewWithString(const char *string)
{
    DFHashTable *raw = CSSParseProperties(string);
    CSSProperties *result = CSSPropertiesNewWithRaw(raw);
    DFHashTableRelease(raw);
    return result;
}

CSSProperties *CSSPropertiesNew(void)
{
    return CSSPropertiesNewWithRaw(NULL);
}

CSSProperties *CSSPropertiesRetain(CSSProperties *properties)
{
    if (properties != NULL)
        properties->retainCount++;
    return properties;
}

void CSSPropertiesRelease(CSSProperties *properties)
{
    assert((properties == NULL) || (properties->retainCount > 0));
    if ((properties == NULL) || (--properties->retainCount > 0))
        return;

    assert(properties->changeCallbacks == NULL);
    DFHashTableRelease(properties->hashTable);
    free(properties);
}
