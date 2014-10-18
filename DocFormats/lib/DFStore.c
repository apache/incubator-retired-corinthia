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
    return 0;
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
//                                             DFStore                                            //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

DFStore *DFStoreNewFilesystem(const char *rootPath)
{
    DFStore *store = (DFStore *)calloc(1,sizeof(DFStore));
    store->retainCount = 1;
    store->rootPath = strdup(rootPath);
    store->ops = &fsOps;
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

int DFStoreSave(DFStore *store, DFError **error)
{
    return store->ops->save(store,error);
}

int DFStoreRead(DFStore *store, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    return store->ops->read(store,path,buf,nbytes,error);
}

int DFStoreWrite(DFStore *store, const char *path, void *buf, size_t nbytes, DFError **error)
{
    return store->ops->write(store,path,buf,nbytes,error);
}

int DFStoreExists(DFStore *store, const char *path)
{
    return store->ops->exists(store,path);
}

int DFStoreDelete(DFStore *store, const char *path, DFError **error)
{
    return store->ops->delete(store,path,error);
}

const char **DFStoreList(DFStore *store, DFError **error)
{
    return store->ops->list(store,error);
}
