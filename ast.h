#ifndef AST_H
#define AST_H
#include "lexer.h"

typedef enum {
    NODE_NUMBER, NODE_STRING, NODE_IDENT, NODE_BOOL, NODE_NIL,
    NODE_FLOAT,            // ← новое
    NODE_BINOP, NODE_LET, NODE_ASSIGN, NODE_FN_DEF, NODE_FN_CALL,
    NODE_IF, NODE_WHILE, NODE_FOR, NODE_RETURN, NODE_PRINT,
    NODE_BLOCK, NODE_LIST, NODE_DICT,        // ← словарь
    NODE_TRY_CATCH, NODE_THROW,              // ← try/catch
    NODE_BREAK, NODE_CONTINUE,               // ← break/continue
    NODE_SWITCH, NODE_CASE,                  // ← switch/case
    NODE_LAMBDA,                             // ← лямбда
    NODE_IMPORT                              // ← импорт
} NodeType;

typedef struct Node {
    NodeType type;
    int   int_val;
    double float_val;      // ← для дробных чисел
    char* str_val;
    TokenType op;
    
    struct Node* left;
    struct Node* right;
    struct Node* value;
    struct Node* cond;
    struct Node* body;
    struct Node* else_body;
    struct Node* catch_body;   // ← для try/catch
    struct Node** params;
    int param_count;
    struct Node* fn;
    struct Node** args;
    int arg_count;
    struct Node** stmts;
    int count;
    struct Node** items;
    int item_count;
    struct Node* iterable;
    struct Node* key;          // ← ключ словаря
    
    // Для switch
    struct Node** cases;
    struct Node* default_case;
    int case_count;
} Node;

// Конструкторы
Node* node_number(int v);
Node* node_float(double v);                  // ← новое
Node* node_string(char* v);
Node* node_ident(char* v);
Node* node_bool(int v);
Node* node_nil(void);
Node* node_binop(TokenType op, Node* l, Node* r);
Node* node_let(char* name, Node* val);
Node* node_assign(char* name, Node* val);
Node* node_fn_def(char* name, Node** params, int n, Node* body);
Node* node_fn_call(Node* f, Node** args, int n);
Node* node_if(Node* cond, Node* body, Node* else_body);
Node* node_while(Node* cond, Node* body);
Node* node_for(char* name, Node* iter, Node* body);
Node* node_return(Node* val);
Node* node_print(Node* val);
Node* node_block(Node** stmts, int n);
Node* node_list(Node** items, int n);
Node* node_dict(Node** keys, Node** values, int n);     // ← новое
Node* node_try_catch(Node* body, Node* catch_body);     // ← новое
Node* node_throw(Node* msg);                            // ← новое
Node* node_break(void);                                 // ← новое
Node* node_continue(void);                              // ← новое
Node* node_switch(Node* expr, Node** cases, Node* def, int n); // ← новое
Node* node_case(Node* value, Node* body);               // ← новое
Node* node_lambda(Node** params, int n, Node* body);    // ← новое
Node* node_import(char* name);                          // ← новое
#endif