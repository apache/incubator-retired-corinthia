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

#include "DFPlatform.h"
#include "TextPackage.h"
#include "DFBuffer.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFCommon.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           TextPackage                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static TextPackage *TextPackageNew(void)
{
    TextPackage *package = (TextPackage *)calloc(1,sizeof(TextPackage));
    package->retainCount = 1;
    package->items = DFHashTableNew((DFCopyFunction)strdup,free);
    package->keys = (char **)calloc(1,sizeof(char *));
    return package;
}

TextPackage *TextPackageRetain(TextPackage *package)
{
    if (package != NULL)
        package->retainCount++;
    return package;
}

void TextPackageRelease(TextPackage *package)
{
    if ((package == NULL) || (--package->retainCount > 0))
        return;

    for (size_t i = 0; i < package->nkeys; i++)
        free(package->keys[i]);
    free(package->keys);
    DFHashTableRelease(package->items);
    free(package);
}

static int processIncludes(TextPackage *package, const char *input, DFBuffer *output, const char *path, DFError **error)
{
    int ok = 1;
    const char **lines = DFStringSplit(input,"\n",0);
    for (int lineno = 0; lines[lineno] && ok; lineno++) {
        const char *line = lines[lineno];
        if (DFStringHasPrefix(line,"#include \"") && DFStringHasSuffix(line,"\"")) {
            char *inclRelPath = DFSubstring(line,10,strlen(line)-1);
            char *inclAbsPath = DFAppendPathComponent(path,inclRelPath);
            char *inclDirName = DFPathDirName(inclAbsPath);
            char *inclContent = DFStringReadFromFile(inclAbsPath,error);
            if (inclContent == NULL) {
                DFErrorFormat(error,"%s: %s",inclRelPath,DFErrorMessage(error));
                ok = 0;
            }
            else if (!processIncludes(package,inclContent,output,inclDirName,error)) {
                ok = 0;
            }
            free(inclRelPath);
            free(inclAbsPath);
            free(inclDirName);
            free(inclContent);
        }
        else {
            DFBufferFormat(output,"%s\n",line);
        }
    }
    free(lines);
    return ok;
}

static int parsePackage(TextPackage *package, const char *string, const char *path, DFError **error)
{
    DFBuffer *replaced = DFBufferNew();
    if (!strcmp(path,""))
        path = ".";

    if (!processIncludes(package,string,replaced,path,error)) {
        DFBufferRelease(replaced);
        return 0;
    }


    char *currentKey = strdup("");
    DFBuffer *currentValue = DFBufferNew();
    const char **lines = DFStringSplit(replaced->data,"\n",0);
    for (int lineno = 0; lines[lineno]; lineno++) {
        const char *line = lines[lineno];

        if (!DFStringHasPrefix(line,"#")) {
            DFBufferFormat(currentValue,"%s\n",line);
        }
        else if (DFStringHasPrefix(line,"#item ")) {
            package->keys = (char **)realloc(package->keys,(package->nkeys+2)*sizeof(char *));
            package->keys[package->nkeys++] = strdup(currentKey);
            package->keys[package->nkeys] = NULL;
            DFHashTableAdd(package->items,currentKey,currentValue->data);
            free(currentKey);
            DFBufferRelease(currentValue);
            currentKey = DFSubstring(line,6,strlen(line));
            currentValue = DFBufferNew();
        }
        else if (DFStringHasPrefix(line,"##")) {
            DFBufferFormat(currentValue,"%s\n",&line[1]);
        }
        else {
            DFErrorFormat(error,"Unknown command: %s on line %d",line,(lineno+1));
            return 0;
        }
    }
    package->keys = (char **)realloc(package->keys,(package->nkeys+2)*sizeof(char *));
    package->keys[package->nkeys++] = strdup(currentKey);
    package->keys[package->nkeys] = NULL;
    DFHashTableAdd(package->items,currentKey,currentValue->data);

    free(lines);
    free(currentKey);
    DFBufferRelease(currentValue);
    DFBufferRelease(replaced);
    return 1;
}

TextPackage *TextPackageNewWithFile(const char *filename, DFError **error)
{
    char *contents = DFStringReadFromFile(filename,error);
    if (contents == NULL) {
        DFErrorFormat(error,"%s: %s",filename,DFErrorMessage(error));
        return NULL;
    }

    char *path = DFPathDirName(filename);
    TextPackage *result = TextPackageNewWithString(contents,path,error);
    free(path);
    free(contents);
    return result;
}

TextPackage *TextPackageNewWithString(const char *string, const char *path, DFError **error)
{
    TextPackage *package = TextPackageNew();
    if (!parsePackage(package,string,path,error)) {
        TextPackageRelease(package);
        return NULL;
    }
    else
        return package;
}
