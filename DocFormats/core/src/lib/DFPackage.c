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

#include "DFPackage.h"
#include "DFCommon.h"
#include "DFString.h"
#include "DFFilesystem.h"
#include "DFBuffer.h"
#include "DFZipFile.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct DFPackageOps DFPackageOps;

struct DFPackageOps {
    int (*save)(DFPackage *package, DFError **error);
    int (*read)(DFPackage *package, const char *path, void **buf, size_t *nbytes, DFError **error);
    int (*write)(DFPackage *package, const char *path, void *buf, size_t nbytes, DFError **error);
    int (*exists)(DFPackage *package, const char *path);
    int (*delete)(DFPackage *package, const char *path, DFError **error);
    const char **(*list)(DFPackage *package, DFError **error);
};

struct DFPackage {
    size_t retainCount;
    DFFileFormat format;
    char *rootPath;
    char *zipFilename;
    DFHashTable *files;
    const DFPackageOps *ops;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                     DFPackage (Filesystem)                                     //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int fsSave(DFPackage *package, DFError **error)
{
    // Nothing to do here; we've already written everything to the filesystem
    // at the time the calls were made.
    return 1;
}

static int fsRead(DFPackage *package, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(package->rootPath,path);
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

static int fsWrite(DFPackage *package, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fullPath = DFAppendPathComponent(package->rootPath,path);
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

static int fsExists(DFPackage *package, const char *path)
{
    char *fullPath = DFAppendPathComponent(package->rootPath,path);
    int r = DFFileExists(fullPath);
    free(fullPath);
    return r;
}

static int fsDelete(DFPackage *package, const char *path, DFError **error)
{
    char *fullPath = DFAppendPathComponent(package->rootPath,path);
    int r = DFDeleteFile(fullPath,error);
    free(fullPath);
    return r;
}

const char **fsList(DFPackage *package, DFError **error)
{
    const char **allPaths = DFContentsOfDirectory(package->rootPath,1,error);
    if (allPaths == NULL)
        return NULL;;

    DFArray *filesOnly = DFArrayNew((DFCopyFunction)strdup,(DFFreeFunction)free);
    for (int i = 0; allPaths[i]; i++) {
        const char *relPath = allPaths[i];
        char *absPath = DFAppendPathComponent(package->rootPath,relPath);
        if (!DFIsDirectory(absPath))
            DFArrayAppend(filesOnly,(void *)relPath);
        free(absPath);
    }

    const char **result = DFStringArrayFlatten(filesOnly);
    DFArrayRelease(filesOnly);
    free(allPaths);
    return result;
}

static DFPackageOps fsOps = {
    .save = fsSave,
    .read = fsRead,
    .write = fsWrite,
    .exists = fsExists,
    .delete = fsDelete,
    .list = fsList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                       DFPackage (Memory)                                       //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static int memSave(DFPackage *package, DFError **error)
{
    // Nothing to do here; memory packages are intended to be temporary, and are never saved to disk
    return 1;
}

static int memRead(DFPackage *package, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    DFBuffer *buffer = DFHashTableLookup(package->files,path);
    if (buffer == NULL) {
        DFErrorSetPosix(error,ENOENT);
        return 0;
    }

    *buf = malloc(buffer->len);
    memcpy(*buf,buffer->data,buffer->len);
    *nbytes = buffer->len;

    return 1;
}

static int memWrite(DFPackage *package, const char *path, void *buf, size_t nbytes, DFError **error)
{
    DFBuffer *buffer = DFBufferNew();
    DFBufferAppendData(buffer,buf,nbytes);
    DFHashTableAdd(package->files,path,buffer);
    DFBufferRelease(buffer);
    return 1;
}

static int memExists(DFPackage *package, const char *path)
{
    return (DFHashTableLookup(package->files,path) != NULL);
}

static int memDelete(DFPackage *package, const char *path, DFError **error)
{
    DFHashTableRemove(package->files,path);
    return 1;
}

static const char **memList(DFPackage *package, DFError **error)
{
    return DFHashTableCopyKeys(package->files);
}

static DFPackageOps memOps = {
    .save = memSave,
    .read = memRead,
    .write = memWrite,
    .exists = memExists,
    .delete = memDelete,
    .list = memList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         DFPackage (Zip)                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Currently, zip packages operate just like memory packages, in that they store their contents in
// a hash table. The only difference is that they extract all the entries from a zip file on
// creation, and overwrite the zip file on save.
//
// Eventually, we should make it so the entries are read on-demand, and also that a new, temporary
// zip file is written if the package is modified, and that new version replaces the existing one on
// save.

static int zipSave(DFPackage *package, DFError **error)
{
    return DFZip(package->zipFilename,package,error);
}

static int zipRead(DFPackage *package, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    DFBuffer *buffer = DFHashTableLookup(package->files,path);
    if (buffer == NULL) {
        DFErrorSetPosix(error,ENOENT);
        return 0;
    }

    *buf = malloc(buffer->len);
    memcpy(*buf,buffer->data,buffer->len);
    *nbytes = buffer->len;

    return 1;
}

static int zipWrite(DFPackage *package, const char *path, void *buf, size_t nbytes, DFError **error)
{
    DFBuffer *buffer = DFBufferNew();
    DFBufferAppendData(buffer,buf,nbytes);
    DFHashTableAdd(package->files,path,buffer);
    DFBufferRelease(buffer);
    return 1;
}

static int zipExists(DFPackage *package, const char *path)
{
    return (DFHashTableLookup(package->files,path) != NULL);
}

static int zipDelete(DFPackage *package, const char *path, DFError **error)
{
    DFHashTableRemove(package->files,path);
    return 1;
}

static const char **zipList(DFPackage *package, DFError **error)
{
    return DFHashTableCopyKeys(package->files);
}

static DFPackageOps zipOps = {
    .save = zipSave,
    .read = zipRead,
    .write = zipWrite,
    .exists = zipExists,
    .delete = zipDelete,
    .list = zipList,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            DFPackage                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

// Normalize the path to remove multiple consecutive / characters, and to remove any trailing /
// character. This ensures that for implementations which rely on an exact match of the path
// (specifically, the memory package), that any non-essential differences in the supplied path will
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

static DFPackage *DFPackageNew(DFFileFormat format, const DFPackageOps *ops)
{
    DFPackage *package = (DFPackage *)calloc(1,sizeof(DFPackage));
    package->retainCount = 1;
    package->format = format;
    package->ops = ops;
    return package;
}

DFPackage *DFPackageNewFilesystem(const char *rootPath, DFFileFormat format)
{
    if ((rootPath == NULL) || (strlen(rootPath) == 0))
        rootPath = ".";;
    DFPackage *package = DFPackageNew(format,&fsOps);
    package->rootPath = strdup(rootPath);
    return package;
}

DFPackage *DFPackageNewMemory(DFFileFormat format)
{
    DFPackage *package = DFPackageNew(format,&memOps);
    package->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    return package;
}

DFPackage *DFPackageCreateZip(const char *filename, DFError **error)
{
    // Note that with the current implementation, the file doesn't actually get saved until we do a DFPackageSave.
    if (DFFileExists(filename)) {
        DFErrorFormat(error,"File already exists");
        return NULL;
    }

    DFPackage *package = DFPackageNew(DFFileFormatFromFilename(filename),&zipOps);
    package->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    package->zipFilename = strdup(filename);
    return package;
}

DFPackage *DFPackageOpenZip(const char *filename, DFError **error)
{
    if (!DFFileExists(filename)) {
        DFErrorFormat(error,"File does not exist");
        return NULL;
    }

    DFPackage *package = DFPackageNew(DFFileFormatFromFilename(filename),&zipOps);
    package->files = DFHashTableNew((DFCopyFunction)DFBufferRetain,(DFFreeFunction)DFBufferRelease);
    package->zipFilename = strdup(filename);

    if (!DFUnzip(filename,package,error)) {
        DFPackageRelease(package);
        return NULL;
    }
    return package;
}

DFPackage *DFPackageRetain(DFPackage *package)
{
    if (package != NULL)
        package->retainCount++;
    return package;
}

void DFPackageRelease(DFPackage *package)
{
    if ((package == NULL) || (--package->retainCount > 0))
        return;

    DFHashTableRelease(package->files);
    free(package->rootPath);
    free(package->zipFilename);
    free(package);
}

DFFileFormat DFPackageFormat(DFPackage *package)
{
    return package->format;
}

int DFPackageSave(DFPackage *package, DFError **error)
{
    return package->ops->save(package,error);
}

int DFPackageRead(DFPackage *package, const char *path, void **buf, size_t *nbytes, DFError **error)
{
    char *fixed = fixPath(path);
    int r = package->ops->read(package,fixed,buf,nbytes,error);
    free(fixed);
    return r;
}

int DFPackageWrite(DFPackage *package, const char *path, void *buf, size_t nbytes, DFError **error)
{
    char *fixed = fixPath(path);
    int r = package->ops->write(package,fixed,buf,nbytes,error);
    free(fixed);
    return r;
}

int DFPackageExists(DFPackage *package, const char *path)
{
    char *fixed = fixPath(path);
    int r = package->ops->exists(package,fixed);
    free(fixed);
    return r;
}

int DFPackageDelete(DFPackage *package, const char *path, DFError **error)
{
    char *fixed = fixPath(path);
    int r = package->ops->delete(package,fixed,error);
    free(fixed);
    return r;
}

const char **DFPackageList(DFPackage *package, DFError **error)
{
    return package->ops->list(package,error);
}
