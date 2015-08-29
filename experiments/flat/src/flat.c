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

#include "Common.h"
#include "BuildGrammar.h"
#include "Builtin.h"
#include "Parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_SIZE 1024

static char *readStringFromFile(const char *filename)
{
    FILE *f = fopen(filename,"rb");
    if (f == NULL)
        return NULL;

    char *data = (char *)malloc(READ_SIZE);
    size_t len = 0;
    size_t r;
    while (0 < (r = fread(&data[len],1,READ_SIZE,f))) {
        len += r;
        data = (char*)realloc(data,len+READ_SIZE);
    }
    data = (char*)realloc(data,len+1);
    data[len] = '\0';
    fclose(f);
    return data;
}

static Grammar *grammarFromStr(Grammar *flatGrammar, const char *filename, const char *input)
{
    Term *term = parse(flatGrammar,"Grammar",input,0,strlen(input));
    if (term == NULL) {
        fprintf(stderr,"%s: Parse failed\n",filename);
        exit(1);
    }

    return grammarFromTerm(term,input);
}

void usage(void)
{
    printf("Usage:\n"
           "\n"
           "flat -g\n"
           "\n"
           "    Print the built-in PEG grammar\n"
           "\n"
           "flat -p FILENAME\n"
           "\n"
           "    Parse FILENAME using the built-in PEG grammar, and print out the resulting\n"
           "    parse tree\n"
           "\n"
           "flat -b FILENAME\n"
           "\n"
           "    Parse FILENAME using the built-in PEG grammar, then use the resulting parse\n"
           "    tree to build a Grammar object, and print out the constructed grammar.\n"
           "\n"
           "flat GRAMMAR INPUT\n"
           "\n"
           "    Use the grammar defined in file GRAMMAR to parse the file INPUT, and print\n"
           "    out the resulting parse tree\n"
           "\n");
    exit(1);
}

char *maybeReadFile(const char *filename)
{
    if (filename == NULL)
        return NULL;
    char *str = readStringFromFile(filename);
    if (str == NULL) {
        perror(filename);
        exit(1);
    }
    return str;
}

int main(int argc, const char **argv)
{
    const char *grammarFilename = NULL;
    const char *inputFilename = NULL;
    char *grammarStr = NULL;
    char *inputStr = NULL;
    int useBuiltinGrammar = 0;
    int showGrammar = 0;
    Grammar *builtGrammar = NULL;
    Term *inputTerm = NULL;

    if ((argc == 2) && !strcmp(argv[1],"-g")) {
        useBuiltinGrammar = 1;
        showGrammar = 1;
    }
    else if ((argc == 3) && !strcmp(argv[1],"-p")) {
        useBuiltinGrammar = 1;
        inputFilename = argv[2];
    }
    else if ((argc == 3) && !strcmp(argv[1],"-b")) {
        grammarFilename = argv[2];
        showGrammar = 1;
    }
    else if (argc == 3) {
        grammarFilename = argv[1];
        inputFilename = argv[2];
    }
    else {
        usage();
    }

    Grammar *flatGrammar = GrammarNewBuiltin();

    inputStr = maybeReadFile(inputFilename);
    grammarStr = maybeReadFile(grammarFilename);

    if (grammarStr != NULL) {
        builtGrammar = grammarFromStr(flatGrammar,grammarFilename,grammarStr);
        if (builtGrammar == NULL) {
            fprintf(stderr,"Cannot build grammar\n");
            exit(1);
        }
    }

    Grammar *useGrammar = NULL;

    if (useBuiltinGrammar)
        useGrammar = flatGrammar;
    else if (builtGrammar != NULL)
        useGrammar = builtGrammar;
    else
        usage();

    if (inputStr != NULL) {
        const char *firstRuleName = GrammarFirstRuleName(useGrammar);
        inputTerm = parse(useGrammar,firstRuleName,inputStr,0,strlen(inputStr));
        if (inputTerm == NULL) {
            fprintf(stderr,"%s: Parse failed\n",inputFilename);
            exit(1);
        }
    }

    if (showGrammar) {
        GrammarPrint(useGrammar);
    }
    else if (inputTerm != NULL) {
        TermPrint(inputTerm,inputStr,"");
    }
    else {
        usage();
    }

    free(grammarStr);
    free(inputStr);
    GrammarFree(flatGrammar);
    if (builtGrammar != NULL)
        GrammarFree(builtGrammar);
    return 0;
}
