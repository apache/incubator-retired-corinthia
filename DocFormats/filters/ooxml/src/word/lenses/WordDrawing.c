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
#include "WordDrawing.h"
#include "WordLenses.h"
#include "WordObjects.h"
#include "WordSection.h"
#include "CSSLength.h"
#include "CSSProperties.h"
#include "DFHTML.h"
#include "DFFilesystem.h"
#include "OPC.h"
#include "DFDOM.h"
#include "DFString.h"
#include "DFPlatform.h"
#include "DFCommon.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int internalPut(WordPutData *put, DFNode *abstract, DFNode *concrete, int isNew);
static void populateDrawingElement(WordConverter *converter, DFNode *root, double widthPts,
                                   double heightPts, const char *drawingId, const char *rId);

typedef struct {
    unsigned int widthPx;
    unsigned int heightPx;
} PixelSize;

const static PixelSize PixelSizeZero = { 0, 0 };

typedef struct {
    unsigned long long widthEmu;
    unsigned long long heightEmu;
} EMUSize;

typedef struct {
    double widthPts;
    double heightPts;
} PointsSize;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            ImageInfo                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    char *rId;
    double widthPts;
    double heightPts;
    const char *progId; // for embedded objects in VML
} ImageInfo;

static ImageInfo *ImageInfoNew(const char *rId, double widthPts, double heightPts)
{
    ImageInfo *info = (ImageInfo *)calloc(1,sizeof(ImageInfo));
    info->rId = DFStrDup(rId);
    info->widthPts = widthPts;
    info->heightPts = heightPts;
    return info;
}

