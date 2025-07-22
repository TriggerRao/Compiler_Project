#ifndef PARSERDEF_H
#define PARSERDEF_H

typedef struct token_struct * token;
typedef struct non_terminal_struct * non_terminal;
typedef struct rule_struct * rule;
typedef struct token_array_struct * token_array;
typedef struct non_terminal_array_struct * non_terminal_array;
typedef struct rule_array_struct  * rule_array;
typedef struct parse_table_struct * parse_table;
typedef struct tree_node_struct * treenode;
typedef struct stack_node_struct * stack;

struct token_struct{
    int ind; 
    char identity[100];
};

struct non_terminal_struct{
    int ind;
    char identity[100];
    int * firstset;
    int * followset;
    rule_array rules;
};

struct rule_struct{
    token head_token;
    non_terminal head_non_terminal;
    struct rule_struct * next;
};

struct token_array_struct{
    int curr, size;
    token * data;
};

struct non_terminal_array_struct{
    int curr, size;
    non_terminal * data;
};

struct rule_array_struct{
    int curr, size;
    rule * data;
};

struct parse_table_struct{
    int rows, cols;
    rule ** grid;
};

struct tree_node_struct{
    token head_token;
    non_terminal head_non_terminal;
    non_terminal parent;
    char * line_number;
    char * lexeme;
    treenode children[16];
};

struct stack_node_struct {
    treenode head;
    stack next;
};

extern token_array tokens;
extern non_terminal_array non_terminals;
extern parse_table table;
extern treenode root;

#endif