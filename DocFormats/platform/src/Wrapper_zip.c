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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "DFPlatform.h"
#include "zlib.h"

#pragma pack(1)
typedef struct {
    uint32_t signature;                  // 0x06054b50
    uint16_t diskNumber;                 // NOT USED
    uint16_t centralDirectoryDiskNumber; // NOT USED
    uint16_t numEntriesThisDisk;         // NOT USED
    uint16_t numEntries;                 // Number of files in Zip
    uint32_t centralDirectorySize;       // NOT USED
    uint32_t centralDirectoryOffset;     // Offset of directory in file (fseek)
    uint16_t zipCommentLength;           // NOT USED
                                            // Followed by .ZIP file comment (variable size)
} ZipEndRecord;
#pragma pack()
static const uint32_t ZipEndRecord_signature = 0x06054B50;

#pragma pack(1)
typedef struct {
    uint32_t signature;                   // 0x02014B50
    uint16_t versionMadeBy;               // NOT USED
    uint16_t versionNeededToExtract;      // NOT USED
    uint16_t generalPurposeBitFlag;       // NOT USED
    uint16_t compressionMethod;           // NOT USED
    uint16_t lastModFileTime;             // NOT USED
    uint16_t lastModFileDate;             // NOT USED
    uint32_t crc32;                       // crc32
    uint32_t compressedSize;              // NOT USED
    uint32_t uncompressedSize;            // NOT USED
    uint16_t fileNameLength;              // Only used to skip to next record 
    uint16_t extraFieldLength;            // Only used to skip to next record
    uint16_t fileCommentLength;           // Only used to skip to next record
    uint16_t diskNumberStart;             // NOT USED
    uint16_t internalFileAttributes;      // NOT USED
    uint32_t externalFileAttributes;      // NOT USED
    uint32_t relativeOffsetOflocalHeader; // offset to file header
} ZipDirectoryRecord;
#pragma pack()
static const uint32_t ZipDirectoryRecord_signature = 0x02014B50;

#pragma pack(1)
typedef struct {
    uint32_t signature;              // 0x04034B50
    uint16_t versionNeededToExtract; // NOT USED
    uint16_t generalPurposeBitFlag;  // NOT USED
    uint16_t compressionMethod;      // flat file or zip compressed
    uint16_t lastModFileTime;        // NOT USED
    uint16_t lastModFileDate;        // NOT USED
    uint32_t crc32;                  // NOT USED
    uint32_t compressedSize;         // size to read from file
    uint32_t uncompressedSize;       // size in memory
    uint16_t fileNameLength;         // Length of file name
    uint16_t extraFieldLength;       // NOT USED
} ZipFileHeader;
#pragma pack()
static const uint32_t ZipFileHeader_signature = 0x04034B50;
static const int FILECOUNT_ALLOC_SIZE = 5;



