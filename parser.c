#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "parser.h"
#include "parserDef.h"
#include "lexer.h"

token_array tokens;
non_terminal_array non_terminals;
parse_table table;
treenode root;

stack push_stack(stack s, treenode t) {
    stack new_s = (stack) malloc(sizeof(*new_s));
    new_s->head = t;
    new_s->next = s;
    return new_s;
}

stack pop_stack(stack s) {
    if (s == NULL){
        perror("popping from an empty stack");
        return NULL;
    }
    stack new_s = s->next;
    free(s);
    return new_s;
}

int is_token(char * buffer){
    if (buffer[0] != '<') return 1;
    return 0;
}

void add_tokens(char * buffer){
    if (tokens->curr >= tokens->size) {
        tokens->size = 2 * tokens->size + 1;
        tokens->data = (token *) realloc(tokens->data, tokens->size * sizeof(token));
        if (tokens->data == NULL) perror("Error reallocating memory\n");
    }
    tokens->data[tokens->curr] = (token) malloc(sizeof(*tokens->data[tokens->curr]));
    tokens->data[tokens->curr]->ind = tokens->curr;
    // printf("added token %s\n", buffer);
    strcpy(tokens->data[tokens->curr]->identity, buffer);
    tokens->curr++;
    return;
}

void add_non_terminals(char * buffer){
    if (non_terminals->curr >= non_terminals->size) {
        non_terminals->size = 2 * non_terminals->size + 1;
        non_terminals->data = (non_terminal *) realloc(non_terminals->data, non_terminals->size * sizeof(non_terminal));
        if (non_terminals->data == NULL) perror("Error reallocating memory\n");
    }
    non_terminals->data[non_terminals->curr] = (non_terminal) malloc(sizeof(*non_terminals->data[non_terminals->curr]));
    non_terminals->data[non_terminals->curr]->ind = non_terminals->curr;
    // printf("added non terminal %s\n", buffer);
    strcpy(non_terminals->data[non_terminals->curr]->identity, buffer);
    non_terminals->data[non_terminals->curr]->firstset = NULL;
    non_terminals->data[non_terminals->curr]->followset = NULL;
    non_terminals->data[non_terminals->curr]->rules = (rule_array) malloc(sizeof(*non_terminals->data[non_terminals->curr]->rules));
    non_terminals->data[non_terminals->curr]->rules->data = NULL;
    non_terminals->data[non_terminals->curr]->rules->curr = 0;
    non_terminals->data[non_terminals->curr]->rules->size = 0;
    non_terminals->curr++;
    return;
}

void add_alphabet(char * buffer){
    if (is_token(buffer)) add_tokens(buffer);
    else add_non_terminals(buffer);
    return;
}

void create_alphabets_from_file(char * filename){
    tokens = (token_array) malloc(sizeof(*tokens));
    if (tokens == NULL) perror("Error allocating memory\n");
    tokens->data = NULL;
    tokens->size = 0;
    tokens->curr = 0;

    non_terminals = (non_terminal_array) malloc(sizeof(*non_terminals));
    if (non_terminals == NULL) perror("Error allocating memory\n");
    non_terminals->data = NULL;
    non_terminals->size = 0;
    non_terminals->curr = 0;

    FILE *file;
    char buffer[100];
    file = fopen(filename, "r");
    if (file == NULL) perror("Error opening file\n");
    while (fscanf(file, "%99s", buffer) == 1) add_alphabet(buffer);
    fclose(file);
    return;
}

void print_alphabets(){
    printf("TERMINALS:\n");
    for(int i = 0; i < tokens->curr; i++) printf(" %d.%s\n",i, tokens->data[i]->identity);
    printf("\nNON TERMINALS:\n");
    for(int i = 0; i < non_terminals->curr; i++) printf(" %d.%s\n",i, non_terminals->data[i]->identity);
    printf("\n");
}

token get_token(const char * buffer){
    for (int i = 0; i < tokens->curr; i++) {
        if (strcmp(tokens->data[i]->identity, buffer) == 0) {
            return tokens->data[i];
        }
    }
    perror("Token not found\n");
    printf("the violating token is: #%s#\n", buffer);
    return NULL;
}

non_terminal get_non_terminal(char * buffer){
    for (int i = 0; i < non_terminals->curr; i++) {
        if (strcmp(non_terminals->data[i]->identity, buffer) == 0) {
            return non_terminals->data[i];
        }
    }
    perror("Non-terminal not found\n");
    printf("violating non terminal: %s\n", buffer);
    return NULL;
}

