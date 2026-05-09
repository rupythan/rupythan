#include "ast.h"

Node* node_number(int v) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_NUMBER; n->int_val = v; return n;
}
Node* node_float(double v) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_FLOAT; n->float_val = v; return n;
}
Node* node_string(char* v) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_STRING; n->str_val = v; return n;
}
Node* node_ident(char* v) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_IDENT; n->str_val = v; return n;
}
Node* node_bool(int v) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_BOOL; n->int_val = v; return n;
}
Node* node_nil(void) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_NIL; return n;
}
Node* node_binop(TokenType op, Node* l, Node* r) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_BINOP; n->op = op; n->left = l; n->right = r; return n;
}
Node* node_let(char* name, Node* val) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_LET; n->str_val = name; n->value = val; return n;
}
Node* node_assign(char* name, Node* val) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_ASSIGN; n->str_val = name; n->value = val; return n;
}
Node* node_fn_def(char* name, Node** params, int c, Node* body) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_FN_DEF; n->str_val = name;
    n->params = params; n->param_count = c; n->body = body; return n;
}
Node* node_fn_call(Node* f, Node** args, int c) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_FN_CALL; n->fn = f; n->args = args; n->arg_count = c; return n;
}
Node* node_if(Node* cond, Node* body, Node* else_body) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_IF; n->cond = cond; n->body = body; n->else_body = else_body; return n;
}
Node* node_while(Node* cond, Node* body) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_WHILE; n->cond = cond; n->body = body; return n;
}
Node* node_for(char* name, Node* iter, Node* body) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_FOR; n->str_val = name; n->iterable = iter; n->body = body; return n;
}
Node* node_return(Node* val) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_RETURN; n->value = val; return n;
}
Node* node_print(Node* val) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_PRINT; n->value = val; return n;
}
Node* node_block(Node** stmts, int c) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_BLOCK; n->stmts = stmts; n->count = c; return n;
}
Node* node_list(Node** items, int c) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_LIST; n->items = items; n->item_count = c; return n;
}
Node* node_dict(Node** keys, Node** values, int n) {
    Node* node = safe_alloc(sizeof(Node)); node->type = NODE_DICT;
    node->items = keys; node->params = values; node->item_count = n; return node;
}
Node* node_try_catch(Node* body, Node* catch_body) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_TRY_CATCH; n->body = body; n->catch_body = catch_body; return n;
}
Node* node_throw(Node* msg) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_THROW; n->str_val = msg->str_val; return n;
}
Node* node_break(void) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_BREAK; return n;
}
Node* node_continue(void) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_CONTINUE; return n;
}
Node* node_switch(Node* expr, Node** cases, Node* def, int c) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_SWITCH;
    n->value = expr; n->cases = cases; n->default_case = def; n->case_count = c; return n;
}
Node* node_case(Node* value, Node* body) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_CASE; n->value = value; n->body = body; return n;
}
Node* node_lambda(Node** params, int n, Node* body) {
    Node* node = safe_alloc(sizeof(Node)); node->type = NODE_LAMBDA;
    node->params = params; node->param_count = n; node->body = body; return node;
}
Node* node_import(char* name) {
    Node* n = safe_alloc(sizeof(Node)); n->type = NODE_IMPORT; n->str_val = name; return n;
}