static int readDirectory(FILE *zipFile, DFextZipHandleP zipHandle)
{
    unsigned long fileSize, readBytes;
    unsigned char workBuf[4096];
    int           i, x, zipOffset;


    //***** Read EndRecord *****
    // the EndRecord contains information, where the directory is located


    // find end of file, and calculate size
    if (fseek(zipFile, 0, SEEK_END)
        || (fileSize = ftell(zipFile)) <= sizeof(ZipEndRecord))
        return -1;

    // Read size of workBuf of filesize from end of file to locate EndRecord
    readBytes = (fileSize < sizeof(workBuf)) ? fileSize : sizeof(workBuf);
    if (fseek(zipFile, fileSize - readBytes, SEEK_SET)
        || fread(workBuf, 1, readBytes, zipFile) < readBytes)
        return -1;

    // search for EndRecord signature
    for (i = readBytes - sizeof(ZipEndRecord); i >= 0; i--) {
        ZipEndRecord *recEnd = (ZipEndRecord *)(workBuf + i);

        // check if we have a signature
        if (recEnd->signature == ZipEndRecord_signature) {
            // update zipHandle
            zipOffset                 = recEnd->centralDirectoryOffset;
            zipHandle->zipFileCount   = recEnd->numEntries;
            zipHandle->zipFileEntries = xmalloc(zipHandle->zipFileCount * sizeof(DFextZipDirEntry));
            break;
        }
    }
    if (i < 0)
        return -1;


    //***** Read Directory *****
    // Each file has a global and a local entry, read both in a loop


    // Find firs directory entry
    if (fseek(zipFile, zipOffset, SEEK_SET))
        return -1;

    // loop through all entries


    for (i = 0; i < zipHandle->zipFileCount; i++) {
        ZipDirectoryRecord *recDir   = (ZipDirectoryRecord *)workBuf;
        DFextZipDirEntry   *dirEntry = &zipHandle->zipFileEntries[i];

        // Find next directory entry, read it and verify signature
         if (fread(workBuf, 1, sizeof(ZipDirectoryRecord), zipFile) < sizeof(ZipDirectoryRecord)
            || recDir->signature != ZipDirectoryRecord_signature)
            return -1;

        dirEntry->compressedSize    = recDir->compressedSize;
        dirEntry->uncompressedSize  = recDir->uncompressedSize;
        dirEntry->compressionMethod = recDir->compressionMethod;
        dirEntry->offset            = recDir->relativeOffsetOflocalHeader;
        dirEntry->crc32             = recDir->crc32;

        // Add filename
        dirEntry->fileName = xmalloc(recDir->fileNameLength + 1);
        if (fread(dirEntry->fileName, 1, recDir->fileNameLength, zipFile) < (unsigned long)recDir->fileNameLength)
            return -1;
        dirEntry->fileName[recDir->fileNameLength] = '\0';

        // Skip extra info and store pointer at next entry
        x = recDir->extraFieldLength + recDir->fileCommentLength;
        if (x && fseek(zipFile, x, SEEK_CUR))
            return -1;
    };

    return 0;
}



static void releaseMemory(DFextZipHandleP zipHandle) {
    if (zipHandle) {
        int count = zipHandle->zipCreateMode ? zipHandle->zipCreateMode : zipHandle->zipFileCount;
        if (count) {
            for (int i = 0; i < count; i++) {
                DFextZipDirEntry *zipDirEntry = &zipHandle->zipFileEntries[i];
                if (zipDirEntry->fileName != NULL)
                    free(zipDirEntry->fileName);
            }
            free(zipHandle->zipFileEntries);
        }
        free(zipHandle);
    }
}



static void writeGlobalDirAndEndRecord(DFextZipHandleP zipHandle) {
    static const char comment[] = "generated by Corinthia";
    ZipDirectoryRecord dirRecord;
    ZipEndRecord       endRecord;
    int                i;


    // Prepare constant part of records
    endRecord.signature              = ZipEndRecord_signature;
    endRecord.diskNumber             = endRecord.centralDirectoryDiskNumber = endRecord.numEntriesThisDisk = 0;
    endRecord.numEntries             = zipHandle->zipFileCount;
    endRecord.centralDirectoryOffset = ftell(zipHandle->zipFile);
    endRecord.zipCommentLength       = strlen(comment);

    dirRecord.signature              = ZipDirectoryRecord_signature;
    dirRecord.versionMadeBy          = 0x2D;
    dirRecord.versionNeededToExtract = 0x14;
    dirRecord.lastModFileTime        = dirRecord.lastModFileDate   =
    dirRecord.crc32                  = dirRecord.extraFieldLength       = dirRecord.fileCommentLength =
    dirRecord.diskNumberStart        = dirRecord.internalFileAttributes = 0;
    dirRecord.generalPurposeBitFlag  = 0x0006;
    dirRecord.externalFileAttributes = 0;

    // loop through all directory entries, write to disk while collecting size
    endRecord.centralDirectorySize = 0;
    for (i = 0; i < zipHandle->zipFileCount; i++) {
        dirRecord.compressionMethod           = zipHandle->zipFileEntries[i].compressionMethod;
        dirRecord.compressedSize              = zipHandle->zipFileEntries[i].compressedSize;
        dirRecord.uncompressedSize            = zipHandle->zipFileEntries[i].uncompressedSize;
        dirRecord.fileNameLength              = strlen(zipHandle->zipFileEntries[i].fileName);
        dirRecord.relativeOffsetOflocalHeader = zipHandle->zipFileEntries[i].offset;
        dirRecord.crc32                       = zipHandle->zipFileEntries[i].crc32;
        endRecord.centralDirectorySize       += sizeof(ZipDirectoryRecord) + dirRecord.fileNameLength;
        fwrite(&dirRecord, 1, sizeof(ZipDirectoryRecord), zipHandle->zipFile);
        fwrite(zipHandle->zipFileEntries[i].fileName, 1, dirRecord.fileNameLength, zipHandle->zipFile);
    }

    // and finally the end record
    fwrite(&endRecord, 1, sizeof(ZipEndRecord), zipHandle->zipFile);
    fwrite(comment,    1, sizeof(comment),      zipHandle->zipFile);
}