void print_rule(rule r) {
    if (r == NULL){
        perror("rule without end cap");
        return;
    }
    if (r->head_token != NULL && r->head_token->ind == 0){
        printf("eps(end of rule)\n");
        return;
    }
    if (r->head_token != NULL) printf("%s->", r->head_token->identity);
    if (r->head_non_terminal != NULL) printf("%s->", r->head_non_terminal->identity);
    print_rule(r->next);
}

rule create_rule(token t, non_terminal nt, rule successor){
    rule r = (rule) malloc(sizeof(*r));
    r->head_token = t;
    r->head_non_terminal = nt;
    r->next = successor;
    return r;
}

rule parse_rule_string(char * buffer){
    if (buffer == NULL) return create_rule(tokens->data[0], NULL, NULL);
    char *first_word = strtok(buffer, " ");
    char *remaining_string = strtok(NULL, "\n");
  //  printf("first word: #%s#, remaining word: #%s#\n", first_word, remaining_string);
    if (strcmp(first_word, "->") == 0) return parse_rule_string(remaining_string);
    if (is_token(first_word)) return create_rule(get_token(first_word), NULL, parse_rule_string(remaining_string));
    else return create_rule(NULL, get_non_terminal(first_word), parse_rule_string(remaining_string));
}

void add_rule_to_non_terminal(rule r, non_terminal t){
    if (t->rules->curr >= t->rules->size) {
        t->rules->size = 2 * t->rules->size + 1;
        t->rules->data = (rule *) realloc(t->rules->data, t->rules->size * sizeof(rule));
        if (t->rules->data == NULL) perror("Error reallocating memory while adding\n");
    }
    t->rules->data[t->rules->curr] = r;
    t->rules->curr++;
}

void create_rules_from_file(char * filename){
    FILE *file;
    char buffer[1000];
    file = fopen(filename, "r");
    if (file == NULL) perror("Error opening file\n");
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
    //    printf("adding rule: %s", buffer);
        rule r = parse_rule_string(buffer);
        add_rule_to_non_terminal(r, get_non_terminal(r->head_non_terminal->identity));
    }
    fclose(file);
}

void print_rules(){
    printf("RULES: \n");
    for (int i = 0; i < non_terminals->curr; i++) {
        printf("%s:\n", non_terminals->data[i]->identity);
        for (int j = 0; j < non_terminals->data[i]->rules->curr; j++) {
            printf(" ");
            print_rule(non_terminals->data[i]->rules->data[j]);
        }
    }
    printf("\n");
    return;
}

void fill_first_set(non_terminal nt);
void fill_follow_set(non_terminal ntq);

void compute_first_set(non_terminal nt, rule r){
    if (r == NULL) return;
    if (r->head_token != NULL){
        nt->firstset[r->head_token->ind] = 1;
        return;
    }
    if (r->head_non_terminal->firstset == NULL) fill_first_set(r->head_non_terminal);
    for(int i = 1; i < tokens->curr; i++) nt->firstset[i] |= r->head_non_terminal->firstset[i];
    if (r->head_non_terminal->firstset[0]) compute_first_set(nt, r->next);
    return;
}

void fill_first_set(non_terminal nt){
    if (nt->firstset != NULL) return;
    nt->firstset = (int *) malloc(tokens->curr * sizeof(int));
    for(int i = 0; i < tokens->curr; i++) nt->firstset[i] = 0;
    for(int rule_index = 0; rule_index < nt->rules->curr; rule_index++) compute_first_set(nt, nt->rules->data[rule_index]->next);
    return;
}

void fill_first_sets(){
    for(int i = 0; i < non_terminals->curr; i++) fill_first_set(non_terminals->data[i]);
    return;
}

void print_first_sets(){
    printf("FIRST SETS: \n");
    for(int i = 0; i < non_terminals->curr; i++){
        printf(" %s: { ", non_terminals->data[i]->identity);
        for(int j = 0; j < tokens->curr; j++) if (non_terminals->data[i]->firstset[j]) printf("%s ", tokens->data[j]->identity);
        printf("}\n");
    }
    printf("\n");
    return;
}

void compute_follow_set(non_terminal par, rule curr_rule, int * temp){
    if (curr_rule == NULL) return;
    if (curr_rule->head_token != NULL){
        if (curr_rule->head_token->ind != 0){
            temp[curr_rule->head_token->ind] = 1;
        }
        else{
            if (par->followset == NULL) fill_follow_set(par);
            for(int ind = 1; ind < tokens->curr; ind++){
                temp[ind] |= par->followset[ind];
            }
        }
        return;
    }
    for(int i = 1; i < tokens->curr; i++){
        temp[i] |= curr_rule->head_non_terminal->firstset[i];
    }
    if (curr_rule->head_non_terminal->firstset[0]) compute_follow_set(par, curr_rule->next, temp);
}

