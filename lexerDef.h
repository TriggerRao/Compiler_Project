#ifndef LEXERDEF_H
#define LEXERDEF_H

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

typedef struct {
    const char *type;  // Changed to const char*
    char *lexeme;
    int line;
} TokenInfo;

struct llnode{
    char * key;
    const char * token;
    struct llnode * next;
};

extern char *current_buffer, *next_buffer;
extern int curr, forw, lineno;
extern FILE *fp;
extern struct llnode *ll;
extern size_t char_read;

#endif // LEXERDEF_H