DFextZipHandleP DFextZipOpen(const char *zipFilename) {
    DFextZipHandleP zipHandle = xmalloc(sizeof(DFextZipHandle));

    // open zip file for reading
    zipHandle->zipCreateMode = zipHandle->zipFileCount = 0;
    zipHandle->zipFile       = fopen(zipFilename, "rb");
    if (zipHandle->zipFile
        && !readDirectory(zipHandle->zipFile, zipHandle))
        return zipHandle;

    // release memory
    releaseMemory(zipHandle);
    return NULL;
}


unsigned char *DFextZipReadFile(DFextZipHandleP zipHandle, DFextZipDirEntryP zipEntry) {
    unsigned char *fileBuf = xmalloc(zipEntry->uncompressedSize);
    ZipFileHeader  recFile;
    z_stream       strm;


    // Position in front of file
    if (fseek(zipHandle->zipFile, zipEntry->offset, SEEK_SET)
        || fread(&recFile, 1, sizeof(ZipFileHeader), zipHandle->zipFile) < sizeof(ZipFileHeader)
        || recFile.signature != ZipFileHeader_signature
        || fseek(zipHandle->zipFile, recFile.extraFieldLength + recFile.fileNameLength, SEEK_CUR)) {
        free(fileBuf);
        return NULL;
    }

    // interesting a zip file that is uncompressed, have to handle that
    if (zipEntry->compressionMethod != Z_DEFLATED) {
        if (fread(fileBuf, 1, zipEntry->uncompressedSize, zipHandle->zipFile) < (unsigned long)zipEntry->uncompressedSize
            || ferror(zipHandle->zipFile)) {
            free(fileBuf);
            fileBuf = NULL;
        }
        return fileBuf;
    }


    //***** Handle zlib inflate *****


    strm.zalloc = Z_NULL;
    strm.zfree = strm.opaque = strm.next_in = Z_NULL;
    strm.avail_in = 0;

    // Use inflateInit2 with negative window bits to indicate raw data
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        free(fileBuf);
        return NULL;
    }

    // Read compressed data
    unsigned char *comprBuf = xmalloc(zipEntry->compressedSize);
    if (fread(comprBuf, 1, zipEntry->compressedSize, zipHandle->zipFile) < (unsigned long)zipEntry->compressedSize
        || ferror(zipHandle->zipFile)) {
        free(fileBuf);
        free(comprBuf);
        return NULL;
    }

    // and inflate data
    strm.avail_in  = zipEntry->compressedSize;
    strm.next_in   = comprBuf;
    strm.avail_out = zipEntry->uncompressedSize;
    strm.next_out  = fileBuf;
    if (inflate(&strm, Z_NO_FLUSH) == Z_STREAM_ERROR) {
        free(fileBuf);
        free(comprBuf);
        return NULL;
    }

    free(comprBuf);
    inflateEnd(&strm);
    return fileBuf;
}



