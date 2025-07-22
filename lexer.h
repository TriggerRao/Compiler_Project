#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexerDef.h" // Ensure this file contains all necessary type definitions



// Stream and token processing
FILE *getStream(FILE *fp, int space_to_fill);
void checklimits(void);
TokenInfo getNextToken(void);

// Comment removal
void removeComments(char *testcaseFile);
void populateKeywords();
long getFileSize(FILE* fp);
void printFileContents(FILE * fp);
void printLexemesinFile(char *fileName);
void printLexemes(char *fileName);

#endif // LEXER_H