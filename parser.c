#include "parser.h"

static Token advance(Parser* p) {
    p->previous = p->current;
    p->current = lexer_next(&p->lexer);
    while (p->current.type == 999) {
        p->previous = p->current;
        p->current = lexer_next(&p->lexer);
    }
    return p->previous;
}

static int check(Parser* p, TokenType t) { return p->current.type == t; }

static int match(Parser* p, TokenType t) {
    if (check(p, t)) { advance(p); return 1; }
    return 0;
}

static void parse_error(Parser* p, const char* msg) {
    if (p->panic_mode) return;
    p->panic_mode = 1;
    p->had_error = 1;
    fprintf(stderr, "💀 Ошибка в строке %d: %s\n", p->current.line, msg);
    fprintf(stderr, "   Найдено: %s\n", token_type_name(p->current.type));
}

static void expect(Parser* p, TokenType t, const char* msg) {
    if (match(p, t)) { p->panic_mode = 0; return; }
    parse_error(p, msg);
}

static int current_indent(Parser* p) {
    return p->lexer.col;
}

static Node* parse_expression(Parser* p);
static Node* parse_stmt(Parser* p);

// ============ PRIMARY ============
static Node* parse_primary(Parser* p) {
    if (match(p, TOK_NUMBER)) {
        // Проверяем, есть ли дробная часть
        if (p->previous.float_val != (double)p->previous.int_val || 
            (p->previous.str_val == NULL && p->previous.float_val != 0))
            return node_float(p->previous.float_val);
        return node_number(p->previous.int_val);
    }
    if (match(p, TOK_STRING)) return node_string(p->previous.str_val);
    if (match(p, TOK_TRUE))   return node_bool(1);
    if (match(p, TOK_FALSE))  return node_bool(0);
    if (match(p, TOK_NIL))    return node_nil();

    if (match(p, TOK_IDENT)) {
        Node* id = node_ident(p->previous.str_val);
        if (check(p, TOK_LPAREN)) {
            advance(p);
            Node** args = NULL; int n = 0;
            if (!match(p, TOK_RPAREN)) {
                args = safe_alloc(sizeof(Node*) * 32);
                args[n++] = parse_expression(p);
                while (match(p, TOK_COMMA))
                    args[n++] = parse_expression(p);
                expect(p, TOK_RPAREN, "ожидалась ')' после аргументов");
            }
            return node_fn_call(id, args, n);
        }
        return id;
    }

    if (match(p, TOK_LPAREN)) {
        Node* e = parse_expression(p);
        expect(p, TOK_RPAREN, "ожидалась ')'");
        return e;
    }

    // Список [1, 2, 3]
    if (match(p, TOK_LBRACKET)) {
        Node** items = NULL; int n = 0;
        if (!match(p, TOK_RBRACKET)) {
            items = safe_alloc(sizeof(Node*) * 256);
            items[n++] = parse_expression(p);
            while (match(p, TOK_COMMA))
                items[n++] = parse_expression(p);
            expect(p, TOK_RBRACKET, "ожидался ']'");
        }
        return node_list(items, n);
    }

    // Словарь {"ключ": значение}
    // ИСКАТЬ: // Словарь
    if (match(p, TOK_LBRACE)) {
        Node** keys = safe_alloc(sizeof(Node*) * 256);
        Node** values = safe_alloc(sizeof(Node*) * 256);
        int n = 0;
        if (!match(p, TOK_RBRACE)) {
            keys[n] = parse_expression(p);
            expect(p, TOK_COLON, "ожидалось ':' в словаре");
            values[n++] = parse_expression(p);
            while (match(p, TOK_COMMA)) {
                keys[n] = parse_expression(p);
                expect(p, TOK_COLON, "ожидалось ':' в словаре");
                values[n++] = parse_expression(p);
            }
            expect(p, TOK_RBRACE, "ожидалась '}'");
        }
        return node_dict(keys, values, n);
    }

    parse_error(p, "неизвестное выражение");
    return node_nil();
}

// ============ УНАРНЫЕ ОПЕРАЦИИ ============
static Node* parse_unary(Parser* p) {
    if (match(p, TOK_MINUS))
        return node_binop(TOK_STAR, node_number(-1), parse_primary(p));
    return parse_primary(p);
}

// ============ УМНОЖЕНИЕ/ДЕЛЕНИЕ ============
static Node* parse_factor(Parser* p) {
    Node* left = parse_unary(p);
    while (match(p, TOK_STAR) || match(p, TOK_SLASH)) {
        TokenType op = p->previous.type;
        Node* right = parse_unary(p);
        left = node_binop(op, left, right);
    }
    return left;
}

