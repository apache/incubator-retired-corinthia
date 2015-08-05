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
#include <stddef.h>
#include <stdio.h>
#include "DFUnitTest.h"
#include "DFPlatform.h"



static void doZip(char *name) {
	DFextZipHandleP   zip;
	DFextZipDirEntryP zipDir;
	int               inp, out;
	unsigned char    *fileBuf[20];
	char              fileName[20][200];
	int               fileLen[20];
	char              tmpName[100];


	zip = DFextZipOpen(name);
	utassert((zip != NULL), "cannot open/read zip");

	for (inp = 0; inp < zip->zipFileCount; inp++) {
		strcpy(fileName[inp], zip->zipFileEntries[inp].fileName);
		fileLen[inp] = zip->zipFileEntries[inp].uncompressedSize;
		fileBuf[inp] = DFextZipReadFile(zip, &zip->zipFileEntries[inp]);
		utassert((fileBuf[inp] != NULL), "cannot read file in zip");
	}
	DFextZipClose(zip);
	zip = NULL;


	sprintf(tmpName, "new_%s", name);
	zip = DFextZipCreate(tmpName);
	utassert((zip != NULL), "cannot create zip");

	for (out = 0; out < inp; out++) {
		zipDir = DFextZipWriteFile(zip, fileName[out], fileBuf[out], fileLen[out]);
		utassert((zipDir != NULL), "cannot write file in zip");
		free(fileBuf[out]);
	}
	DFextZipClose(zip);
}



static void test_DFextZipOOXML(void)
{
	doZip("test.docx");
}



static void test_DFextZipODF(void)
{
	doZip("test.odt");
}



TestGroup PlatformWrapperTests = {
    "platform.wrapper", {
		    { "DFextZipOOXML", PlainTest, test_DFextZipOOXML },
			{ "DFextZipODF",   PlainTest, test_DFextZipODF },
			{ NULL,            PlainTest, NULL }
    }
};
