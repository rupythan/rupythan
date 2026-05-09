#ifndef EVAL_H
#define EVAL_H
#include "ast.h"

typedef struct Env {
    char** names;
    Node** values;
    int count;
    struct Env* parent;
} Env;

Env* env_new(Env* parent);
void env_set(Env* e, const char* name, Node* value);
Node* env_get(Env* e, const char* name);
Node* eval(Node* node, Env* env);
void eval_program(Node* ast);
void print_help();

#endif