DFextZipHandleP DFextZipCreate(const char *zipFilename) {
    DFextZipHandleP zipHandle = xmalloc(sizeof(DFextZipHandle));
    int             memSize;
    // Open file for write
    if ((zipHandle->zipFile = fopen(zipFilename, "wb")) == NULL) {
        free(zipHandle);
        return NULL;
    }

    // prepare to add files
    zipHandle->zipFileCount   = 0;
    zipHandle->zipCreateMode  = FILECOUNT_ALLOC_SIZE;
    memSize                   = zipHandle->zipCreateMode * sizeof(DFextZipDirEntry);

    zipHandle->zipFileEntries = xmalloc(memSize);
    bzero(zipHandle->zipFileEntries, memSize);
    return zipHandle;
}



DFextZipDirEntryP DFextZipWriteFile(DFextZipHandleP zipHandle, const char *fileName, const void *buf, const int len) {
    z_stream       strm;
    unsigned char *outbuf;
    ZipFileHeader  header;
    int            fileNameLength = strlen(fileName);

    // do we have space for one more entry ?
    if (zipHandle->zipFileCount >= zipHandle->zipCreateMode) {
        zipHandle->zipCreateMode += FILECOUNT_ALLOC_SIZE;
        zipHandle->zipFileEntries = xrealloc(zipHandle->zipFileEntries, zipHandle->zipCreateMode * sizeof(DFextZipDirEntry));
        bzero(&zipHandle->zipFileEntries[zipHandle->zipFileCount], FILECOUNT_ALLOC_SIZE * sizeof(DFextZipDirEntry));
    }

    // prepare local and global file entry
    DFextZipDirEntryP entryPtr  = &zipHandle->zipFileEntries[zipHandle->zipFileCount++];
    entryPtr->offset            = ftell(zipHandle->zipFile);
    entryPtr->uncompressedSize  = len;
    entryPtr->fileName          = xmalloc(fileNameLength + 1);
    entryPtr->compressionMethod = Z_DEFLATED;
    entryPtr->crc32             = crc32(0L, Z_NULL, 0);
    entryPtr->crc32             = crc32(entryPtr->crc32, buf, len);

    strcpy(entryPtr->fileName, fileName);

    // prepare to deflate
    strm.zalloc = Z_NULL;
    strm.zfree  = strm.opaque = Z_NULL;
    if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
        return NULL;

    // deflate buffer
    strm.next_in   = buf;
    strm.avail_in  = len;
    strm.avail_out = deflateBound(&strm, len);
    strm.next_out  = (Bytef *)xmalloc(strm.avail_out);
    outbuf         = strm.next_out;
    if (deflate(&strm, Z_FINISH) != Z_STREAM_END) {
        free(outbuf);
        return NULL;
    }
    deflateEnd(&strm);
    entryPtr->compressedSize = strm.total_out;

    // prepare local header
    header.versionNeededToExtract = 0x0014;
    header.generalPurposeBitFlag  = 0x0006;
    header.lastModFileTime        = header.lastModFileDate = header.extraFieldLength = header.crc32 = 0;
    header.signature              = ZipFileHeader_signature;
    header.compressionMethod      = entryPtr->compressionMethod;
    header.compressedSize         = entryPtr->compressedSize;
    header.uncompressedSize       = entryPtr->uncompressedSize;
    header.fileNameLength         = fileNameLength;
    header.crc32                  = entryPtr->crc32;

    // put data to file
    fwrite(&header,            1, sizeof(header),        zipHandle->zipFile);
    fwrite(entryPtr->fileName, 1, fileNameLength,        zipHandle->zipFile);
    fwrite(outbuf,             1, header.compressedSize, zipHandle->zipFile); // skip CMD bytes in front

    // cleanup
    free(outbuf);
    return entryPtr;
}



void DFextZipClose(DFextZipHandleP zipHandle)
{
    if (zipHandle->zipCreateMode)
        writeGlobalDirAndEndRecord(zipHandle);

    fclose(zipHandle->zipFile);
    releaseMemory(zipHandle);
}
