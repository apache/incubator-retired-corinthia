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
#include <DocFormats/DocFormats.h>
#include <stdio.h>
#include <string.h>

void usage(void)
{
    printf("Usage:\n"
           "\n"
           "dfconvert get concrete.docx abstract.html\n"
           "\n"
           "    Create a new HTML file from a Word document. The HTML file must not\n"
           "    already exist.\n"
           "\n"
           "dfconvert put concrete.docx abstract.html\n"
           "\n"
           "    Update an existing Word document based on a modified HTML file. The\n"
           "    Word document must already exist, and must be same document from\n"
           "    which the HTML file was originally generated.\n"
           "\n"
           "    The put operation cannot be executed twice on the same Word\n"
           "    document, because after the first time, the fact that the document\n"
           "    is modified will mean that the ids become out of sync with the HTML\n"
           "    file.  If you want to update a document multiple times, you must\n"
           "    create a copy of the .docx file each time, and update that.\n"
           "\n"
           "dfconvert create concrete.docx abstract.html\n"
           "\n"
           "    Create a new Word document from a HTML file. The Word document must\n"
           "    not already exist.\n"
           "\n");
}

int main(int argc, const char **argv)
{
    DFError *error = NULL;
    if ((argc == 4) && !strcmp(argv[1],"get")) {
        if (DFGetFile(argv[2],argv[3],&error))
            return 0;
    }
    else if ((argc == 4) && !strcmp(argv[1],"put")) {
        if (DFPutFile(argv[2],argv[3],&error))
            return 0;
    }
    else if ((argc == 4) && !strcmp(argv[1],"create")) {
        if (DFCreateFile(argv[2],argv[3],&error))
            return 0;
    }
    else {
        usage();
        return 0;
    }

    fprintf(stderr,"%s\n",DFErrorMessage(&error));
    DFErrorRelease(error);
    return 1;
}
