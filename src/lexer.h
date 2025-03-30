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

/**
 * @brief Closes a lexer and releases associated resources
 *
 * @param lex Pointer to the lexer to close
 */
void lexer_close(lexer_t *lex);

/**
 * @brief Opens a file for lexical analysis
 *
 * @param lex Pointer to the lexer to initialize
 * @param path Path to the file to open
 * @return error_t* nullptr on success, or error describing the failure
 */
error_t *lexer_open(lexer_t *lex, char *path);

/**
 * @brief Reads the next token from the input stream
 *
 * @param lex Pointer to an initialized lexer
 * @param token Pointer to a token structure to fill with the next token
 * @return error_t* nullptr on success, err_eof at end of file, or other error
 */
error_t *lexer_next(lexer_t *lex, lexer_token_t *token);

/**
 * @brief Prints a token to stdout for debugging purposes
 *
 * @param token Pointer to the token to print
 */
void lexer_token_print(lexer_token_t *token);

/**
 * @brief Frees any resources associated with a token
 *
 * @param token Pointer to the token to clean up
 */
void lexer_token_cleanup(lexer_token_t *token);

#endif // INCLUDE_SRC_LEXER_H_
