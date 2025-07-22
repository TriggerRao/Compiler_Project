#ifndef PARSER_H
#define PARSER_H

#include "parserDef.h"

stack push_stack(stack s, treenode t);
stack pop_stack(stack s);
int is_token(char * buffer);
void add_tokens(char * buffer);
void add_non_terminals(char * buffer);
void add_alphabet(char * buffer);
void create_alphabets_from_file(char * filename);
void print_alphabets();
token get_token(const char * buffer);
non_terminal get_non_terminal(char * buffer);
void print_rule(rule r);
rule create_rule(token t, non_terminal nt, rule successor);
rule parse_rule_string(char * buffer);
void add_rule_to_non_terminal(rule r, non_terminal t);
void create_rules_from_file(char * filename);
void print_rules();
void fill_first_set(non_terminal nt);
void fill_follow_set(non_terminal nt);
void compute_first_set(non_terminal nt, rule r);
void fill_first_sets();
void print_first_sets();
void compute_follow_set(non_terminal par, rule curr_rule, int * temp);
void fill_follow_sets();
void print_follow_sets();
void get_terminal_list_for_parse_table(non_terminal par, rule curr_rule, int * temp);
void create_parse_table();
void print_parse_table();
treenode create_treenode(token t, non_terminal nt, char * line_number, char * lexeme, non_terminal parent);
void print_tree_node(treenode t);
void printTree(treenode node, const char *prefix, int isLast);
void print_stack(stack s);
void parse_input_get_tree();
void initialiseParser();
void printTreeInOrder(treenode node);

#endif