// ============ СЛОЖЕНИЕ/ВЫЧИТАНИЕ ============
static Node* parse_term(Parser* p) {
    Node* left = parse_factor(p);
    while (match(p, TOK_PLUS) || match(p, TOK_MINUS)) {
        TokenType op = p->previous.type;
        Node* right = parse_factor(p);
        left = node_binop(op, left, right);
    }
    return left;
}

// ============ СРАВНЕНИЕ ============
static Node* parse_comparison(Parser* p) {
    Node* left = parse_term(p);
    if (match(p, TOK_EQEQ) || match(p, TOK_NEQ) || match(p, TOK_LT) ||
        match(p, TOK_GT) || match(p, TOK_LEQ) || match(p, TOK_GEQ)) {
        TokenType op = p->previous.type;
        Node* right = parse_term(p);
        left = node_binop(op, left, right);
    }
    return left;
}

// ============ ЛОГИЧЕСКИЕ И/ИЛИ ============
// ИСКАТЬ: parse_logic
static Node* parse_logic(Parser* p) {
    Node* left = parse_comparison(p);
    while (match(p, TOK_AND) || match(p, TOK_OR)) {
        TokenType op = p->previous.type;
        Node* right = parse_comparison(p);
        left = node_binop(op, left, right);
    }
    return left;
}

// ============ ВЫРАЖЕНИЕ ============
static Node* parse_expression(Parser* p) {
    return parse_logic(p);
}

// ============ БЛОК С ОТСТУПАМИ ============
static Node* parse_block(Parser* p, int base_indent) {
    Node** stmts = safe_alloc(sizeof(Node*) * 256);
    int count = 0;

    while (!check(p, TOK_EOF) && !check(p, TOK_ELSE) && !check(p, TOK_ELIF) &&
           !check(p, TOK_CATCH) && !check(p, TOK_CASE) && !check(p, TOK_DEFAULT)) {
        int indent = p->lexer.col;
        
        if (indent < base_indent) break;
        if (indent > base_indent) {
            parse_error(p, "неожиданный отступ");
            break;
        }
        
        stmts[count++] = parse_stmt(p);
    }
    
    return node_block(stmts, count);
}

// ============ LET ============
static Node* parse_let(Parser* p) {
    char* name = p->current.str_val;
    advance(p);
    expect(p, TOK_EQ, "ожидался '=' после имени переменной");
    Node* value = parse_expression(p);
    return node_let(name, value);
}

// ============ IF/ELSE ============
static Node* parse_if(Parser* p) {
    Node* cond = parse_expression(p);
    
    int base_indent = p->lexer.col;
    if (base_indent == 0) base_indent = 2;
    
    advance(p);
    Node* body = parse_block(p, base_indent);
    
    Node* else_node = NULL;
    if (match(p, TOK_ELSE)) {
        advance(p);
        else_node = parse_block(p, base_indent);
    } else if (match(p, TOK_ELIF)) {
        else_node = parse_if(p);
    }
    
    return node_if(cond, body, else_node);
}

// ============ WHILE ============
static Node* parse_while(Parser* p) {
    Node* cond = parse_expression(p);
    int base_indent = p->lexer.col;
    if (base_indent == 0) base_indent = 2;
    advance(p);
    Node* body = parse_block(p, base_indent);
    return node_while(cond, body);
}

// ============ FOR ============
static Node* parse_for(Parser* p) {
    char* var_name = p->current.str_val;
    advance(p);
    expect(p, TOK_FROM, "ожидалось 'из'");
    Node* iterable = parse_expression(p);
    
    int base_indent = p->lexer.col;
    if (base_indent == 0) base_indent = 2;
    advance(p);
    
    Node* body = parse_block(p, base_indent);
    return node_for(var_name, iterable, body);
}

// ============ FUNCTION ============
static Node* parse_fn(Parser* p) {
    char* name = p->current.str_val;
    advance(p);
    expect(p, TOK_LPAREN, "ожидалась '(' после имени функции");
    
    Node** params = NULL; int n = 0;
    if (!match(p, TOK_RPAREN)) {
        params = safe_alloc(sizeof(Node*) * 16);
        params[n++] = node_ident(p->current.str_val);
        advance(p);
        while (match(p, TOK_COMMA)) {
            params[n++] = node_ident(p->current.str_val);
            advance(p);
        }
        expect(p, TOK_RPAREN, "ожидалась ')' после параметров");
    }
    
    int base_indent = p->lexer.col;
    if (base_indent == 0) base_indent = 2;
    advance(p);
    
    Node* body = parse_block(p, base_indent);
    return node_fn_def(name, params, n, body);
}

