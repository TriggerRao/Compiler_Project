#include "lexerDef.h"
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

char *current_buffer = NULL, *next_buffer = NULL; // Buffers to store the current and next 512 characters
int curr = 0, forw = 0, lineno = 1;             // Pointers to the current and next character in the buffer
FILE *fp = NULL;                                // File pointer to the input file   
struct llnode *ll = NULL;                // Hashmap to store the keywords
size_t char_read = 0;                           // Number of characters read from the file

struct llnode * createNode(char *key, char *token, struct llnode * tail) {
    struct llnode *newNode = (struct llnode *)malloc(sizeof(struct llnode));
    newNode->key = strdup(key);
    newNode->token = strdup(token);
    newNode->next = tail;
    return newNode;
}

void populateKeywords() {
    ll = NULL;
    FILE *file = fopen("project_language/keywords.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, " ");
        char *token = strtok(NULL, "\n");
        ll = createNode(key, token, ll);
    }
    fclose(file);
}

const char* searchLL(struct llnode *head, char *key) {
    struct llnode *current = head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->token;
        }
        current = current->next;
    }
    return "TK_ERROR_PATTERN";
}

FILE *getStream(FILE *fp, int space_to_fill) // Function to fill the buffer with the next 512 characters
{
    free(next_buffer);
    next_buffer = (char *)malloc(sizeof(char) * 512);
    for (int i = 0; i < space_to_fill; i++)
    {
        *(next_buffer + i) = current_buffer[curr++];
    }

    char_read = fread(next_buffer + space_to_fill, 1, 512 - space_to_fill, fp);
    if (char_read == -1)
    {
        printf("Error opening file.");
        return fp;
    }
    char_read += space_to_fill;
    if (char_read < 512)
    {
        char *temp = (char *)malloc(char_read * sizeof(char));
        memcpy(temp, next_buffer, char_read);
        free(next_buffer);
        next_buffer = current_buffer;
        current_buffer = temp;
        return fp;
    }
    char *temp;
    temp = current_buffer;
    current_buffer = next_buffer;
    next_buffer = temp;
    curr = 0;
    forw = space_to_fill;

    return fp;
}

void buffer_check() // Function to check if the buffer has reached its limit
{
    if (forw > 511)
    {
        fp = getStream(fp, 512 - curr);
    }
    return;
}

long getFileSize(FILE * fp) {
    if (fp == NULL) {
        perror("Error opening file");
        return -1;
    }

    fseek(fp, 0, SEEK_END); // Move to the end of the file
    long size = ftell(fp);  // Get the current byte offset in the file

    return size;  // Return the size
}

TokenInfo fun(char* ch){
    forw++;
    TokenInfo TkCurr;
    buffer_check();
    TkCurr.lexeme = (char *)malloc(strlen(ch) + 1);
    strcpy(TkCurr.lexeme, ch);
    TkCurr.line = lineno;
    return TkCurr;
}

