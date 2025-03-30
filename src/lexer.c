#include "lexer.h"
#include "error.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

error_t *err_lexer_already_open = &(error_t){
    .message =
        "Can't open on a lexer object that is already opened. Close it first."};
error_t *err_prefix_too_large =
    &(error_t){.message = "Prefix too large for internal lexer buffer"};
error_t *err_buffer_underrun = &(error_t){
    .message = "Buffer does not contain enough characters for lexer_consume_n"};
error_t *err_consume_excessive_length =
    &(error_t){.message = "Too many valid characters to consume"};

error_t *err_eof =
    &(error_t){.message = "Can't read from file because EOF is reached"};

error_t *err_unknown_read = &(error_t){.message = "Unknown read error"};

error_t *err_allocation_failed =
    &(error_t){.message = "Memory allocation failed"};

typedef bool (*char_predicate_t)(char);

const char *lexer_token_id_to_cstr(lexer_token_id_t id) {
    switch (id) {
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    case TOKEN_IDENTIFIER:
        return "TOKEN_IDENTIFIER";
    case TOKEN_DECIMAL:
        return "TOKEN_DECIMAL";
    case TOKEN_HEXADECIMAL:
        return "TOKEN_HEXADECIMAL";
    case TOKEN_OCTAL:
        return "TOKEN_OCTAL";
    case TOKEN_BINARY:
        return "TOKEN_BINARY";
    case TOKEN_CHAR:
        return "TOKEN_CHAR";
    case TOKEN_STRING:
        return "TOKEN_STRING";
    case TOKEN_COLON:
        return "TOKEN_COLON";
    case TOKEN_COMMA:
        return "TOKEN_COMMA";
    case TOKEN_LBRACKET:
        return "TOKEN_LBRACKET";
    case TOKEN_RBRACKET:
        return "TOKEN_RBRACKET";
    case TOKEN_PLUS:
        return "TOKEN_PLUS";
    case TOKEN_MINUS:
        return "TOKEN_MINUS";
    case TOKEN_ASTERISK:
        return "TOKEN_ASTERISK";
    case TOKEN_DOT:
        return "TOKEN_DOT";
    case TOKEN_COMMENT:
        return "TOKEN_COMMENT";
    case TOKEN_NEWLINE:
        return "TOKEN_NEWLINE";
    case TOKEN_WHITESPACE:
        return "TOKEN_WHITESPACE";
    }
    assert(!"Unreachable, weird token id" && id);
    __builtin_unreachable();
}

void lexer_token_print(lexer_token_t *token) {
    printf("(%zu, %zu) %s[%d]%s%s\n", token->line_number,
           token->character_number, lexer_token_id_to_cstr(token->id),
           token->id, token->value ? ": " : "",
           token->value ? token->value : "");
    if (token->id == TOKEN_ERROR)
        printf("  `--> %s\n", token->explanation);
}

void lexer_token_cleanup(lexer_token_t *token) {
    free(token->value);
    memset(token, 0, sizeof(lexer_token_t));
}

void lexer_close(lexer_t *lex) {
    fclose(lex->fp);
    memset(lex, 0, sizeof(lexer_t));
}

error_t *lexer_fill_buffer(lexer_t *lex) {
    if (feof(lex->fp) && lex->buffer_count == 0)
        return err_eof;
    if (feof(lex->fp))
        return nullptr;
    if (lex->buffer_count == lexer_buffer_size)
        return nullptr;

    size_t remaining = lexer_buffer_size - lex->buffer_count;
    while (remaining > 0) {
        char *buffer = lex->buffer + lex->buffer_count;
        size_t n = fread(buffer, 1, remaining, lex->fp);
        if (n == 0 && feof(lex->fp))
            break;
        if (n == 0 && ferror(lex->fp))
            return errorf("Read error: %s", strerror(errno));
        if (n == 0)
            return err_unknown_read;
        remaining -= n;
        lex->buffer_count += n;
    }
    return nullptr;
}

error_t *lexer_open(lexer_t *lex, char *path) {
    if (lex->fp != nullptr)
        return err_lexer_already_open;

    lex->fp = fopen(path, "rb");
    if (lex->fp == nullptr)
        return errorf("Failed to open file '%s': %s", path, strerror(errno));
    lex->line_number = 0;
    lex->character_number = 0;
    lex->buffer_count = 0;
    return nullptr;
}

void lexer_shift_buffer(lexer_t *lex, int n) {
    lex->buffer_count -= n;
    memmove(lex->buffer, lex->buffer + n, lex->buffer_count);
}

error_t *lexer_peek(lexer_t *lex, char *c) {
    error_t *err = lexer_fill_buffer(lex);
    if (err)
        return err;
    if (lex->buffer_count == 0)
        return err_eof;
    *c = lex->buffer[0];
    lexer_shift_buffer(lex, 1);
    return nullptr;
}

