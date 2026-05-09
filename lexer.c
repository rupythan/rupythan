
#include "lexer.h"

static char peek(Lexer* l) { return l->source[l->pos]; }
static char prev(Lexer* l) { return l->pos > 0 ? l->source[l->pos-1] : 0; }

static char advance(Lexer* l) {
    char c = l->source[l->pos++];
    l->col++;
    if (c == '\n') { l->line++; l->col = 0; }
    return c;
}

static int is_digit(char c) { return c >= '0' && c <= '9'; }

static int is_alpha_utf8(const char* s, int pos) {
    unsigned char c = s[pos];
    return (c >= 0x80) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_alpha_num_utf8(const char* s, int pos) {
    return is_digit(s[pos]) || is_alpha_utf8(s, pos);
}

static int utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if (c < 0xE0) return 2;
    return 3;
}

static Token make_tok(Lexer* l, TokenType t) {
    Token tok;
    memset(&tok, 0, sizeof(tok));
    tok.type = t;
    tok.line = l->line;
    tok.col = l->col;
    tok.int_val = 0;
    tok.float_val = 0.0;
    tok.str_val = NULL;
    return tok;
}

static Token read_number(Lexer* l) {
    char buf[64];
    int i = 0;
    buf[i++] = prev(l);
    int has_dot = 0;
    
    while (is_digit(peek(l)) || (peek(l) == '.' && !has_dot)) {
        if (peek(l) == '.') has_dot = 1;
        buf[i++] = advance(l);
    }
    buf[i] = 0;
    
    Token t = make_tok(l, TOK_NUMBER);
    if (has_dot) {
        t.float_val = atof(buf);
        t.int_val = (int)t.float_val;
    } else {
        t.int_val = atoi(buf);
        t.float_val = (double)t.int_val;
    }
    return t;
}

static Token read_ident(Lexer* l) {
    char buf[256];
    int i = 0;
    int start = l->pos - 1;
    int len = utf8_char_len(l->source[start]);
    
    for (int k = 0; k < len; k++) buf[i++] = l->source[start + k];
    l->pos = start + len;
    l->col += len - 1;
    
    while (peek(l) && is_alpha_num_utf8(l->source, l->pos)) {
        int clen = utf8_char_len(peek(l));
        for (int k = 0; k < clen; k++) buf[i++] = advance(l);
    }
    buf[i] = 0;
    
    Token t = make_tok(l, TOK_IDENT);
    t.str_val = str_dup(buf);
    
    // Ключевые слова
    if (!strcmp(buf, "пусть"))        t.type = TOK_LET;
    else if (!strcmp(buf, "фн"))      t.type = TOK_FN;
    else if (!strcmp(buf, "если"))    t.type = TOK_IF;
    else if (!strcmp(buf, "иначе"))   t.type = TOK_ELSE;
    else if (!strcmp(buf, "илиесли")) t.type = TOK_ELIF;
    else if (!strcmp(buf, "пока"))    t.type = TOK_WHILE;
    else if (!strcmp(buf, "для"))     t.type = TOK_FOR;
    else if (!strcmp(buf, "из"))      t.type = TOK_FROM;
    else if (!strcmp(buf, "верни"))   t.type = TOK_RETURN;
    else if (!strcmp(buf, "выведи"))  t.type = TOK_PRINT;
    else if (!strcmp(buf, "ничто"))   t.type = TOK_NIL;
    else if (!strcmp(buf, "истина"))  t.type = TOK_TRUE;
    else if (!strcmp(buf, "ложь"))    t.type = TOK_FALSE;
    else if (!strcmp(buf, "выход"))   t.type = TOK_EOF;
    else if (!strcmp(buf, "справка")) t.type = TOK_IMPORT;
    // Новые ключевые слова
    else if (!strcmp(buf, "и"))       t.type = TOK_AND;
    else if (!strcmp(buf, "или"))     t.type = TOK_OR;
    else if (!strcmp(buf, "попробуй")) t.type = TOK_TRY;
    else if (!strcmp(buf, "лови"))    t.type = TOK_CATCH;
    else if (!strcmp(buf, "бросай"))  t.type = TOK_THROW;
    else if (!strcmp(buf, "стоп"))    t.type = TOK_BREAK;
    else if (!strcmp(buf, "дальше"))  t.type = TOK_CONTINUE;
    else if (!strcmp(buf, "выбор"))   t.type = TOK_SWITCH;
    else if (!strcmp(buf, "случай"))  t.type = TOK_CASE;
    else if (!strcmp(buf, "лямбда"))  t.type = TOK_LAMBDA;
    else if (!strcmp(buf, "импорт"))  t.type = TOK_IMPORT;
    
    return t;
}

static Token read_string(Lexer* l) {
    char buf[4096];
    int i = 0;
    
    while (peek(l) != '"' && peek(l) && peek(l) != '\n') {
        if (peek(l) == '\\') {
            advance(l);
            switch (peek(l)) {
                case 'n':  buf[i++] = '\n'; advance(l); break;
                case 't':  buf[i++] = '\t'; advance(l); break;
                case '"':  buf[i++] = '"';  advance(l); break;
                case '\\': buf[i++] = '\\'; advance(l); break;
                case 'r':  buf[i++] = '\r'; advance(l); break;
                default:   buf[i++] = peek(l); advance(l); break;
            }
        } else {
            buf[i++] = peek(l);
            advance(l);
        }
    }
    
    if (peek(l) == '"') advance(l);
    buf[i] = 0;
    
    Token t = make_tok(l, TOK_STRING);
    t.str_val = str_dup(buf);
    return t;
}