TokenInfo getNextToken() // Function to get the next token from the input file
{
    curr = forw;
    buffer_check();
    TokenInfo TkCurr;
    char choice  = current_buffer[curr];
    switch (choice)
    {
    case EOF:

        TkCurr = fun("$");
        TkCurr.type = "$";
        return TkCurr;

    case ' ': case '\t':

        TkCurr = fun(" ");
        TkCurr.type = "TK_DELIM";
        return TkCurr;

    case '\n':
        lineno++;
        TkCurr = fun(" ");
        TkCurr.type = "TK_DELIM";
        return TkCurr;

    case '@':
        forw++;
        buffer_check();

        switch (current_buffer[forw]) {
            case '@':
                forw++;
                buffer_check();
                switch (current_buffer[forw]) {
                    case '@':
                        TkCurr = fun("@@@");
                        TkCurr.type = "TK_OR";
                        return TkCurr;

                    default:
                        TkCurr.lexeme = (char *)malloc(strlen("@@") + 1);
                        strcpy(TkCurr.lexeme, "@@");
                        TkCurr.line = lineno;
                        TkCurr.type = "TK_ERROR_PATTERN";
                        return TkCurr;
                }
                break;

            default:
                TkCurr.lexeme = (char *)malloc(strlen("@") + 1);
                strcpy(TkCurr.lexeme, "@");
                TkCurr.line = lineno;
                TkCurr.type = "TK_ERROR_SYMBOL";
                return TkCurr;
        }


    case '&':
        forw++;
        buffer_check();

        switch (current_buffer[forw]) {
            case '&':
                forw++;
                buffer_check();
                switch (current_buffer[forw]) {
                    case '&':
                        TkCurr = fun("&&&");
                        TkCurr.type = "TK_AND";
                        return TkCurr;

                    default:
                        TkCurr.lexeme = (char *)malloc(strlen("&&") + 1);
                        strcpy(TkCurr.lexeme, "&&");
                        TkCurr.line = lineno;
                        TkCurr.type = "TK_ERROR_PATTERN";
                        return TkCurr;
                }
                break;

            default:
                TkCurr.lexeme = (char *)malloc(strlen("&") + 1);
                strcpy(TkCurr.lexeme, "&");
                TkCurr.line = lineno;
                TkCurr.type = "TK_ERROR_SYMBOL";
                return TkCurr;
        }



    case'*':

        TkCurr = fun("*");
        TkCurr.type = "TK_MUL";
        return TkCurr;


    case '/':

        TkCurr = fun("/");
        TkCurr.type = "TK_DIV";
        return TkCurr;


    case '~':

        TkCurr = fun("~");
        TkCurr.type = "TK_NOT";
        return TkCurr;

    case '-':

        TkCurr = fun("-");
        TkCurr.type = "TK_MINUS";
        return TkCurr;


    case '+':

        TkCurr = fun("+");
        TkCurr.type = "TK_PLUS";
        return TkCurr;


    case ')':

        TkCurr = fun(")");
        TkCurr.type = "TK_CL";
        return TkCurr;


    case '(':

        TkCurr = fun("(");
        TkCurr.type = "TK_OP";
        return TkCurr;

    case '.':
        TkCurr = fun(".");
        TkCurr.type = "TK_DOT";
        return TkCurr;


    case ':' :
        TkCurr = fun(":");
        TkCurr.type = "TK_COLON";
        return TkCurr;

    case ';':
        TkCurr = fun(";");
        TkCurr.type = "TK_SEM";
        return TkCurr;

    case ',':
        TkCurr = fun(",");
        TkCurr.type = "TK_COMMA";
        return TkCurr;


    case ']':
        TkCurr = fun("]");
        TkCurr.type = "TK_SQR";
        return TkCurr;


    case '[':
        TkCurr = fun("[");
        TkCurr.type = "TK_SQL";
        return TkCurr;

    case '%':

        TkCurr = fun("%");
        TkCurr.type = "TK_COMMENT";
        while (current_buffer[forw] != '\n')
        {
            if (current_buffer[forw] == '\0')
            {
                break;
            }
            forw++;
            buffer_check();
        }
        return TkCurr;


    case '=':

        forw++;
        buffer_check();
        if (current_buffer[forw] == '=')
        {
            TkCurr = fun("==");
            TkCurr.type = "TK_EQ";
            return TkCurr;
        }
        else
        {
            TkCurr.lexeme = (char *)malloc(strlen("=") + 1);
            strcpy(TkCurr.lexeme, "=");
            TkCurr.line = lineno;
            TkCurr.type = "TK_ERROR_SYMBOL";
            return TkCurr;
        }


    case '!':

        forw++;
        buffer_check();
        if (current_buffer[forw] == '=')
        {
            TkCurr = fun("!=");
            TkCurr.type = "TK_NE";
            return TkCurr;
        }
        else
        {
            TkCurr.lexeme = (char *)malloc(strlen("!") + 1);
            strcpy(TkCurr.lexeme, "!");
            TkCurr.line = lineno;
            TkCurr.type = "TK_ERROR_SYMBOL";
            return TkCurr;
        }


    case '>':

        forw++;
        buffer_check();
        if (current_buffer[forw] == '=')
        {
            TkCurr = fun(">=");
            TkCurr.type = "TK_GE";
            return TkCurr;
        }
        else
        {
            TkCurr.lexeme = (char *)malloc(strlen(">") + 1);
            strcpy(TkCurr.lexeme, ">");
            TkCurr.line = lineno;
            TkCurr.type = "TK_GT";
            return TkCurr;
        }


    case '<':
        forw++;
        buffer_check();

        switch (current_buffer[forw]) {
            case '=':
                TkCurr = fun("<=");
                TkCurr.type = "TK_LE";
                return TkCurr;

            case '-':
                forw++;
                buffer_check();
                switch (current_buffer[forw]) {
                    case '-':
                        forw++;
                        buffer_check();
                        switch (current_buffer[forw]) {
                            case '-':
                                TkCurr = fun("<---");
                                TkCurr.type = "TK_ASSIGNOP";
                                return TkCurr;

                            default:
                                TkCurr.lexeme = (char *)malloc(strlen("<--") + 1);
                                strcpy(TkCurr.lexeme, "<--");
                                TkCurr.line = lineno;
                                TkCurr.type = "TK_ERROR_ASSIGNOP";
                                return TkCurr;
                        }
                        break;

                    default:
                        forw--;
                        TkCurr.lexeme = (char *)malloc(strlen("<") + 1);
                        strcpy(TkCurr.lexeme, "<");
                        TkCurr.line = lineno;
                        TkCurr.type = "TK_LT";
                        return TkCurr;
                }
                break;

            default:
                TkCurr.lexeme = (char *)malloc(strlen("<") + 1);
                strcpy(TkCurr.lexeme, "<");
                TkCurr.line = lineno;
                TkCurr.type = "TK_LT";
                return TkCurr;
        }


    case '#':
        forw++;
        buffer_check();
        int counter = 0;

        switch (current_buffer[forw]) {
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
                counter++;
                forw++;
                buffer_check();

                while (current_buffer[forw] >= 'a' && current_buffer[forw] <= 'z') {
                    counter++;
                    forw++;
                    buffer_check();
                }

                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                strncpy(TkCurr.lexeme, current_buffer + curr, forw - curr);
                TkCurr.lexeme[forw - curr] = '\0';  // Null-terminate the string
                TkCurr.line = lineno;

                TkCurr.type = (counter <= 20) ? "TK_RUID" : "TK_ERROR_SIZE20";
                return TkCurr;

            default:
                TkCurr.lexeme = (char *)malloc(strlen("#") + 1);
                strcpy(TkCurr.lexeme, "#");
                TkCurr.line = lineno;
                TkCurr.type = "TK_ERROR_SYMBOL";
                return TkCurr;
        }



    case '_':
        forw++;
        buffer_check();
        int counter1 = 0;

        switch (current_buffer[forw]) {
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
                counter1++;
                forw++;
                buffer_check();

                while ((current_buffer[forw] >= 'a' && current_buffer[forw] <= 'z') ||
                       (current_buffer[forw] >= 'A' && current_buffer[forw] <= 'Z')) {
                    counter1++;
                    forw++;
                    buffer_check();
                }

                while (current_buffer[forw] >= '0' && current_buffer[forw] <= '9') {
                    counter1++;
                    forw++;
                    buffer_check();
                }

                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                strncpy(TkCurr.lexeme, current_buffer + curr, forw - curr);
                TkCurr.lexeme[forw - curr] = '\0';  // Null-terminate the string
                TkCurr.line = lineno;

                if (counter1 <= 30) {
                    TkCurr.type = (strcmp(TkCurr.lexeme, "_main") == 0) ? "TK_MAIN" : "TK_FUNID";
                    return TkCurr;
                } else {
                    TkCurr.type = "TK_ERROR_SIZE30";
                    return TkCurr;
                }

            default:
                TkCurr.lexeme = (char *)malloc(strlen("_") + 1);
                strcpy(TkCurr.lexeme, "_");
                TkCurr.line = lineno;
                TkCurr.type = "TK_ERROR_SYMBOL";
                return TkCurr;
        }



    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':{
        int counter2 = 0;
        if ((current_buffer[curr] >= 'b') && (current_buffer[curr] <= 'd'))
        {
            forw++;
            counter2++;
            buffer_check();
            if ((current_buffer[forw] >= 'a') && (current_buffer[forw] <= 'z'))
            {
                counter2++;
                forw++;
                buffer_check();
                while ((current_buffer[forw] >= 'a') && (current_buffer[forw] <= 'z'))
                {
                    counter2++;
                    forw++;
                    buffer_check();
                }
                if (counter2 <= 20)
                {
                    TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                    strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);

                    TkCurr.line = lineno;
                    const char* type = searchLL(ll, TkCurr.lexeme);
                    if (strcmp(type, "TK_ERROR_PATTERN")==0)
                        TkCurr.type = "TK_FIELDID";
                    else
                    {
                        TkCurr.type = type;
                    }
                    return TkCurr;
                }
                else
                {
                    TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                    strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                    TkCurr.line = lineno;
                    TkCurr.type = "TK_ERROR_SIZE20";
                    return TkCurr;
                }
            }
            else if ((current_buffer[forw] >= '2') && (current_buffer[forw] <= '7'))
            {
                counter2++;
                forw++;
                buffer_check();
                while ((current_buffer[forw] >= 'b') && (current_buffer[forw] <= 'd'))
                {
                    counter2++;
                    forw++;
                    buffer_check();
                }
                while ((current_buffer[forw] >= '2') && (current_buffer[forw] <= '7'))
                {
                    counter2++;
                    forw++;
                    buffer_check();
                }
                if (counter2 <= 20)
                {
                    TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                    strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                    TkCurr.line = lineno;
                    TkCurr.type = "TK_ID";
                    return TkCurr;
                }
                else
                {
                    TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                    strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                    TkCurr.line = lineno;
                    TkCurr.type = "TK_ERROR_SIZE20";
                    return TkCurr;
                }
            }
            else
            {
                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                TkCurr.line = lineno;
                const char* type = searchLL(ll, TkCurr.lexeme);
                if (strcmp(type, "TK_ERROR_PATTERN")==0)
                    TkCurr.type = "TK_FIELDID";
                else
                {
                    TkCurr.type = type;
                }
                return TkCurr;
            }
        }
        else
        {
            counter2++;
            forw++;
            buffer_check();
            while ((current_buffer[forw] >= 'a') && (current_buffer[forw] <= 'z'))
            {
                counter2++;
                forw++;
                buffer_check();
            }
            if (counter2 <= 20)
            {
                TkCurr.lexeme = (char *)malloc(forw - curr + 2);
                strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                TkCurr.line = lineno;
                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                if (TkCurr.lexeme != NULL)
                {
                    strncpy(TkCurr.lexeme, current_buffer + curr, forw - curr);
                    TkCurr.lexeme[forw - curr] = '\0'; // Ensure null-termination
                }
                const char* type = searchLL(ll, TkCurr.lexeme);
                if (strcmp(type, "TK_ERROR_PATTERN")==0)
                    TkCurr.type = "TK_FIELDID";
                else
                {
                    TkCurr.type = type;
                }
                return TkCurr;
            }
            else
            {
                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                TkCurr.line = lineno;
                TkCurr.type = "TK_ERROR_SIZE20";
                return TkCurr;
            }
        }
    }

    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':{

        forw++;
        buffer_check();
        while ((current_buffer[forw] >= '0') && (current_buffer[forw] <= '9'))
        {
            forw++;
            buffer_check();
        }
        if (current_buffer[forw] == '.')
        {
            forw++;
            buffer_check();
            if ((current_buffer[forw] >= '0') && (current_buffer[forw] <= '9'))
            {
                forw++;
                buffer_check();
                if ((current_buffer[forw] >= '0') && (current_buffer[forw] <= '9'))
                {
                    forw++;
                    buffer_check();
                    if (current_buffer[forw] == 'E' || current_buffer[forw] == 'e')
                    {
                        forw++;
                        buffer_check();
                        if (current_buffer[forw] == '+' || current_buffer[forw] == '-')
                        {
                            forw++;
                            buffer_check();
                        }
                        if ((current_buffer[forw] >= '0') && (current_buffer[forw] <= '9'))
                        {
                            forw++;
                            buffer_check();
                            if ((current_buffer[forw] >= '0') && (current_buffer[forw] <= '9'))
                            {
                                forw++;
                                buffer_check();
                                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                                strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                                TkCurr.line = lineno;
                                TkCurr.type = "TK_RNUM";
                                return TkCurr;
                            }
                            else
                            {
                                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                                strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                                TkCurr.line = lineno;
                                TkCurr.type = "TK_ERROR_PATTERN";
                                return TkCurr;
                            }
                        }
                        else
                        {
                            TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                            strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                            TkCurr.line = lineno;
                            TkCurr.type = "TK_ERROR_PATTERN";
                            return TkCurr;
                        }
                    }
                    else
                    {
                        TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                        strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                        TkCurr.line = lineno;
                        TkCurr.type = "TK_RNUM";
                        return TkCurr;
                    }
                }
                else
                {
                    TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                    strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                    TkCurr.line = lineno;
                    TkCurr.type = "TK_ERROR_PATTERN";
                    return TkCurr;
                }
            }
            else
            {
                forw--;
                TkCurr.lexeme = (char *)malloc(forw - curr + 1);
                strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
                TkCurr.line = lineno;
                TkCurr.type = "TK_NUM";
                return TkCurr;
            }
        }
        else
        {
            TkCurr.lexeme = (char *)malloc(forw - curr + 1);
            strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
            TkCurr.line = lineno;
            TkCurr.type = "TK_NUM";
            return TkCurr;
        }
    }    
    }
    forw++;
    buffer_check();
    TkCurr.type = "TK_ERROR_SYMBOL";
    TkCurr.lexeme = (char *)malloc(forw - curr + 1);
    strncpy(TkCurr.lexeme, current_buffer + (curr), forw - curr);
    TkCurr.line = lineno;
    return TkCurr;
}