void fill_follow_set(non_terminal ntq){
    if (ntq->followset != NULL) return;
    ntq->followset = (int *) malloc(tokens->curr * sizeof(int));
    for (int k = 0; k < tokens->curr; k++) ntq->followset[k] = 0;
    if (ntq->ind == 0) ntq->followset[1] = 1;
    for(int j = 0; j < non_terminals->curr; j++){
        non_terminal par = non_terminals->data[j];
        for(int r = 0; r < par->rules->curr; r++){
            rule curr_rule = par->rules->data[r]->next;
            while(curr_rule){
                while(curr_rule != NULL && curr_rule->head_non_terminal != ntq) curr_rule = curr_rule->next;
                if (curr_rule != NULL) curr_rule = curr_rule -> next;
                int * temp = (int *) malloc(tokens->curr * sizeof(int));
                for(int tempind = 0; tempind < tokens->curr; tempind++) temp[tempind] = 0;
                compute_follow_set(par, curr_rule, temp);
                for(int tempind = 0; tempind < tokens->curr; tempind++) ntq->followset[tempind] |= temp[tempind];
                free(temp);
            }
        }
    }   
}

void fill_follow_sets(){
    for(int i = 0; i < non_terminals->curr; i++) fill_follow_set(non_terminals->data[i]);
    return;
}

void print_follow_sets(){
    printf("FOLLOW SETS: \n");
    for(int i = 0; i < non_terminals->curr; i++){
        printf(" %s: { ", non_terminals->data[i]->identity);
        for(int j = 0; j < tokens->curr; j++) if (non_terminals->data[i]->followset[j]) printf("%s ", tokens->data[j]->identity);
        printf("}\n");
    }
    printf("\n");
    return;
}

void get_terminal_list_for_parse_table(non_terminal par, rule curr_rule, int * temp){
    if (curr_rule == NULL) return;
    if (curr_rule->head_token != NULL){
        if (curr_rule->head_token->ind != 0){
            temp[curr_rule->head_token->ind] = 1;
        }
        else{
            for(int ind = 1; ind < tokens->curr; ind++){
                temp[ind] |= par->followset[ind];
            }
        }
        return;
    }
    for(int i = 1; i < tokens->curr; i++){
        temp[i] |= curr_rule->head_non_terminal->firstset[i];
    }
    if (curr_rule->head_non_terminal->firstset[0]) get_terminal_list_for_parse_table(par, curr_rule->next, temp);
}

void create_parse_table(){
    table = (parse_table) malloc(sizeof(*table));
    table->grid = (rule**) malloc(non_terminals->curr * sizeof(*table->grid));
    for(int i = 0; i < non_terminals->curr; i++) table->grid[i] = (rule*) malloc(tokens->curr * sizeof(*table->grid[i]));
    for(int i = 0; i < non_terminals->curr; i++){
        for(int j = 0; j < tokens->curr; j++){
            table->grid[i][j] = NULL; 
        }
    }
    for(int nti = 0; nti  < non_terminals->curr; nti++){
        non_terminal nt = non_terminals->data[nti];
        for(int ri = 0; ri < nt->rules->curr; ri++){
            rule r = nt->rules->data[ri];
            int * temp = (int *) malloc(tokens->curr * sizeof(int));
            for(int ti = 0; ti < tokens->curr; ti++) temp[ti] = 0;
            get_terminal_list_for_parse_table(r->head_non_terminal, r->next, temp);
            for(int ti = 0; ti < tokens->curr; ti++){
                if (temp[ti]){
                    if (table->grid[nti][ti] != NULL){
                        perror("multiple rules for a single grid value");
                        return;
                    }
                    table->grid[nti][ti] = r;
                }
            }
        } 
    }    
}

void print_parse_table() {
    printf("PARSE TABLE:\n\n");
    for (int i = 0; i < non_terminals->curr; i++) {
        printf("----------%s---------- \n", non_terminals->data[i]->identity);
        for (int j = 0; j < tokens->curr; j++) {
            if (table->grid[i][j] != NULL) {
                printf("%s :: ", tokens->data[j]->identity);
                print_rule(table->grid[i][j]);
            }
        }
        printf("\n");
    }
}

treenode create_treenode(token t, non_terminal nt, char * line_number, char * lexeme, non_terminal parent) {
    treenode new_node = (treenode) malloc(sizeof(*new_node));
    if (new_node == NULL) {
        perror("Error allocating memory for tree node\n");
        return NULL;
    }
    new_node->head_token = t;
    new_node->head_non_terminal = nt;
    new_node->parent = parent;
    if (new_node->head_token != NULL){
        new_node->line_number = strdup(line_number);  
        new_node->lexeme = strdup(lexeme);              
    }
    else{
        new_node->line_number = strdup("-1");
        new_node->lexeme = strdup("----");
    }
    for (int i = 0; i < 16; i++) {
        new_node->children[i] = NULL;
    }
    return new_node;
}

