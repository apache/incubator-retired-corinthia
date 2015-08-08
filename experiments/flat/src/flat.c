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

static Grammar *grammarFromFile(const char *filename)
{
    char *input = readStringFromFile(filename);
    if (input == NULL) {
        perror(filename);
        exit(1);
    }

    Grammar *flatGrammar = GrammarNewBuiltin();
    Term *term = parse(flatGrammar,"Grammar",input,0,strlen(input));
    if (term == NULL) {
        fprintf(stderr,"%s: Parse failed\n",filename);
        exit(1);
    }

    Grammar *builtGrammar = grammarFromTerm(term,input);

    free(input);
    GrammarFree(flatGrammar);

    return builtGrammar;
}

int main(int argc, const char **argv)
{

    if ((argc == 2) && !strcmp(argv[1],"-g")) {
        // Build and print out the built-in PEG grammar
        Grammar *gram = GrammarNewBuiltin();
        GrammarPrint(gram);
        GrammarFree(gram);
    }
    else if ((argc == 3) && !strcmp(argv[1],"-p")) {
        const char *filename = argv[2];
        char *input = readStringFromFile(filename);
        if (input == NULL) {
            perror(filename);
            exit(1);
        }
        Grammar *gram = GrammarNewBuiltin();
        Term *term = parse(gram,"Grammar",input,0,strlen(input));
        if (term == NULL) {
            fprintf(stderr,"%s: Parse failed\n",filename);
            exit(1);
        }
        TermPrint(term,input,"");
        free(input);
        GrammarFree(gram);
    }
    else if ((argc == 3) && !strcmp(argv[1],"-b")) {
        Grammar *built = grammarFromFile(argv[2]);
        GrammarPrint(built);
        GrammarFree(built);
    }
    else if (argc == 3) {
        const char *grammarFilename = argv[1];
        const char *inputFilename = argv[2];

        char *inputStr = readStringFromFile(inputFilename);
        if (inputStr == NULL) {
            perror(inputFilename);
            exit(1);
        }

        Grammar *builtGrammar = grammarFromFile(grammarFilename);
        const char *firstRuleName = GrammarFirstRuleName(builtGrammar);
        Term *inputTerm = parse(builtGrammar,firstRuleName,inputStr,0,strlen(inputStr));
        if (inputTerm == NULL) {
            fprintf(stderr,"%s: Parse failed\n",inputFilename);
            exit(1);
        }
        TermPrint(inputTerm,inputStr,"");

        free(inputStr);
        GrammarFree(builtGrammar);
    }
    else {
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

        return 1;
    }
    return 0;
}
