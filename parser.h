#ifndef PARSER_H
#define PARSER_H
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    int had_error;
    int panic_mode;
} Parser;

Node* parse(const char* source);
void parser_init(Parser* p, const char* src);
Node* parse_program(Parser* p);

#endif