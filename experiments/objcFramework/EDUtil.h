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

#import <Foundation/Foundation.h>
#import <Editor/DocFormats.h>

extern int debugIndent;
void debug(NSString *format, ...) NS_FORMAT_FUNCTION(1,2);
DFHashTable *HashTableFromNSDictionary(NSDictionary *dict);
NSDictionary *NSDictionaryFromHashTable(DFHashTable *hashTable);
NSDictionary *NSDictionaryFromNestedHashTable(DFHashTable *outerHash);
NSString *DFFixStringEncoding(NSString *str);
NSString *NSStringFromC(const char *cstr);
void NSErrorSetFromDFError(NSError **nserror, DFError *dferror);
void DFErrorReleaseToNSError(DFError *dferror, NSError **nserror);
void DFErrorSetFromNSError(DFError **dferror, NSError *nserror);

BOOL EDStringEquals(NSString *a, NSString *b);
NSString *EDEncodeFontFamily(NSString *input);
NSString *EDDecodeFontFamily(NSString *input);