static void ImageInfoFree(ImageInfo *info)
{
    free(info->rId);
    free(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           DrawingInfo                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *DrawingInfoTop(DFNode *element)
{
    assert(element->tag == WORD_DRAWING);
    DFNode *top = DFChildWithTag(element,DML_WP_INLINE);
    if (top == NULL)
        top = DFChildWithTag(element,DML_WP_ANCHOR);
    return top;
}

static DFNode *DrawingInfoBlip(DFNode *element)
{
    DFNode *top = DrawingInfoTop(element);
    DFNode *graphic = DFChildWithTag(top,DML_MAIN_GRAPHIC);
    DFNode *graphicData = DFChildWithTag(graphic,DML_MAIN_GRAPHICDATA);
    DFNode *pic = DFChildWithTag(graphicData,DML_PICTURE_PIC);
    DFNode *blipFill = DFChildWithTag(pic,DML_PICTURE_BLIPFILL);
    DFNode *blip = DFChildWithTag(blipFill,DML_MAIN_BLIP);
    return blip;
}

static const char *DrawingInfoRId(DFNode *element)
{
    DFNode *blip = DrawingInfoBlip(element);
    return DFGetAttribute(blip,OREL_EMBED);
}

static EMUSize DrawingInfoSize(DFNode *element)
{
    EMUSize result = { 0, 0 };

    DFNode *top = DrawingInfoTop(element);
    DFNode *extent = DFChildWithTag(top,DML_WP_EXTENT);
    const char *cx = DFGetAttribute(extent,NULL_CX);
    const char *cy = DFGetAttribute(extent,NULL_CY);
    if ((cx != NULL) && (cy != NULL)) {
        result.widthEmu = atoll(cx);
        result.heightEmu = atoll(cy);
    }

    return result;
}

static const char *DrawingInfoDrawingId(DFNode *element)
{
    DFNode *top = DrawingInfoTop(element);
    DFNode *docPr = DFChildWithTag(top,DML_WP_DOCPR);
    return DFGetAttribute(docPr,NULL_ID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                           WordDrawing                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

WordDrawing *WordDrawingNew(const char *drawingId)
{
    WordDrawing *drawing = (WordDrawing *)calloc(1,sizeof(WordDrawing));
    drawing->retainCount = 1;
    drawing->drawingId = strdup(drawingId);
    return drawing;
}

WordDrawing *WordDrawingRetain(WordDrawing *drawing)
{
    if (drawing != NULL)
        drawing->retainCount++;
    return drawing;
}

void WordDrawingRelease(WordDrawing *drawing)
{
    if ((drawing == NULL) || (--drawing->retainCount > 0))
        return;

    free(drawing->drawingId);
    free(drawing);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         WordDrawingLens                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

static DFNode *createAbstractPlaceholder(WordGetData *get, const char *placeholderText, DFNode *concrete)
{
    DFNode *span = WordConverterCreateAbstract(get,HTML_SPAN,concrete);
    DFSetAttribute(span,HTML_CLASS,DFPlaceholderClass);
    DFNode *text = DFCreateTextNode(get->conv->html,placeholderText);
    DFAppendChild(span,text);
    return span;
}

static DFNode *imageWithFilename(WordGetData *get, const char *filename, double widthPts, DFNode *concrete)
{
    char *lastComponent = DFPathBaseName(filename);
    char *relImagePath = DFAppendPathComponent("images",lastComponent);
    DFBuffer *content = NULL;

    DFError *error = NULL;
    DFNode *imageNode = NULL;

    content = DFBufferReadFromStorage(get->conv->package->opc->storage,filename,&error);
    if (content == NULL) {
        WordConverterWarning(get->conv,"Copy %s to %s: %s",filename,relImagePath,DFErrorMessage(&error));
        DFErrorRelease(error);
        goto end;
    }

    if (!DFBufferWriteToStorage(content,get->conv->abstractStorage,relImagePath,&error)) {
        WordConverterWarning(get->conv,"Copy %s to %s: %s",filename,relImagePath,DFErrorMessage(&error));
        DFErrorRelease(error);
        goto end;
    }

    imageNode = WordConverterCreateAbstract(get,HTML_IMG,concrete);
    DFFormatAttribute(imageNode,HTML_SRC,"images/%s",lastComponent);

    double contentWidthPts = WordSectionContentWidthPts(get->conv->mainSection);
    if (contentWidthPts > 0) {
        double widthPct = widthPts/contentWidthPts*100.0;
        CSSProperties *properties = CSSPropertiesNew();
        char buf[100];
        CSSPut(properties,"width",DFFormatDoublePct(buf,100,widthPct));
        char *propertiesText = CSSPropertiesCopyDescription(properties);
        DFSetAttribute(imageNode,HTML_STYLE,propertiesText);
        free(propertiesText);
        CSSPropertiesRelease(properties);
    }
    goto end;

end:
    free(lastComponent);
    free(relImagePath);
    DFBufferRelease(content);
    if (imageNode != NULL)
        return imageNode;
    else
        return createAbstractPlaceholder(get,"[Error reading image]",concrete);
}

static ImageInfo *getImageInfoDrawing(DFNode *concrete)
{
    EMUSize size = DrawingInfoSize(concrete);
    if ((size.widthEmu <= 0) || (size.heightEmu <= 0) || (DrawingInfoRId(concrete) == NULL))
        return NULL;

    double widthPts = size.widthEmu/(double)EMUS_PER_POINT;
    double heightPts = size.heightEmu/(double)EMUS_PER_POINT;

    return ImageInfoNew(DrawingInfoRId(concrete),widthPts,heightPts);
}

static ImageInfo *getImageInfoObject(DFNode *concrete)
{
    DFNode *shape = DFChildWithTag(concrete,VML_SHAPE);
    DFNode *imageData = DFChildWithTag(shape,VML_IMAGEDATA);
    const char *cssText = DFGetAttribute(shape,NULL_STYLE);
    const char *rId = DFGetAttribute(imageData,OREL_ID);
    if ((shape == NULL) || (imageData == NULL) || (cssText == NULL) || (rId == NULL))
        return NULL;;

    CSSProperties *imgProperties = CSSPropertiesNewWithString(cssText);
    CSSLength width = CSSLengthFromString(CSSGet(imgProperties,"width"));
    CSSLength height = CSSLengthFromString(CSSGet(imgProperties,"height"));
    CSSPropertiesRelease(imgProperties);

    if (!CSSLengthIsValid(width) || !CSSLengthIsAbsolute(width))
        return NULL;

    if (!CSSLengthIsValid(height) || !CSSLengthIsAbsolute(height))
        return NULL;

    double widthPts = convertBetweenUnits(width.value,width.units,UnitsPt);
    double heightPts = convertBetweenUnits(height.value,height.units,UnitsPt);

    ImageInfo *info = ImageInfoNew(rId,widthPts,heightPts);

    DFNode *oleObject = DFChildWithTag(concrete,MSOFFICE_OLEOBJECT);
    info->progId = DFGetAttribute(oleObject,NULL_PROGID);

    return info;
}

static ImageInfo *getImageInfo(DFNode *concrete)
{
    switch (concrete->tag) {
        case WORD_DRAWING:
            return getImageInfoDrawing(concrete);
        case WORD_OBJECT:
        case WORD_PICT:
            return getImageInfoObject(concrete);
        default:
            return NULL;
    }
}

DFNode *WordDrawingGet(WordGetData *get, DFNode *concrete)
{
    ImageInfo *info = getImageInfo(concrete);
    if (info == NULL)
        return createAbstractPlaceholder(get,"[Unsupported object]",concrete);

    int validFileType = 0;

    const char *filename = WordPackageTargetForDocumentRel(get->conv->package,info->rId);
    if (filename != NULL) {
        char *origExtension = DFPathExtension(filename);
        char *lowerExtension = DFLowerCase(origExtension);
        if (DFHashTableLookup(get->conv->supportedContentTypes,lowerExtension) != NULL)
            validFileType = 1;
        free(origExtension);
        free(lowerExtension);
    }

    DFNode *result = NULL;
    if (!validFileType) {
        if (info->progId != NULL) {
            char *message = DFFormatString("[Unsupported object: %s]",info->progId);
            DFNode *placeholder = createAbstractPlaceholder(get,message,concrete);
            free(message);
            result = placeholder;
        }
        else {
            result = createAbstractPlaceholder(get,"[Unsupported object]",concrete);
        }

    }

    if (result == NULL)
        result = imageWithFilename(get,filename,info->widthPts,concrete);

    ImageInfoFree(info);
    return result;
}

int WordDrawingIsVisible(WordPutData *put, DFNode *concrete)
{
    return 1;
}

static char *genImageFilename(DFStorage *storage, const char *mediaRelDir, const char *extension, DFError **error)
{
    const char **paths = DFStorageList(storage,error);
    if (paths == NULL)
        return NULL;;

    DFHashTable *existingPaths = DFHashTableNew((DFCopyFunction)strdup,free);
    for (int i = 0; paths[i]; i++) {
        const char *path = paths[i];
        char *lowerPath = DFLowerCase(path);
        char *noExtension = DFPathWithoutExtension(lowerPath);
        DFHashTableAdd(existingPaths,noExtension,noExtension);
        free(lowerPath);
        free(noExtension);
    }

    int num = 1;
    char *candidateName = NULL;
    char *candidatePath = NULL;
    do {
        free(candidateName);
        free(candidatePath);
        candidateName = DFFormatString("image%d",num);
        candidatePath = DFAppendPathComponent(mediaRelDir,candidateName);
        num++;
    } while (DFHashTableLookup(existingPaths,candidatePath) != NULL);

    char *result = DFFormatString("%s.%s",candidateName,extension);

    free(candidateName);
    free(candidatePath);
    DFHashTableRelease(existingPaths);
    free(paths);
    return result;
}

static OPCRelationship *addImageRelationship(WordConverter *converter, const char *unescapedSrc, DFError **error)
{
    DFStorage *storage = converter->package->opc->storage;
    const char *mediaDir = "word/media";

    char *ext = DFPathExtension(unescapedSrc);
    char *filename = genImageFilename(storage,mediaDir,ext,error);
    free(ext);
    if (filename == NULL)
        return NULL;

    char *destPath = DFAppendPathComponent(mediaDir,filename);

    OPCRelationship *result = NULL;
    DFBuffer *content = DFBufferReadFromStorage(converter->abstractStorage,unescapedSrc,error);
    if ((content != NULL) && DFBufferWriteToStorage(content,storage,destPath,error)) {
        OPCRelationshipSet *rels = converter->package->documentPart->relationships;
        char *relPath = DFFormatString("/word/media/%s",filename);
        result = OPCRelationshipSetAddType(rels,WORDREL_IMAGE,relPath,0);
        free(relPath);
    }
    DFBufferRelease(content);

    free(filename);
    free(destPath);
    return result;
}

static int getImageFile(WordConverter *converter, const char *unescapedSrc, PixelSize *size, DFError **error)
{
    size->widthPx = 0;
    size->heightPx = 0;

    char *ext = DFPathExtension(unescapedSrc);
    int ok = 0;

    DFBuffer *imageData = DFBufferReadFromStorage(converter->abstractStorage,unescapedSrc,error);
    if (imageData == NULL)
        goto end;

    char *errmsg = NULL;
    ok = DFGetImageDimensions(imageData->data,imageData->len,ext,&size->widthPx,&size->heightPx,&errmsg);
    if (!ok) {
        DFErrorFormat(error,"%s",errmsg);
        free(errmsg);
    }

end:
    DFBufferRelease(imageData);
    free(ext);
    return ok;
}

static PointsSize pointsSizeFromHTMLImg(WordConverter *converter, DFNode *img, PixelSize fileSize)
{
    double aspectRatio;
    if (fileSize.heightPx > 0.0)
        aspectRatio = fileSize.widthPx/(double)fileSize.heightPx;
    else
        aspectRatio = 1.0;

    CSSSize htmlSize = HTML_getImageDimensions(img);

    double widthPts = WordSectionContentWidthPts(converter->mainSection);
    if (CSSLengthIsValid(htmlSize.width)) {
        if (CSSLengthIsPercentage(htmlSize.width))
            widthPts = (htmlSize.width.value/100.0)*WordSectionContentWidthPts(converter->mainSection);
        else if (CSSLengthIsAbsolute(htmlSize.width))
            widthPts = convertBetweenUnits(htmlSize.width.value,htmlSize.width.units,UnitsPt);
    }

    double heightPts = widthPts/aspectRatio;

    if ((widthPts <= 0) || (heightPts <= 0)) {
        widthPts = WordSectionContentWidthPts(converter->mainSection);
        heightPts = widthPts/aspectRatio;
    }

    PointsSize result;
    result.widthPts = widthPts;
    result.heightPts = heightPts;
    return result;
}

static int checkContentType(WordConverter *converter, const char *htmlSrc)
{
    char *extension = DFPathExtension(htmlSrc);
    const char *contentType = DFHashTableLookup(converter->supportedContentTypes,extension);
    if (contentType == NULL) {
        WordConverterWarning(converter,"Unsupported image type: %s",extension);
        free(extension);
        return 0;
    }
    else {
        OPCContentTypesSetDefault(converter->package->opc->contentTypes,extension,contentType);
        free(extension);
        return 1;
    }
}

DFNode *WordDrawingCreate(WordPutData *put, DFNode *abstract)
{
    DFNode *concrete = DFCreateElement(put->contentDoc,WORD_DRAWING);
    if (internalPut(put,abstract,concrete,1))
        return concrete;
    else
        return NULL;
}

static int internalPut2(WordPutData *put, DFNode *abstract, DFNode *concrete, int isNew, const char *htmlFilename);

static int internalPut(WordPutData *put, DFNode *abstract, DFNode *concrete, int isNew)
{
    if (abstract->tag != HTML_IMG)
        return 0;

    switch (concrete->tag) {
        case WORD_DRAWING:
        case WORD_PICT:
        case WORD_OBJECT:
            break;
        default:
            return 0;
    }

    const char *htmlSrc = DFGetAttribute(abstract,HTML_SRC);
    if ((htmlSrc == NULL) || !checkContentType(put->conv,htmlSrc))
        return 0;

    char *htmlFilename = DFRemovePercentEncoding(htmlSrc);
    int r = internalPut2(put,abstract,concrete,isNew,htmlFilename);
    free(htmlFilename);
    return r;
}

static int internalPut2(WordPutData *put, DFNode *abstract, DFNode *concrete, int isNew, const char *htmlFilename)
{
    int imageChanged = 0;
    int sizeChanged = 0;

    if (!DFStorageExists(put->conv->abstractStorage,htmlFilename)) {
        WordConverterWarning(put->conv,"HTML image %s does not exist",htmlFilename);
        return 0;
    }

    PixelSize htmlFileSize = PixelSizeZero;
    DFError *error = NULL;
    if (!getImageFile(put->conv,htmlFilename,&htmlFileSize,&error)) {
        WordConverterWarning(put->conv,"Could not get aspect ratio of image: %s\n",DFErrorMessage(&error));
        DFErrorRelease(error);
        return 0;
    }
    PointsSize htmlSize = pointsSizeFromHTMLImg(put->conv,abstract,htmlFileSize);

    OPCRelationship *rel = NULL;
    const char *drawingId = NULL;

    if (isNew) {
        imageChanged = 1;
        sizeChanged = 1;
    }
    else {
        ImageInfo *wordInfo = getImageInfo(concrete);
        if (wordInfo == NULL)
            return 0;

        rel = OPCRelationshipSetLookupById(put->conv->package->documentPart->relationships,wordInfo->rId);

        if ((wordInfo != NULL) && (wordInfo->widthPts > 0) && (wordInfo->heightPts > 0) && (rel != NULL)) {
            const char *wordSrc = rel->target;
            DFStorage *storage = put->conv->package->opc->storage;

            if (!DFStorageExists(storage,wordSrc)) {
                WordConverterWarning(put->conv,"Word image %s does not exist",wordSrc);
                ImageInfoFree(wordInfo);
                return 0;
            }

            DFBuffer *content1 = DFBufferReadFromStorage(put->conv->abstractStorage,htmlFilename,NULL);
            DFBuffer *content2 = DFBufferReadFromStorage(storage,wordSrc,NULL);
            int contentsEqual = ((content1 != NULL) && (content2 != NULL) &&
                                 (content1->len == content2->len) &&
                                 !memcmp(content1->data,content2->data,content1->len));
            DFBufferRelease(content1);
            DFBufferRelease(content2);

            if (!contentsEqual) {
                rel->needsRemoveCheck = 1;
                imageChanged = 1;
            }

            if (fabs(wordInfo->widthPts - htmlSize.widthPts) >= 0.1)
                sizeChanged = 1;
        }
        ImageInfoFree(wordInfo);

        if (concrete->tag == WORD_DRAWING)
            drawingId = DrawingInfoDrawingId(concrete);
    }

    if (imageChanged || sizeChanged) {

        if (imageChanged || (rel == NULL)) { // FIXME: is the rel == NULL needed?
            DFError *error = NULL;
            rel = addImageRelationship(put->conv,htmlFilename,&error);
            if (rel == NULL) {
                WordConverterWarning(put->conv,"%s",DFErrorMessage(&error));
                return 0;
            }
        }

        if (drawingId == NULL)
            drawingId = WordObjectsAddDrawing(put->conv->objects)->drawingId;

        populateDrawingElement(put->conv,concrete,htmlSize.widthPts,htmlSize.heightPts,
                               drawingId,rel->rId);
    }

    return 1;
}

void WordDrawingPut(WordPutData *put, DFNode *abstract, DFNode *concrete)
{
    internalPut(put,abstract,concrete,0);
}

static void populateDrawingElement(WordConverter *converter, DFNode *root, double widthPts,
                                   double heightPts, const char *drawingId, const char *rId)
{
    EMUSize size;
    size.widthEmu = (unsigned long long)round(widthPts*EMUS_PER_POINT);
    size.heightEmu = (unsigned long long)round(heightPts*EMUS_PER_POINT);

    root->tag = WORD_DRAWING;
    DFRemoveAllAttributes(root);

    while (root->first != NULL)
        DFRemoveNode(root->first);

    char *docName = DFFormatString("Picture %s",drawingId);
    const char *imgName = "image";
    const char *imgId = "0";
    DFNode *inlin = DFCreateChildElement(root,DML_WP_INLINE);

    DFNode *extent = DFCreateChildElement(inlin,DML_WP_EXTENT); // a_CT_PositiveSize2D
    DFFormatAttribute(extent,NULL_CX,"%llu",size.widthEmu);
    DFFormatAttribute(extent,NULL_CY,"%llu",size.heightEmu);
    DFNode *docPr = DFCreateChildElement(inlin,DML_WP_DOCPR); // a_CT_NonVisualDrawingProps
    DFSetAttribute(docPr,NULL_ID,drawingId);
    DFSetAttribute(docPr,NULL_NAME,docName);
    DFNode *cNvGraphicFramePr = DFCreateChildElement(inlin,DML_WP_CNVGRAPHICFRAMEPR);
    DFNode *graphicFrameLocks = DFCreateChildElement(cNvGraphicFramePr,DML_MAIN_GRAPHICFRAMELOCKS);
    DFSetAttribute(graphicFrameLocks,NULL_NOCHANGEASPECT,"1");

    DFNode *graphic = DFCreateChildElement(inlin,DML_MAIN_GRAPHIC);
    DFNode *graphicData = DFCreateChildElement(graphic,DML_MAIN_GRAPHICDATA);
    DFSetAttribute(graphicData,NULL_URI,"http://schemas.openxmlformats.org/drawingml/2006/picture");

    DFNode *pic = DFCreateChildElement(graphicData,DML_PICTURE_PIC);
    DFNode *nvPicPr = DFCreateChildElement(pic,DML_PICTURE_NVPICPR);
    DFNode *blipFill = DFCreateChildElement(pic,DML_PICTURE_BLIPFILL); // a_CT_BlipFillProperties
    DFNode *spPr = DFCreateChildElement(pic,DML_PICTURE_SPPR); // a_CT_ShapeProperties

    // Graphic - Non-visual properties
    DFNode *cNvPr = DFCreateChildElement(nvPicPr,DML_PICTURE_CNVPR);
    DFNode *cNvPicPr = DFCreateChildElement(nvPicPr,DML_PICTURE_CNVPICPR);
    DFSetAttribute(cNvPr,NULL_ID,imgId);
    DFSetAttribute(cNvPr,NULL_NAME,imgName);

    // Blip
    DFNode *blip = DFCreateChildElement(blipFill,DML_MAIN_BLIP);
    DFSetAttribute(blip,OREL_EMBED,rId);
    DFNode *stretch = DFCreateChildElement(blipFill,DML_MAIN_STRETCH);
    DFNode *fillRect = DFCreateChildElement(stretch,DML_MAIN_FILLRECT);

    // Shape properties
    DFNode *xfrm = DFCreateChildElement(spPr,DML_MAIN_XFRM);
    DFNode *off = DFCreateChildElement(xfrm,DML_MAIN_OFF);
    DFSetAttribute(off,NULL_X,"0");
    DFSetAttribute(off,NULL_Y,"0");
    DFNode *ext = DFCreateChildElement(xfrm,DML_MAIN_EXT);
    DFFormatAttribute(ext,NULL_CX,"%llu",size.widthEmu);
    DFFormatAttribute(ext,NULL_CY,"%llu",size.heightEmu);
    DFNode *prstGeom = DFCreateChildElement(spPr,DML_MAIN_PRSTGEOM);
    DFSetAttribute(prstGeom,NULL_PRST,"rect");


    (void)cNvPicPr;
    (void)fillRect;
    free(docName);
}

void WordDrawingRemove(WordPutData *put, DFNode *concrete)
{
    // FIXME: This only causes the relationship to be deleted - the image file itself is not removed

    if (concrete->tag == WORD_DRAWING) {
        EMUSize wordEMUSize = DrawingInfoSize(concrete);
        const char *wordRId = DrawingInfoRId(concrete);
        const char *drawingId = DrawingInfoDrawingId(concrete);
        OPCRelationship *rel = NULL;

        if (wordRId != NULL)
            rel = OPCRelationshipSetLookupById(put->conv->package->documentPart->relationships,wordRId);

        if ((wordEMUSize.widthEmu > 0) && (wordEMUSize.heightEmu > 0) && (rel != NULL) && (drawingId != NULL)) {
            rel->needsRemoveCheck = 1;
        }
    }
    else if (concrete->tag == WORD_OBJECT) {
    }
}

WordLens WordDrawingLens = {
    .isVisible = WordDrawingIsVisible,
    .get = WordDrawingGet,
    .put = WordDrawingPut,
    .create = WordDrawingCreate,
    .remove = WordDrawingRemove,
};
