#!/usr/local/bin/phantomjs --web-security=no

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

/*

Output: CSV file with the following fields:

type (element, attribute, or namespace)
namespaceURI
localName
source file

*/

var RELAXNG_NAMESPACE = "http://relaxng.org/ns/structure/1.0";
var XML_NAMESPACE = "http://www.w3.org/XML/1998/namespace";

var filenames = [
    "ODF/OpenDocument-v1.2-os-schema.rng",
    "ODF/OpenDocument-v1.2-os-manifest-schema.rng",
    "OOXML/transitional/DrawingML_Chart.rng",
    "OOXML/transitional/DrawingML_Chart_Drawing.rng",
    "OOXML/transitional/DrawingML_Diagram_Colors.rng",
    "OOXML/transitional/DrawingML_Diagram_Data.rng",
    "OOXML/transitional/DrawingML_Diagram_Layout_Definition.rng",
    "OOXML/transitional/DrawingML_Diagram_Style.rng",
    "OOXML/transitional/DrawingML_Table_Styles.rng",
    "OOXML/transitional/DrawingML_Theme.rng",
    "OOXML/transitional/DrawingML_Theme_Override.rng",
    "OOXML/transitional/PresentationML_Comment_Authors.rng",
    "OOXML/transitional/PresentationML_Comments.rng",
    "OOXML/transitional/PresentationML_Handout_Master.rng",
    "OOXML/transitional/PresentationML_Notes_Master.rng",
    "OOXML/transitional/PresentationML_Notes_Slide.rng",
    "OOXML/transitional/PresentationML_Presentation.rng",
    "OOXML/transitional/PresentationML_Presentation_Properties.rng",
    "OOXML/transitional/PresentationML_Slide.rng",
    "OOXML/transitional/PresentationML_Slide_Layout.rng",
    "OOXML/transitional/PresentationML_Slide_Master.rng",
    "OOXML/transitional/PresentationML_Slide_Synchronization_Data.rng",
    "OOXML/transitional/PresentationML_User-Defined_Tags.rng",
    "OOXML/transitional/PresentationML_View_Properties.rng",
    "OOXML/transitional/Shared_Additional_Characteristics.rng",
    "OOXML/transitional/Shared_Bibliography.rng",
    "OOXML/transitional/Shared_Custom_File_Properties.rng",
    "OOXML/transitional/Shared_Custom_XML_Data_Storage_Properties.rng",
    "OOXML/transitional/Shared_Extended_File_Properties.rng",
    "OOXML/transitional/SpreadsheetML_Calculation_Chain.rng",
    "OOXML/transitional/SpreadsheetML_Chartsheet.rng",
    "OOXML/transitional/SpreadsheetML_Comments.rng",
    "OOXML/transitional/SpreadsheetML_Connections.rng",
    "OOXML/transitional/SpreadsheetML_Custom_XML_Mappings.rng",
    "OOXML/transitional/SpreadsheetML_Dialogsheet.rng",
    "OOXML/transitional/SpreadsheetML_Drawing.rng",
    "OOXML/transitional/SpreadsheetML_External_Workbook_References.rng",
    "OOXML/transitional/SpreadsheetML_Metadata.rng",
    "OOXML/transitional/SpreadsheetML_Pivot_Table.rng",
    "OOXML/transitional/SpreadsheetML_Pivot_Table_Cache_Definition.rng",
    "OOXML/transitional/SpreadsheetML_Pivot_Table_Cache_Records.rng",
    "OOXML/transitional/SpreadsheetML_Query_Table.rng",
    "OOXML/transitional/SpreadsheetML_Shared_String_Table.rng",
    "OOXML/transitional/SpreadsheetML_Shared_Workbook_Revision_Headers.rng",
    "OOXML/transitional/SpreadsheetML_Shared_Workbook_Revision_Log.rng",
    "OOXML/transitional/SpreadsheetML_Shared_Workbook_User_Data.rng",
    "OOXML/transitional/SpreadsheetML_Single_Cell_Table_Definitions.rng",
    "OOXML/transitional/SpreadsheetML_Styles.rng",
    "OOXML/transitional/SpreadsheetML_Table_Definitions.rng",
    "OOXML/transitional/SpreadsheetML_Volatile_Dependencies.rng",
    "OOXML/transitional/SpreadsheetML_Workbook.rng",
    "OOXML/transitional/SpreadsheetML_Worksheet.rng",
//    "OOXML/transitional/VML_Drawing.rng",
    "OOXML/transitional/WordprocessingML_Comments.rng",
    "OOXML/transitional/WordprocessingML_Document_Settings.rng",
    "OOXML/transitional/WordprocessingML_Endnotes.rng",
    "OOXML/transitional/WordprocessingML_Font_Table.rng",
    "OOXML/transitional/WordprocessingML_Footer.rng",
    "OOXML/transitional/WordprocessingML_Footnotes.rng",
    "OOXML/transitional/WordprocessingML_Glossary_Document.rng",
    "OOXML/transitional/WordprocessingML_Header.rng",
    "OOXML/transitional/WordprocessingML_Mail_Merge_Recipient_Data.rng",
    "OOXML/transitional/WordprocessingML_Main_Document.rng",
    "OOXML/transitional/WordprocessingML_Numbering_Definitions.rng",
    "OOXML/transitional/WordprocessingML_Style_Definitions.rng",
    "OOXML/transitional/WordprocessingML_Web_Settings.rng",
    "OOXML/transitional/any.rng",
    "OOXML/transitional/dml-chart.rng",
    "OOXML/transitional/dml-chartDrawing.rng",
    "OOXML/transitional/dml-diagram.rng",
    "OOXML/transitional/dml-lockedCanvas.rng",
    "OOXML/transitional/dml-main.rng",
    "OOXML/transitional/dml-picture.rng",
    "OOXML/transitional/dml-spreadsheetDrawing.rng",
    "OOXML/transitional/dml-wordprocessingDrawing.rng",
    "OOXML/transitional/pml.rng",
    "OOXML/transitional/shared-additionalCharacteristics.rng",
    "OOXML/transitional/shared-bibliography.rng",
    "OOXML/transitional/shared-commonSimpleTypes.rng",
    "OOXML/transitional/shared-customXmlDataProperties.rng",
    "OOXML/transitional/shared-customXmlSchemaProperties.rng",
    "OOXML/transitional/shared-documentPropertiesCustom.rng",
    "OOXML/transitional/shared-documentPropertiesExtended.rng",
//    "OOXML/transitional/shared-documentPropertiesVariantTypes.rng",
    "OOXML/transitional/shared-math.rng",
    "OOXML/transitional/shared-relationshipReference.rng",
    "OOXML/transitional/sml.rng",
    "OOXML/transitional/vml-main.rng",
    "OOXML/transitional/vml-officeDrawing.rng",
//    "OOXML/transitional/vml-presentationDrawing.rng",
//    "OOXML/transitional/vml-spreadsheetDrawing.rng",
//    "OOXML/transitional/vml-wordprocessingDrawing.rng",
    "OOXML/transitional/wml.rng",
    "OOXML/transitional/xml.rng"];