void lexer_init(Lexer* l, const char* src) {
    l->source = src;
    l->pos = 0;
    l->line = 1;
    l->col = 0;
}

Token lexer_next(Lexer* l) {
    // Пропуск пробелов и табуляции
    while (peek(l) == ' ' || peek(l) == '\t' || peek(l) == '\r') {
        advance(l);
    }
    
    // Перевод строки — возвращаем спец-токен 999
    if (peek(l) == '\n') {
        advance(l);
        Token t = make_tok(l, (TokenType)999);
        t.line = l->line - 1;
        return t;
    }
    
    // Комментарий //
    if (peek(l) == '/' && l->source[l->pos + 1] == '/') {
        while (peek(l) != '\n' && peek(l)) advance(l);
        return lexer_next(l);
    }
    
    // Комментарий /* */
    if (peek(l) == '/' && l->source[l->pos + 1] == '*') {
        advance(l);
        advance(l);
        while (!(prev(l) == '*' && peek(l) == '/') && peek(l)) advance(l);
        if (peek(l)) advance(l);
        return lexer_next(l);
    }
    
    // Конец файла
    if (!peek(l)) return make_tok(l, TOK_EOF);
    
    char c = advance(l);
    
    switch (c) {
        case '(': return make_tok(l, TOK_LPAREN);
        case ')': return make_tok(l, TOK_RPAREN);
        case '[': return make_tok(l, TOK_LBRACKET);
        case ']': return make_tok(l, TOK_RBRACKET);
        case '{': return make_tok(l, TOK_LBRACE);
        case '}': return make_tok(l, TOK_RBRACE);
        case ':': return make_tok(l, TOK_COLON);
        case ',': return make_tok(l, TOK_COMMA);
        case '.': return make_tok(l, TOK_DOT);
        case '+': return make_tok(l, TOK_PLUS);
        case '*': return make_tok(l, TOK_STAR);
        case '/': return make_tok(l, TOK_SLASH);
        case '-': return make_tok(l, TOK_MINUS);
        
        case '=':
            if (peek(l) == '=') { advance(l); return make_tok(l, TOK_EQEQ); }
            return make_tok(l, TOK_EQ);
        
        case '<':
            if (peek(l) == '=') { advance(l); return make_tok(l, TOK_LEQ); }
            return make_tok(l, TOK_LT);
        
        case '>':
            if (peek(l) == '=') { advance(l); return make_tok(l, TOK_GEQ); }
            return make_tok(l, TOK_GT);
        
        case '!':
            if (peek(l) == '=') { advance(l); return make_tok(l, TOK_NEQ); }
            break;
        
        case '"':
            return read_string(l);
        
        default:
            if (is_digit(c)) return read_number(l);
            if (is_alpha_utf8(l->source, l->pos - 1)) return read_ident(l);
            break;
    }
    
    Token t;
    memset(&t, 0, sizeof(t));
    t.type = TOK_ERROR;
    t.line = l->line;
    return t;
}

const char* token_type_name(TokenType t) {
    switch (t) {
        case TOK_NUMBER:   return "ЧИСЛО";
        case TOK_STRING:   return "СТРОКА";
        case TOK_IDENT:    return "ИМЯ";
        case TOK_LPAREN:   return "(";
        case TOK_RPAREN:   return ")";
        case TOK_LBRACKET: return "[";
        case TOK_RBRACKET: return "]";
        case TOK_LBRACE:   return "{";
        case TOK_RBRACE:   return "}";
        case TOK_COLON:    return ":";
        case TOK_COMMA:    return ",";
        case TOK_DOT:      return ".";
        case TOK_PLUS:     return "+";
        case TOK_MINUS:    return "-";
        case TOK_STAR:     return "*";
        case TOK_SLASH:    return "/";
        case TOK_EQ:       return "=";
        case TOK_EQEQ:     return "==";
        case TOK_NEQ:      return "!=";
        case TOK_LT:       return "<";
        case TOK_GT:       return ">";
        case TOK_LEQ:      return "<=";
        case TOK_GEQ:      return ">=";
        case TOK_LET:      return "пусть";
        case TOK_FN:       return "фн";
        case TOK_IF:       return "если";
        case TOK_ELSE:     return "иначе";
        case TOK_ELIF:     return "илиесли";
        case TOK_WHILE:    return "пока";
        case TOK_FOR:      return "для";
        case TOK_FROM:     return "из";
        case TOK_RETURN:   return "верни";
        case TOK_PRINT:    return "выведи";
        case TOK_NIL:      return "ничто";
        case TOK_TRUE:     return "истина";
        case TOK_FALSE:    return "ложь";
        case TOK_AND:      return "и";
        case TOK_OR:       return "или";
        case TOK_TRY:      return "попробуй";
        case TOK_CATCH:    return "лови";
        case TOK_THROW:    return "бросай";
        case TOK_BREAK:    return "стоп";
        case TOK_CONTINUE: return "дальше";
        case TOK_SWITCH:   return "выбор";
        case TOK_CASE:     return "случай";
        case TOK_LAMBDA:   return "лямбда";
        case TOK_IMPORT:   return "импорт";
        case TOK_EOF:      return "КОНЕЦ";
        default:           return "???";
    }
}