// This does _not_ fill the internal lexer buffer and you _must_ call
// lexer_fill_buffer() before calling this. It will always return false if your
// prefix is larger than lexer_buffer_size
bool lexer_has_prefix(lexer_t *lex, char *prefix) {
    size_t len = strlen(prefix);
    if (len > lex->buffer_count)
        return false;
    return memcmp(lex->buffer, prefix, len) == 0;
}

error_t *lexer_not_implemented(lexer_t *lex, lexer_token_t *token) {
    (void)token;
    return errorf("Not implemented, character %02x (%c) at (%zu, %zu).\n",
                  lex->buffer[0], lex->buffer[0], lex->line_number,
                  lex->character_number);
}

error_t *lexer_consume_n(lexer_t *lex, const size_t len,
                         char buffer[static len], const size_t n) {
    if (lex->buffer_count < n)
        return err_buffer_underrun;
    if (len > n)
        return err_consume_excessive_length;

    memcpy(buffer, lex->buffer, n);
    lexer_shift_buffer(lex, n);
    return nullptr;
}
error_t *lexer_consume(lexer_t *lex, const size_t n, char buffer[static n],
                       char_predicate_t is_valid, size_t *n_consumed) {
    const size_t buffer_size = n;
    bool have_more_characters = false;
    *n_consumed = 0;
    do {
        size_t i = 0;
        while (i < lex->buffer_count && i < buffer_size - *n_consumed &&
               is_valid(lex->buffer[i])) {
            ++i;
        }
        memcpy(buffer + *n_consumed, lex->buffer, i);
        lexer_shift_buffer(lex, i);
        *n_consumed += i;

        error_t *err = lexer_fill_buffer(lex);
        if (err == err_eof)
            have_more_characters = false;
        else if (err)
            return err;
        else
            have_more_characters =
                (lex->buffer_count > 0 && is_valid(lex->buffer[0]));

        if (have_more_characters && *n_consumed == buffer_size) {
            return err_consume_excessive_length;
        }
    } while (have_more_characters);
    return nullptr;
}