void printFileContents(FILE * fp) {
    if (fp == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[1024];
    size_t read;

    while ((fgets(line, sizeof(line), fp)) != NULL) {
        printf("%s", line);
    }

    // free(line);
}


void removeComments(char *testcaseFile) // Function to remove comments from the input file
{
    FILE *inputFile = fopen(testcaseFile, "r"); // Open the input file
    FILE *outputFile = fopen("cleanFile.txt", "w"); // Create a new file to store the contents of the input file without comments

    if (inputFile == NULL || outputFile == NULL)
    {
        printf("Error opening files.\n");
        return;
    }

    char line[1024];
    size_t charactersRead;

    while ((fgets(line, sizeof(line), inputFile)) != NULL)
    {
        char *commentPos = strchr(line, '%');
        if (commentPos != NULL)
        {
            *commentPos = '\n';
            *(commentPos + 1) = '\0';
        }
        fprintf(outputFile, "%s", line);
    }
    fclose(outputFile);
    outputFile = fopen("cleanFile.txt", "r"); // Open the file in read mode
    printFileContents(outputFile);
    
    // free(line);
    fclose(inputFile);
    fclose(outputFile);
}

void initialiseInputStreams(char * fileName){
    fp = fopen(fileName, "r");
    current_buffer = (char *)malloc(sizeof(char) * 512); // Allocate memory for the buffer
    next_buffer = (char *)malloc(sizeof(char) * 512);   // Allocate memory for the buffer

    fp = getStream(fp, 0); // Fill the buffer with the first 512 characters
    if (fp == NULL)
        printf("Error in opening the file\n");
    lineno = 1;                                          // Initialize the line number to 1
}

void printLexemesinFile(char *fileName){ // Function to print the lexemes and tokens

    initialiseInputStreams(fileName);

    FILE *new = fopen("lexemeToParser.txt", "w"); // or "a" for appending
    if (new == NULL) {
        perror("Failed to open file");
        return; // or handle the error as necessary
    }
    TokenInfo printToken; // Variable to store the token to be printed
    while (feof(fp) == 0)
    {
        printToken = getNextToken();
        if (strcmp(printToken.type, "TK_ERROR_SYMBOL")==0)
        {
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SYMBOL", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_DELIM")==0)
            continue;
        else if (strcmp(printToken.type, "TK_ERROR_PATTERN")==0){
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_PATTERN", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_ASSIGNOP")==0){
            char temp[70];
            strcpy(temp, "Error: Wrong assignment operator '<--' found, expected '<---'");
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_ASSIGNOP", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE20")==0){
            char temp[70];
            strcpy(temp, "Error: Variable Identifier is longer than the prescribed length of 20");
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE20", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE30")==0){
            char temp[80];
            strcpy(temp, "Error: Function/Record Identifier is longer than the prescribed length of 30");
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE30", printToken.line);
        }
        else
        {
            if(strcmp(printToken.lexeme, "")==0) continue;
            if(strcmp(printToken.type, "TK_COMMENT")==0){
                continue;
            }
            else{
                fprintf(new, "%s,%s,%d\n", printToken.lexeme, printToken.type, printToken.line);
            }
        }
    }
    forw = curr = 0;
    while ((forw < char_read))
    {
        printToken = getNextToken();
        if (strcmp(printToken.type, "TK_ERROR_SYMBOL")==0)
        {
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SYMBOL", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_DELIM")==0)
            continue;
        else if (strcmp(printToken.type, "TK_ERROR_PATTERN")==0){
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_PATTERN", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_ASSIGNOP")==0){
            char temp[70];
            strcpy(temp, "Error: Wrong assignment operator '<--' found, expected '<---'");
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_ASSIGNOP", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE20")==0){
            char temp[70];
            strcpy(temp, "Error: Variable Identifier is longer than the prescribed length of 20");
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE20", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE30")==0){
            char temp[80];
            strcpy(temp, "Error: Function/Record Identifier is longer than the prescribed length of 30");
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE30", printToken.line);
        }
        else
        {
            if(strcmp(printToken.type, "TK_COMMENT")==0){
                continue;
            }
            else{
                fprintf(new, "%s,%s,%d\n", printToken.lexeme, printToken.type, printToken.line);
            }
        }
    }
    fprintf(new, "$,$,%d\n", printToken.line);
    fclose(new);
}

