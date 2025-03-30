#ifndef INCLUDE_SRC_LEXER_H_
#define INCLUDE_SRC_LEXER_H_

#include "error.h"
#include <stddef.h>
#include <stdio.h>

extern error_t *err_eof;

typedef enum {
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,
    TOKEN_DECIMAL,
    TOKEN_HEXADECIMAL,
    TOKEN_OCTAL,
    TOKEN_BINARY,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERISK,
    TOKEN_DOT,
    TOKEN_COMMENT,
    TOKEN_NEWLINE,
    TOKEN_WHITESPACE,
} lexer_token_id_t;

typedef struct lexer_token {
    lexer_token_id_t id;
    size_t line_number;
    size_t character_number;
    char *value;
    const char *explanation;
} lexer_token_t;

constexpr size_t lexer_buffer_size = 32;

typedef struct lexer {
    size_t line_number;
    size_t character_number;
    size_t buffer_count;
    char buffer[lexer_buffer_size];
    FILE *fp;
} lexer_t;

void lexer_close(lexer_t *lex);
error_t *lexer_open(lexer_t *lex, char *path);
error_t *lexer_next(lexer_t *lex, lexer_token_t *token);
void lexer_token_print(lexer_token_t *token);
void lexer_token_cleanup(lexer_token_t *token);

#endif // INCLUDE_SRC_LEXER_H_