bool is_hexadecimal_character(char c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_octal_character(char c) {
    return c >= '0' && c <= '7';
}

bool is_binary_character(char c) {
    return c == '0' || c == '1';
}

bool is_decimal_character(char c) {
    return isdigit(c);
}

error_t *lexer_next_number(lexer_t *lex, lexer_token_t *token) {
    constexpr size_t max_number_length = 128;
    size_t so_far = 0;
    size_t n = 0;
    char buffer[max_number_length + 1] = {};

    token->line_number = lex->line_number;
    token->character_number = lex->character_number;
    char_predicate_t is_valid;
    if (lexer_has_prefix(lex, "0x")) {
        is_valid = is_hexadecimal_character;
        token->id = TOKEN_HEXADECIMAL;
        strcpy(buffer, "0x");
        so_far = 2;
    } else if (lexer_has_prefix(lex, "0o")) {
        is_valid = is_octal_character;
        token->id = TOKEN_OCTAL;
        strcpy(buffer, "0o");
        so_far = 2;
    } else if (lexer_has_prefix(lex, "0b")) {
        token->id = TOKEN_BINARY;
        is_valid = is_binary_character;
        strcpy(buffer, "0b");
        so_far = 2;
    } else {
        token->id = TOKEN_DECIMAL;
        is_valid = is_decimal_character;
        so_far = 0;
    }
    if (so_far > 0) {
        lex->character_number += so_far;
        lexer_shift_buffer(lex, so_far);
    }

    error_t *err = lexer_consume(lex, max_number_length - so_far,
                                 buffer + so_far, is_valid, &n);
    if (err == err_consume_excessive_length) {
        token->id = TOKEN_ERROR;
        token->explanation =
            "Number length exceeds the maximum of 128 characters";
    }
    so_far += n;
    if (n == 0) {
        token->id = TOKEN_ERROR;
        token->explanation = "Invalid number format";
    }

    err = lexer_fill_buffer(lex);
    if (err != err_eof && err) {
        return err;
    }

    size_t suffix_length = 0;
    if (lexer_has_prefix(lex, ":8")) {
        suffix_length = 2;
    } else if (lexer_has_prefix(lex, ":16")) {
        suffix_length = 3;
    } else if (lexer_has_prefix(lex, ":32")) {
        suffix_length = 3;
    } else if (lexer_has_prefix(lex, ":64")) {
        suffix_length = 3;
    }

    if (suffix_length > 0) {
        err = lexer_consume_n(lex, max_number_length - so_far, buffer + so_far,
                              suffix_length);
        if (err == err_consume_excessive_length) {
            token->id = TOKEN_ERROR;
            token->explanation =
                "Number length exceeds the maximum of 128 characters";
        }
    }

    lex->character_number += n;
    token->value = strdup(buffer);
    return nullptr;
}
error_t *lexer_next_newline(lexer_t *lex, lexer_token_t *token) {
    token->line_number = lex->line_number;
    token->character_number = lex->character_number;
    token->id = TOKEN_NEWLINE;

    if (lexer_has_prefix(lex, "\r\n")) {
        lexer_shift_buffer(lex, 2);
        token->value = strdup("\r\n");
        lex->character_number = 0;
        lex->line_number += 1;
    } else if (lexer_has_prefix(lex, "\n")) {
        lexer_shift_buffer(lex, 1);
        token->value = strdup("\n");
        lex->character_number = 0;
        lex->line_number += 1;
    } else {
        lexer_shift_buffer(lex, 1);
        token->id = TOKEN_ERROR;
        lex->character_number += 1;
        token->value = strdup((char[]){lex->buffer[0]});
        token->explanation = "Invalid newline format";
    }
    return nullptr;
}

bool is_identifier_character(char c) {
    return isalnum(c) || c == '_';
}

error_t *lexer_next_identifier(lexer_t *lex, lexer_token_t *token) {
    constexpr size_t max_identifier_length = 128;
    size_t n = 0;
    char buffer[max_identifier_length + 1] = {};

    token->id = TOKEN_IDENTIFIER;
    token->line_number = lex->line_number;
    token->character_number = lex->character_number;

    error_t *err = lexer_consume(lex, max_identifier_length, buffer,
                                 is_identifier_character, &n);
    if (err == err_consume_excessive_length) {
        token->id = TOKEN_ERROR;
        token->explanation =
            "Identifier length exceeds the maximum of 128 characters";
    }
    lex->character_number += n;
    token->value = strdup(buffer);
    return nullptr;
}

error_t *lexer_next_character(lexer_t *lex, lexer_token_t *token) {
    return lexer_not_implemented(lex, token);
}
error_t *lexer_next_string(lexer_t *lex, lexer_token_t *token) {
    return lexer_not_implemented(lex, token);
}

bool is_whitespace_character(char c) {
    return c == ' ' || c == '\t';
}

error_t *lexer_next_whitespace(lexer_t *lex, lexer_token_t *token) {
    constexpr size_t max_whitespace_length = 1024;
    size_t n = 0;
    char buffer[max_whitespace_length + 1] = {};

    token->id = TOKEN_WHITESPACE;
    token->line_number = lex->line_number;
    token->character_number = lex->character_number;

    error_t *err = lexer_consume(lex, max_whitespace_length, buffer,
                                 is_whitespace_character, &n);
    if (err == err_consume_excessive_length) {
        token->id = TOKEN_ERROR;
        token->explanation =
            "Whitespace length exceeds the maximum of 1024 characters";
    }
    lex->character_number += n;
    token->value = strdup(buffer);
    return nullptr;
}

bool is_comment_character(char c) {
    return c != '\r' && c != '\n';
}

error_t *lexer_next_comment(lexer_t *lex, lexer_token_t *token) {
    constexpr size_t max_comment_length = 1024;
    size_t n = 0;
    char buffer[max_comment_length + 1] = {};

    token->id = TOKEN_COMMENT;
    token->line_number = lex->line_number;
    token->character_number = lex->character_number;

    error_t *err = lexer_consume(lex, max_comment_length, buffer,
                                 is_comment_character, &n);
    if (err == err_consume_excessive_length) {
        token->id = TOKEN_ERROR;
        token->explanation =
            "Comment length exceeds the maximum of 1024 characters";
    }
    lex->character_number += n;
    token->value = strdup(buffer);
    return nullptr;
}

error_t *lexer_next(lexer_t *lex, lexer_token_t *token) {
    memset(token, 0, sizeof(lexer_token_t));
    error_t *err = lexer_fill_buffer(lex);
    if (err)
        return err;
    char first = lex->buffer[0];
    if (isalpha(first) || first == '_')
        return lexer_next_identifier(lex, token);
    if (isdigit(first))
        return lexer_next_number(lex, token);

    switch (first) {
    case '\'':
        return lexer_next_character(lex, token);
    case '"':
        return lexer_next_string(lex, token);
    case ' ':
    case '\t':
        return lexer_next_whitespace(lex, token);
    case ';':
        return lexer_next_comment(lex, token);
    case ':':
        token->id = TOKEN_COLON;
        break;
    case ',':
        token->id = TOKEN_COMMA;
        break;
    case '[':
        token->id = TOKEN_LBRACKET;
        break;
    case ']':
        token->id = TOKEN_RBRACKET;
        break;
    case '+':
        token->id = TOKEN_PLUS;
        break;
    case '-':
        token->id = TOKEN_MINUS;
        break;
    case '*':
        token->id = TOKEN_ASTERISK;
        break;
    case '.':
        token->id = TOKEN_DOT;
        break;
    case '\r':
    case '\n':
        return lexer_next_newline(lex, token);
    default:
        token->id = TOKEN_ERROR;
        break;
    }
    token->value = strdup((char[]){first, 0});
    lexer_shift_buffer(lex, 1);
    token->line_number = lex->line_number;
    token->character_number = lex->character_number;
    if (token->id == TOKEN_ERROR) {
        token->explanation =
            "unexpected character during lexing (first of token)";
    }
    lex->character_number += 1;
    return nullptr;
}