// ============ LAMBDA ============
// ИСКАТЬ: parse_lambda
static Node* parse_lambda(Parser* p) {
    Node** params = NULL; int n = 0;
    
    if (!check(p, TOK_COLON)) {
        params = safe_alloc(sizeof(Node*) * 16);
        params[n++] = node_ident(p->current.str_val);
        advance(p);
        while (match(p, TOK_COMMA)) {
            params[n++] = node_ident(p->current.str_val);
            advance(p);
        }
    }
    
    // Если есть ':', парсим тело как выражение
    Node* body;
    if (match(p, TOK_COLON)) {
        advance(p); // пропускаем пробелы
        body = parse_expression(p);
    } else {
        body = parse_expression(p);
    }
    
    return node_lambda(params, n, body);
}

// ============ SWITCH ============
// ИСКАТЬ: parse_switch
static Node* parse_switch(Parser* p) {
    Node* expr = parse_expression(p);
    int base_indent = p->lexer.col;
    if (base_indent == 0) base_indent = 2;
    advance(p);
    
    Node** cases = safe_alloc(sizeof(Node*) * 64);
    int case_count = 0;
    Node* default_case = NULL;
    
    while (!check(p, TOK_EOF)) {
        int indent = p->lexer.col;
        if (indent < base_indent) break;
        
        if (match(p, TOK_CASE)) {
            Node* case_val = parse_expression(p);
            advance(p);
            Node* case_body = parse_block(p, base_indent + 2);
            cases[case_count++] = node_case(case_val, case_body);
        }
        else if (match(p, TOK_ELSE)) {
            advance(p);
            default_case = parse_block(p, base_indent + 2);
            break;
        }
        else {
            break;
        }
    }
    
    return node_switch(expr, cases, default_case, case_count);
}

// ============ TRY/CATCH ============
// ИСКАТЬ: parse_try
static Node* parse_try(Parser* p) {
    int base_indent = p->lexer.col;
    if (base_indent == 0) base_indent = 2;
    advance(p);
    
    Node* body = parse_block(p, base_indent);
    
    expect(p, TOK_CATCH, "ожидался 'лови' после 'попробуй'");
    advance(p);
    
    Node* catch_body = parse_block(p, base_indent);
    
    return node_try_catch(body, catch_body);
}

// ============ STATEMENT ============
// ИСКАТЬ: parse_stmt
static Node* parse_stmt(Parser* p) {
    // Ключевые слова
    if (match(p, TOK_LET))       return parse_let(p);
    if (match(p, TOK_IF))        return parse_if(p);
    if (match(p, TOK_WHILE))     return parse_while(p);
    if (match(p, TOK_FOR))       return parse_for(p);
    if (match(p, TOK_FN))        return parse_fn(p);
    if (match(p, TOK_LAMBDA))    return parse_lambda(p);
    if (match(p, TOK_SWITCH))    return parse_switch(p);
    if (match(p, TOK_TRY))       return parse_try(p);
    if (match(p, TOK_RETURN))    return node_return(parse_expression(p));
    if (match(p, TOK_BREAK))     return node_break();
    if (match(p, TOK_CONTINUE))  return node_continue();
    if (match(p, TOK_THROW))     return node_throw(parse_expression(p));
    if (match(p, TOK_IMPORT)) {
        char* name = p->current.str_val;
        advance(p);
        return node_import(name);
    }
    if (match(p, TOK_PRINT)) {
        expect(p, TOK_LPAREN, "ожидалась '(' после 'выведи'");
        Node* val = parse_expression(p);
        expect(p, TOK_RPAREN, "ожидалась ')'");
        return node_print(val);
    }
    
    // Присваивание или выражение
    Node* expr = parse_expression(p);
    if (match(p, TOK_EQ)) {
        if (expr->type == NODE_IDENT) {
            Node* val = parse_expression(p);
            return node_assign(expr->str_val, val);
        }
        parse_error(p, "можно присваивать только переменной");
    }
    return expr;
}

// ============ ИНИЦИАЛИЗАЦИЯ ============
void parser_init(Parser* p, const char* src) {
    lexer_init(&p->lexer, src);
    p->had_error = 0;
    p->panic_mode = 0;
    advance(p);
}

Node* parse_program(Parser* p) {
    Node** stmts = safe_alloc(sizeof(Node*) * 512);
    int n = 0;
    while (!check(p, TOK_EOF)) {
        stmts[n++] = parse_stmt(p);
    }
    return node_block(stmts, n);
}

Node* parse(const char* source) {
    Parser p;
    parser_init(&p, source);
    Node* ast = parse_program(&p);
    if (p.had_error) return NULL;
    return ast;
}