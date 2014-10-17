// Copyright 2012-2014 UX Productivity Pty Ltd
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "DFStore.h"
#include "DFCommon.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFBuffer.h"

struct DFStore {
    size_t retainCount;
    char *rootPath;
};

DFStore *DFStoreNewFilesystem(const char *rootPath)
{
    DFStore *store = (DFStore *)calloc(1,sizeof(DFStore));
    store->retainCount = 1;
    store->rootPath = strdup(rootPath);
    return store;
}

DFStore *DFStoreRetain(DFStore *store)
{
    if (store != NULL)
        store->retainCount++;
    return store;
}

void DFStoreRelease(DFStore *store)
{
    if ((store == NULL) || (--store->retainCount > 0))
        return;

    free(store->rootPath);
    free(store);
}

void DFStoreSave(DFStore *store)
{
}

int DFStoreRead(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int ok = 0;

    FILE *file = fopen(fullPath,"rb");
    if (file == NULL) {
        DFErrorSetPosix(error,errno);
        goto end;
    }

    size_t balloc = 4096;
    size_t blen = 0;
    char *mem = (char *)malloc(balloc);

    size_t r;
    while (0 < (r = fread(&mem[blen],1,4096,file))) {
        balloc += r;
        blen += r;
        mem = (char *)realloc(mem,balloc);
    }
    ok = 1;

    *buf = mem;
    *nbytes = blen;

end:
    if (file != NULL)
        fclose(file);
    free(fullPath);
    return ok;
}

int DFStoreWrite(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = 0;

    FILE *file = fopen(fullPath,"wb");
    if (file == NULL) {
        DFErrorSetPosix(error,errno);
        goto end;
    }
    size_t w = fwrite(buf,1,nbytes,file);
    if (w != nbytes) {
        DFErrorFormat(error,"Incomplete write");
        goto end;
    }
    r = 1;

end:
    if (file != NULL)
        fclose(file);
    free(fullPath);
    return r;
}

int DFStoreExists(DFStore *store, const char *path)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = DFFileExists(fullPath);
    free(fullPath);
    return r;
}

int DFStoreIsDir(DFStore *store, const char *path)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = DFIsDirectory(fullPath);
    free(fullPath);
    return r;
}

int DFStoreMkDir(DFStore *store, const char *path, int intermediates, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = DFCreateDirectory(fullPath,intermediates,error);
    free(fullPath);
    return r;
}

int DFStoreDelete(DFStore *store, const char *path, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = DFDeleteFile(fullPath,error);
    free(fullPath);
    return r;
}

const char **DFStoreList(DFStore *store, const char *path, int recursive, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    const char **r = DFContentsOfDirectory(fullPath,recursive,error);
    free(fullPath);
    return r;
}