void print_tree_node(treenode t){
    if (t == NULL) printf("printing null treenode");
    else if (t->head_token != NULL) printf("%s %s %s %s", t->head_token->identity, t->line_number, t->lexeme, (t->parent)?t->parent->identity:"ROOT");
    else if (t->head_non_terminal != NULL) printf("%s %s %s %s", t->head_non_terminal->identity, t->line_number, t->lexeme, (t->parent)?t->parent->identity:"ROOT");
    else printf("uninitialised treenode");
    printf("\n");
}

void printTree(treenode node, const char *prefix, int isLast) {
    if (node == NULL) return;
    printf("%s%s", prefix, (isLast ? "`-- " : "|-- "));
    print_tree_node(node);
    char newPrefix[256];
    snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, (isLast ? "    " : "|   "));
    int childCount = 0;
    for (int i = 0; i < 16; i++) if (node->children[i] != NULL) childCount++;
    int printed = 0;
    for (int i = 0; i < 16; i++) {
        if (node->children[i] != NULL) {
            printed++;
            int lastChild = (printed == childCount);
            printTree(node->children[i], newPrefix, lastChild);
        }
    }
}

void printTreeInOrder(treenode node){
    if (node == NULL) return;
    printTreeInOrder(node->children[0]);
    print_tree_node(node);
    for(int i = 1; i < 16; i++) printTreeInOrder(node->children[i]);
    return;
}

void print_stack(stack s){
    // stack temp = s;
    // printf("STACK:\n");
    // while (temp != NULL) {
    //     print_tree_node(temp->head);
    //     temp = temp->next;
    // }
    // printf("\n");
}

void parse_input_get_tree(){
    char * filename = "lexemeToParser.txt";
    root = create_treenode(NULL, non_terminals->data[0], "-1", "----", NULL);
    stack s = push_stack(NULL, create_treenode(tokens->data[1], NULL, "-1", "----", NULL));
    s = push_stack(s, root);
    print_stack(s);
    FILE *file;
    char buffer[1000];
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file\n");
        return;
    }
    while (fscanf(file, "%999s", buffer) == 1) {
        char *lexeme = strtok(buffer, ",");
        char *token_name = strtok(NULL, ",");
        char *line_number = strtok(NULL, "\n");
        if (lexeme == NULL || token_name == NULL || line_number == NULL) {
            perror("Error parsing input file\n");
            fclose(file);
            return;
        }
    //    printf("Lexeme: %s, Token: %s, Line: %s\n", lexeme, token_name, line_number);
        while (s->head->head_token == NULL) {
            non_terminal nt = s->head->head_non_terminal;
            treenode s_top = s->head;
            s = pop_stack(s);
            rule r = table->grid[nt->ind][get_token(token_name)->ind];
            r = r->next;
            int childind = 0;
            while (r->head_token == NULL || r->head_token->ind != 0){
                s_top->children[childind++] = create_treenode(r->head_token, r->head_non_terminal, line_number, lexeme, s_top->head_non_terminal);
                r = r->next;
            }
            for(int i = childind - 1; i >= 0; i--){
                s = push_stack(s, s_top->children[i]);
            } 
        }
        if (s->head->head_token != NULL) {
    //        printf("evaluating token %s, against stack head %s\n", token_name, s->head->head_token->identity);
            print_stack(s);
            if (strcmp(s->head->head_token->identity, token_name) != 0) {
                perror("cannot match the grammar to the token input");
                fclose(file);
                return;
            } 
            else {
                if (strcmp(token_name, "$") == 0) {
                    printf("PARSING SUCCESSFUL!!\n\n");
                    fclose(file);
                    return;
                }
                else{
                    s->head->line_number = strdup(line_number);
                    s->head->lexeme = strdup(lexeme);
                }
                s = pop_stack(s);
            }
        }
        else{
            perror("inaccessible segment of code reached!");
            fclose(file);
            return;
        }
    }
    fclose(file);
    return;
}


void initialiseParser(){
    char * alphabet_file = "project_language/alphabets.txt";
    char * rule_file = "project_language/rules.txt";
    char * stream_file = "project_language/stream.txt";

    create_alphabets_from_file(alphabet_file);
  //  print_alphabets();

    create_rules_from_file(rule_file);
  //  print_rules();

    fill_first_sets();
  //  print_first_sets();

    fill_follow_sets();
 //   print_follow_sets();

    create_parse_table();
 //  print_parse_table();

    // parse_input_get_tree(stream_file);  
    // printTree(root, "", 1);
}

// int main(){
//     initialiseParser();
//     return 0;
// }