void printLexemes(char *fileName){ // Function to print the lexemes and tokens

    initialiseInputStreams(fileName);

    FILE *new = fopen("lexemeToParser.txt", "w"); // or "a" for appending
    if (new == NULL) {
        perror("Failed to open file");
        return; // or handle the error as necessary
    }
    TokenInfo printToken; // Variable to store the token to be printed
    while (feof(fp) == 0)
    {
        printToken = getNextToken();
        if (strcmp(printToken.type, "TK_ERROR_SYMBOL")==0)
        {
            printf("Line no. %-8d | \033[31mError: Unknown Symbol <%s>\033[0m\n", printToken.line, printToken.lexeme);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SYMBOL", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_DELIM")==0)
            continue;
        else if (strcmp(printToken.type, "TK_ERROR_PATTERN")==0){
            printf("\033[31mLine no. %-8d | Error: Unknown Pattern <%s>\033[0m\n", printToken.line, printToken.lexeme);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_PATTERN", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_ASSIGNOP")==0){
            char temp[70];
            strcpy(temp, "Error: Wrong assignment operator '<--' found, expected '<---'");
            printf("\033[31mLine no. %-8d | %20s\033[0m\n", printToken.line, temp);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_ASSIGNOP", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE20")==0){
            char temp[70];
            strcpy(temp, "Error: Variable Identifier is longer than the prescribed length of 20");
            printf("\033[31mLine no. %-8d | %40s\033[0m\n", printToken.line, temp);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE20", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE30")==0){
            char temp[80];
            strcpy(temp, "Error: Function/Record Identifier is longer than the prescribed length of 30");
            printf("\033[31mLine no. %-8d | %40s\033[0m \n", printToken.line, temp);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE30", printToken.line);
        }
        else
        {
            if(strcmp(printToken.lexeme, "")==0) continue;
            printf("Line no. %-8d | Lexeme %-25s | Token %-40s\n", printToken.line, printToken.lexeme, printToken.type);
            if(strcmp(printToken.type, "TK_COMMENT")==0){
                continue;
            }
            else{
                fprintf(new, "%s,%s,%d\n", printToken.lexeme, printToken.type, printToken.line);
            }
        }
    }
    forw = curr = 0;
    while ((forw < char_read))
    {
        printToken = getNextToken();
        if (strcmp(printToken.type, "TK_ERROR_SYMBOL")==0)
        {
            printf("\033[31mLine no. %-8d | Error: Unknown Symbol <%s>\033[0m\n", printToken.line, printToken.lexeme);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SYMBOL", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_DELIM")==0)
            continue;
        else if (strcmp(printToken.type, "TK_ERROR_PATTERN")==0){
            printf("\033[31mLine no. %-8d | Error: Unknown Pattern <%s>\033[0m\n", printToken.line, printToken.lexeme);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_PATTERN", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_ASSIGNOP")==0){
            char temp[70];
            strcpy(temp, "Error: Wrong assignment operator '<--' found, expected '<---'");
            printf("\033[31mLine no. %-8d | %20s\033[0m\n", printToken.line, temp);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_ASSIGNOP", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE20")==0){
            char temp[70];
            strcpy(temp, "Error: Variable Identifier is longer than the prescribed length of 20");
            printf("\033[31mLine no. %-8d | %20s\033[0m\n", printToken.line, temp);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE20", printToken.line);
        }
        else if (strcmp(printToken.type, "TK_ERROR_SIZE30")==0){
            char temp[80];
            strcpy(temp, "Error: Function/Record Identifier is longer than the prescribed length of 30");
            printf("\033[31mLine no. %-8d | %20s\033[0m \n", printToken.line, temp);
            fprintf(new, "%s,%s,%d\n", printToken.lexeme, "TK_ERROR_SIZE30", printToken.line);
        }
        else
        {
            printf("Line no. %-8d | Lexeme %-25s | Token %-40s\n", printToken.line, printToken.lexeme, printToken.type);
            if(strcmp(printToken.type, "TK_COMMENT")==0){
                continue;
            }
            else{
                fprintf(new, "%s,%s,%d\n", printToken.lexeme, printToken.type, printToken.line);
            }
        }
    }
    fprintf(new, "$,$,%d\n", printToken.line);
    fclose(new);
}