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

typedef struct DFStoreOps DFStoreOps;

struct DFStoreOps {
    int (*save)(DFStore *store, DFError **error);
    int (*read)(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error);
    int (*write)(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error);
    int (*exists)(DFStore *store, const char *path);
    int (*delete)(DFStore *store, const char *path, DFError **error);
    const char **(*list)(DFStore *store, DFError **error);
};

struct DFStore {
    size_t retainCount;
    char *rootPath;
    DFHashTable *files;
    const DFStoreOps *ops;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                      DFStore (Filesystem)                                      //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int fsSave(DFStore *store, DFError **error)
{
    // Nothing to do here; we've already written everything to the filesystem
    // at the time the calls were made.
    return 1;
}

static int fsRead(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error)
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

static int fsWrite(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    char *parentPath = DFPathDirName(fullPath);
    int r = 0;
    FILE *file = NULL;

    if (!DFFileExists(parentPath) && !DFCreateDirectory(parentPath,1,error))
        goto end;

    file = fopen(fullPath,"wb");
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
    free(parentPath);
    free(fullPath);
    return r;
}

static int fsExists(DFStore *store, const char *path)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = DFFileExists(fullPath);
    free(fullPath);
    return r;
}

static int fsDelete(DFStore *store, const char *path, DFError **error)
{
    char *fullPath = DFAppendPathComponent(store->rootPath,path);
    int r = DFDeleteFile(fullPath,error);
    free(fullPath);
    return r;
}

const char **fsList(DFStore *store, DFError **error)
{
    const char **allPaths = DFContentsOfDirectory(store->rootPath,1,error);
    if (allPaths == NULL)
        return NULL;;

    DFArray *filesOnly = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (int i = 0; allPaths[i]; i++) {
        const char *relPath = allPaths[i];
        char *absPath = DFAppendPathComponent(store->rootPath,relPath);
        if (!DFIsDirectory(absPath))
            DFArrayAppend(filesOnly,(void *)relPath);
        free(absPath);
    }

    const char **result = DFStringArrayFlatten(filesOnly);
    DFArrayRelease(filesOnly);
    free(allPaths);
    return result;
}

static DFStoreOps fsOps = {
    .save = fsSave,
    .read = fsRead,
    .write = fsWrite,
    .exists = fsExists,
    .delete = fsDelete,
    .list = fsList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                        DFStore (Memory)                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int memSave(DFStore *store, DFError **error)
{
    // Nothing to do here; memory stores are intended to be temporary, and are never saved to disk
    return 1;
}

static int memRead(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    DFBuffer *buffer = DFHashTableLookup(store->files,path);
    if (buffer == NULL) {
        DFErrorSetPosix(error,ENOENT);
        return 0;
    }

    *buf = malloc(buffer->len);
    memcpy(*buf,buffer->data,buffer->len);
    *nbytes = buffer->len;

    return 1;
}

static int memWrite(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error)
{
    DFBuffer *buffer = DFBufferNew();
    DFBufferAppendData(buffer,buf,nbytes);
    DFHashTableAdd(store->files,path,buffer);
    DFBufferRelease(buffer);
    return 1;
}

static int memExists(DFStore *store, const char *path)
{
    return (DFHashTableLookup(store->files,path) != NULL);
}

static int memDelete(DFStore *store, const char *path, DFError **error)
{
    DFHashTableRemove(store->files,path);
    return 1;
}

static const char **memList(DFStore *store, DFError **error)
{
    return DFHashTableCopyKeys(store->files);
}

static DFStoreOps memOps = {
    .save = memSave,
    .read = memRead,
    .write = memWrite,
    .exists = memExists,
    .delete = memDelete,
    .list = memList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                             DFStore                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Normalize the path to remove multiple consecutive / characters, and to remove any trailing /
// character. This ensures that for implementations which rely on an exact match of the path
// (specifically, the memory store), that any non-essential differences in the supplied path will
// not matter.

static char *fixPath(const char *input)
{
    char *normalized = DFPathNormalize(input);
    char *result;
    if (normalized[0] == '/')
        result = strdup(&normalized[1]);
    else
        result = strdup(normalized);
    free(normalized);
    return result;
}

DFStore *DFStoreNewFilesystem(const char *rootPath)
{
    DFStore *store = (DFStore *)calloc(1,sizeof(DFStore));
    store->retainCount = 1;
    store->rootPath = strdup(rootPath);
    store->ops = &fsOps;
    return store;
}

DFStore *DFStoreNewMemory(void)
{
    DFStore *store = (DFStore *)calloc(1,sizeof(DFStore));
    store->retainCount = 1;
    store->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    store->ops = &memOps;
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

    DFHashTableRelease(store->files);
    free(store->rootPath);
    free(store);
}

int DFStoreSave(DFStore *store, DFError **error)
{
    return store->ops->save(store,error);
}

int DFStoreRead(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    char *fixed = fixPath(path);
    int r = store->ops->read(store,fixed,buf,nbytes,error);
    free(fixed);
    return r;
}

int DFStoreWrite(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fixed = fixPath(path);
    int r = store->ops->write(store,fixed,buf,nbytes,error);
    free(fixed);
    return r;
}

int DFStoreExists(DFStore *store, const char *path)
{
    char *fixed = fixPath(path);
    int r = store->ops->exists(store,fixed);
    free(fixed);
    return r;
}

int DFStoreDelete(DFStore *store, const char *path, DFError **error)
{
    char *fixed = fixPath(path);
    int r = store->ops->delete(store,fixed,error);
    free(fixed);
    return r;
}

const char **DFStoreList(DFStore *store, DFError **error)
{
    return store->ops->list(store,error);
}