var fs = require("fs");
var entries = new Array();

function Entry(type,namespaceURI,localName,filename)
{
    this.type = type;
    this.namespaceURI = namespaceURI;
    this.localName = localName;
    this.filename = filename;
}

function debug(str)
{
    console.log(str);
}

function resolvePrefix(node,prefix)
{
    if (node == null)
        return null;

    if (node.attributes != null) {
        for (var i = 0; i < node.attributes.length; i++) {
            var attr = node.attributes[i];
            if ((attr.prefix == "xmlns") && (attr.localName == prefix))
                return node.getAttribute(attr.nodeName);
            if ((prefix == null) && (attr.localName == "ns"))
                return node.getAttribute(attr.nodeName);
        }
    }

    return resolvePrefix(node.parentNode,prefix);
}

function foundEntry(node,type,name,filename)
{
    var re = /^(([^:]+):)?([^:]+)/;
    var result = re.exec(name);

    var prefix = result[2];
    var localName = result[3];

    var namespaceURI = null;
    if (prefix == "xml") {
        namespaceURI = "http://www.w3.org/XML/1998/namespace";
    }
    else {
        namespaceURI = resolvePrefix(node,prefix);
//        if (namespaceURI == null)
//            throw new Error("Can't resolve namespace prefix "+prefix);
    }

    var entry = new Entry(type,namespaceURI,localName,filename);
    entries.push(entry);
}

function recurse(node,filename)
{
    if (node.nodeType == Node.ELEMENT_NODE) {
        if ((node.localName == "element") && node.hasAttribute("name"))
            foundEntry(node,"element",node.getAttribute("name"),filename);
        else if ((node.localName == "attribute") && node.hasAttribute("name"))
            foundEntry(node,"attribute",node.getAttribute("name"),filename);
    }

    if (node.attributes != null) {
        for (var i = 0; i < node.attributes.length; i++) {
            var attr = node.attributes[i];
            if (attr.prefix == "xmlns") {
                var prefix = attr.localName;
                var namespaceURI = node.getAttribute(attr.nodeName);
                entries.push(new Entry("namespace",namespaceURI,prefix,filename));
            }
        }
    }


    for (var child = node.firstChild; child != null; child = child.nextSibling) {
        recurse(child,filename);
    }
}

function processRelaxNG(filename)
{
    var data = fs.read(filename);
    var parser = new DOMParser();
    var doc = parser.parseFromString(data,"text/xml");
    if (doc == null)
        throw new Error("Can't parse "+filename);
    recurse(doc.documentElement,filename);
}

function printEntries()
{
    for (var i = 0; i < entries.length; i++) {
        var entry = entries[i];
        var namespaceURI = (entry.namespaceURI != null) ? entry.namespaceURI : "";
        var localName = (entry.localName != null) ? entry.localName : "";
        var filename = (entry.filename != null) ? entry.filename : "";
        debug(entry.type+","+namespaceURI+","+localName+","+filename);
    }
}

function main()
{
    try {
        for (var i = 0; i < filenames.length; i++)
            processRelaxNG(filenames[i]);
        printEntries();
        phantom.exit(0);
    }
    catch (e) {
        debug("Error: "+e);
        phantom.exit(1);
    }
}

main();
