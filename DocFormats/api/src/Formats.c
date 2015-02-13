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

#include <DocFormats/Formats.h>
#include "DFPlatform.h"
#include "DFFilesystem.h"
#include "DFString.h"
#include <stdlib.h>

DFFileFormat DFFileFormatFromFilename(const char *filename)
{
    char *ext = DFPathExtension(filename);
    DFFileFormat format = DFFileFormatFromExtension(ext);
    free(ext);
    return format;
}

DFFileFormat DFFileFormatFromExtension(const char *ext)
{
    if (DFStringEqualsCI(ext,"html"))
        return DFFileFormatHTML;
    else if (DFStringEqualsCI(ext,"docx"))
        return DFFileFormatDocx;
    else if (DFStringEqualsCI(ext,"xlsx"))
        return DFFileFormatXlsx;
    else if (DFStringEqualsCI(ext,"pptx"))
        return DFFileFormatPptx;
    else if (DFStringEqualsCI(ext,"odt"))
        return DFFileFormatOdt;
    else if (DFStringEqualsCI(ext,"ods"))
        return DFFileFormatOds;
    else if (DFStringEqualsCI(ext,"odp"))
        return DFFileFormatOdp;
    else if (DFStringEqualsCI(ext,"md"))
        return DFFileFormatMarkdown;
    else if (DFStringEqualsCI(ext,"tex"))
        return DFFileFormatLaTeX;
    else if (DFStringEqualsCI(ext,"rtf"))
        return DFFileFormatRTF;
    else if (DFStringEqualsCI(ext,"scriv"))
        return DFFileFormatScriv;
    else if (DFStringEqualsCI(ext,"epub"))
        return DFFileFormatEPUB;
    else
        return DFFileFormatUnknown;
}

const char *DFFileFormatToExtension(DFFileFormat format)
{
    switch (format) {
        case DFFileFormatUnknown:
            return NULL;
        case DFFileFormatHTML:
            return "html";
        case DFFileFormatDocx:
            return "docx";
        case DFFileFormatXlsx:
            return "xlsx";
        case DFFileFormatPptx:
            return "pptx";
        case DFFileFormatOdt:
            return "odt";
        case DFFileFormatOds:
            return "ods";
        case DFFileFormatOdp:
            return "odp";
        case DFFileFormatMarkdown:
            return "md";
        case DFFileFormatLaTeX:
            return "tex";
        case DFFileFormatRTF:
            return "rtf";
        case DFFileFormatScriv:
            return "scriv";
        case DFFileFormatEPUB:
            return "epub";
    }
    return NULL;
}
