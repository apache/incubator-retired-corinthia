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
#include <DocFormats/DFStorage.h>
#include "DFCommon.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFBuffer.h"
#include "DFZipFile.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DFStorageOps DFStorageOps;

struct DFStorageOps {
    int (*save)(DFStorage *storage, DFError **error);
    int (*read)(DFStorage *storage, const char *path, void **buf, size_t *nbytes, DFError **error);
    int (*write)(DFStorage *storage, const char *path, void *buf, size_t nbytes, DFError **error);
    int (*exists)(DFStorage *storage, const char *path);
    int (*delete)(DFStorage *storage, const char *path, DFError **error);
    const char **(*list)(DFStorage *storage, DFError **error);
};

struct DFStorage {
    size_t retainCount;
    DFFileFormat format;
    char *rootPath;
    char *zipFilename;
    DFHashTable *files;
    const DFStorageOps *ops;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                     DFStorage (Filesystem)                                     //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int fsSave(DFStorage *storage, DFError **error)
{
    // Nothing to do here; we've already written everything to the filesystem
    // at the time the calls were made.
    return 1;
}

static int fsRead(DFStorage *storage, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(storage->rootPath,path);
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

static int fsWrite(DFStorage *storage, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(storage->rootPath,path);
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

static int fsExists(DFStorage *storage, const char *path)
{
    char *fullPath = DFAppendPathComponent(storage->rootPath,path);
    int r = DFFileExists(fullPath);
    free(fullPath);
    return r;
}

static int fsDelete(DFStorage *storage, const char *path, DFError **error)
{
    char *fullPath = DFAppendPathComponent(storage->rootPath,path);
    int r = DFDeleteFile(fullPath,error);
    free(fullPath);
    return r;
}

const char **fsList(DFStorage *storage, DFError **error)
{
    const char **allPaths = DFContentsOfDirectory(storage->rootPath,1,error);
    if (allPaths == NULL)
        return NULL;;

    DFArray *filesOnly = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (int i = 0; allPaths[i]; i++) {
        const char *relPath = allPaths[i];
        char *absPath = DFAppendPathComponent(storage->rootPath,relPath);
        if (!DFIsDirectory(absPath))
            DFArrayAppend(filesOnly,(void *)relPath);
        free(absPath);
    }

    const char **result = DFStringArrayFlatten(filesOnly);
    DFArrayRelease(filesOnly);
    free(allPaths);
    return result;
}

static DFStorageOps fsOps = {
    .save = fsSave,
    .read = fsRead,
    .write = fsWrite,
    .exists = fsExists,
    .delete = fsDelete,
    .list = fsList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       DFStorage (Memory)                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int memSave(DFStorage *storage, DFError **error)
{
    // Nothing to do here; memory-backed storage objects are intended to be temporary, and are never
    // saved to disk
    return 1;
}

static int memRead(DFStorage *storage, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    DFBuffer *buffer = DFHashTableLookup(storage->files,path);
    if (buffer == NULL) {
        DFErrorSetPosix(error,ENOENT);
        return 0;
    }

    *buf = malloc(buffer->len);
    memcpy(*buf,buffer->data,buffer->len);
    *nbytes = buffer->len;

    return 1;
}

static int memWrite(DFStorage *storage, const char *path, void *buf, size_t nbytes, DFError **error)
{
    DFBuffer *buffer = DFBufferNew();
    DFBufferAppendData(buffer,buf,nbytes);
    DFHashTableAdd(storage->files,path,buffer);
    DFBufferRelease(buffer);
    return 1;
}

static int memExists(DFStorage *storage, const char *path)
{
    return (DFHashTableLookup(storage->files,path) != NULL);
}

static int memDelete(DFStorage *storage, const char *path, DFError **error)
{
    DFHashTableRemove(storage->files,path);
    return 1;
}

static const char **memList(DFStorage *storage, DFError **error)
{
    return DFHashTableCopyKeys(storage->files);
}

static DFStorageOps memOps = {
    .save = memSave,
    .read = memRead,
    .write = memWrite,
    .exists = memExists,
    .delete = memDelete,
    .list = memList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         DFStorage (Zip)                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Currently, zip storage objects operate just like memory storage objects, in that they store their
// contents in a hash table. The only difference is that they extract all the entries from a zip
// file on creation, and overwrite the zip file on save.
//
// Eventually, we should make it so the entries are read on-demand, and also that a new, temporary
// zip file is written if the storage is modified, and that new version replaces the existing one on
// save.

static int zipSave(DFStorage *storage, DFError **error)
{
    return DFZip(storage->zipFilename,storage,error);
}

static int zipRead(DFStorage *storage, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    DFBuffer *buffer = DFHashTableLookup(storage->files,path);
    if (buffer == NULL) {
        DFErrorSetPosix(error,ENOENT);
        return 0;
    }

    *buf = malloc(buffer->len);
    memcpy(*buf,buffer->data,buffer->len);
    *nbytes = buffer->len;

    return 1;
}

static int zipWrite(DFStorage *storage, const char *path, void *buf, size_t nbytes, DFError **error)
{
    DFBuffer *buffer = DFBufferNew();
    DFBufferAppendData(buffer,buf,nbytes);
    DFHashTableAdd(storage->files,path,buffer);
    DFBufferRelease(buffer);
    return 1;
}

static int zipExists(DFStorage *storage, const char *path)
{
    return (DFHashTableLookup(storage->files,path) != NULL);
}

static int zipDelete(DFStorage *storage, const char *path, DFError **error)
{
    DFHashTableRemove(storage->files,path);
    return 1;
}

static const char **zipList(DFStorage *storage, DFError **error)
{
    return DFHashTableCopyKeys(storage->files);
}

static DFStorageOps zipOps = {
    .save = zipSave,
    .read = zipRead,
    .write = zipWrite,
    .exists = zipExists,
    .delete = zipDelete,
    .list = zipList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFStorage                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Normalize the path to remove multiple consecutive / characters, and to remove any trailing /
// character. This ensures that for implementations which rely on an exact match of the path
// (specifically, the memory storage object), that any non-essential differences in the supplied
// path will not matter.

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

static DFStorage *DFStorageNew(DFFileFormat format, const DFStorageOps *ops)
{
    DFStorage *storage = (DFStorage *)calloc(1,sizeof(DFStorage));
    storage->retainCount = 1;
    storage->format = format;
    storage->ops = ops;
    return storage;
}

DFStorage *DFStorageNewFilesystem(const char *rootPath, DFFileFormat format)
{
    if ((rootPath == NULL) || (strlen(rootPath) == 0))
        rootPath = ".";;
    DFStorage *storage = DFStorageNew(format,&fsOps);
    storage->rootPath = strdup(rootPath);
    return storage;
}

DFStorage *DFStorageNewMemory(DFFileFormat format)
{
    DFStorage *storage = DFStorageNew(format,&memOps);
    storage->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    return storage;
}

DFStorage *DFStorageCreateZip(const char *filename, DFError **error)
{
    // Note that with the current implementation, the file doesn't actually get saved until we do a DFStorageSave.
    if (DFFileExists(filename)) {
        DFErrorFormat(error,"File already exists");
        return NULL;
    }

    DFStorage *storage = DFStorageNew(DFFileFormatFromFilename(filename),&zipOps);
    storage->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    storage->zipFilename = strdup(filename);
    return storage;
}

DFStorage *DFStorageOpenZip(const char *filename, DFError **error)
{
    if (!DFFileExists(filename)) {
        DFErrorFormat(error,"File does not exist");
        return NULL;
    }

    DFStorage *storage = DFStorageNew(DFFileFormatFromFilename(filename),&zipOps);
    storage->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    storage->zipFilename = strdup(filename);

    if (!DFUnzip(filename,storage,error)) {
        DFStorageRelease(storage);
        return NULL;
    }
    return storage;
}

DFStorage *DFStorageRetain(DFStorage *storage)
{
    if (storage != NULL)
        storage->retainCount++;
    return storage;
}

void DFStorageRelease(DFStorage *storage)
{
    if ((storage == NULL) || (--storage->retainCount > 0))
        return;

    DFHashTableRelease(storage->files);
    free(storage->rootPath);
    free(storage->zipFilename);
    free(storage);
}

DFFileFormat DFStorageFormat(DFStorage *storage)
{
    return storage->format;
}

int DFStorageSave(DFStorage *storage, DFError **error)
{
    return storage->ops->save(storage,error);
}

int DFStorageRead(DFStorage *storage, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    char *fixed = fixPath(path);
    int r = storage->ops->read(storage,fixed,buf,nbytes,error);
    free(fixed);
    return r;
}

int DFStorageWrite(DFStorage *storage, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fixed = fixPath(path);
    int r = storage->ops->write(storage,fixed,buf,nbytes,error);
    free(fixed);
    return r;
}

int DFStorageExists(DFStorage *storage, const char *path)
{
    char *fixed = fixPath(path);
    int r = storage->ops->exists(storage,fixed);
    free(fixed);
    return r;
}

int DFStorageDelete(DFStorage *storage, const char *path, DFError **error)
{
    char *fixed = fixPath(path);
    int r = storage->ops->delete(storage,fixed,error);
    free(fixed);
    return r;
}

const char **DFStorageList(DFStorage *storage, DFError **error)
{
    return storage->ops->list(storage,error);
}
