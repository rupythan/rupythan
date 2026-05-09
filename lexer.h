#ifndef LEXER_H
#define LEXER_H
#include "utils.h"

typedef enum {
    // Литералы
    TOK_NUMBER,
    TOK_FLOAT,
    TOK_STRING,
    TOK_IDENT,

    // Скобки и разделители
    TOK_LPAREN,      // (
    TOK_RPAREN,      // )
    TOK_LBRACKET,    // [
    TOK_RBRACKET,    // ]
    TOK_LBRACE,      // {
    TOK_RBRACE,      // }
    TOK_COLON,       // :
    TOK_COMMA,       // ,
    TOK_DOT,         // .

    // Операторы
    TOK_PLUS,        // +
    TOK_MINUS,       // -
    TOK_STAR,        // *
    TOK_SLASH,       // /
    TOK_EQ,          // =
    TOK_EQEQ,        // ==
    TOK_NEQ,         // !=
    TOK_LT,          // <
    TOK_GT,          // >
    TOK_LEQ,         // <=
    TOK_GEQ,         // >=
    TOK_AND,         // и
    TOK_OR,          // или

    // Ключевые слова
    TOK_IMPORT,      // импорт
    TOK_LET,         // пусть
    TOK_FN,          // фн
    TOK_IF,          // если
    TOK_ELSE,        // иначе
    TOK_ELIF,        // илиесли
    TOK_WHILE,       // пока
    TOK_FOR,         // для
    TOK_FROM,        // из
    TOK_RETURN,      // верни
    TOK_PRINT,       // выведи
    TOK_NIL,         // ничто
    TOK_TRUE,        // истина
    TOK_FALSE,       // ложь

    // Новые ключевые слова
    TOK_TRY,         // попробуй
    TOK_CATCH,       // лови
    TOK_THROW,       // бросай
    TOK_BREAK,       // стоп
    TOK_CONTINUE,    // дальше
    TOK_SWITCH,      // выбор
    TOK_CASE,        // случай
    TOK_DEFAULT,     // по_умолчанию (зарезервировано)
    TOK_LAMBDA,      // лямбда

    TOK_EOF,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    union {
        int     int_val;
        double  float_val;
        char*   str_val;
    };
    int line;
    int col;
} Token;

typedef struct {
    const char* source;
    int pos;
    int line;
    int col;
} Lexer;

void lexer_init(Lexer* l, const char* src);
Token lexer_next(Lexer* l);
const char* token_type_name(TokenType t);

#endif