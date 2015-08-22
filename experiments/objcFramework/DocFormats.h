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

// This file comes from the portion of the UX Write editor that
// works on both Apple platforms (that is, it can run on either
// OS X or iOS). It's in the repository for illustrative purposes
// only, to assist with the creation of the framework for the
// Corinthia editor UI. The code does not compile independently in
// its present form.

/** \mainpage

 Documentation for the DocFormats library

 # CSS

 \see CSSProperties.h
 \see CSSSheet.h
 \see CSSStyle.h

 # XML

 \see DFDOM.h

 # Word

 \see WordPPr.h

 */

#import <Foundation/Foundation.h>

#import "DFPlatform.h"
#import <DocFormats/DocFormats.h>

//#import <DocFormats/DFXMLNames.h>
#import "DFXMLNames.h"
//#import <DocFormats/DFXMLNamespaces.h>
#import "DFXMLNamespaces.h"

//#import <DocFormats/DFDOM.h>
//#import <DocFormats/DFXML.h>
#import "DFDOM.h"
#import "DFXML.h"
#import <DocFormats/DFXMLForward.h>

#import "CSS.h"
#import "CSSClassNames.h"
#import "CSSLength.h"
#import "CSSProperties.h"
#import "CSSSelector.h"
#import "CSSStyle.h"
#import "CSSSheet.h"

#import "DFHTML.h"
#import "DFHTMLNormalization.h"

#import "DFString.h"
//#import "DFHashTable.h"
#import "DFCallback.h"
#import "DFFilesystem.h"
#import <DocFormats/DFError.h>
#import "DFZipFile.h"
#import "Word.h"
#import "HTMLToLaTeX.h"
