#include "primitives.h"
#include "../ast.h"
#include "../data/registers.h"
#include <string.h>

parse_result_t parse_identifier(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_IDENTIFIER, nullptr);
}

parse_result_t parse_decimal(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_DECIMAL, NODE_DECIMAL, nullptr);
}

parse_result_t parse_hexadecimal(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_HEXADECIMAL, NODE_HEXADECIMAL, nullptr);
}

parse_result_t parse_binary(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_BINARY, NODE_BINARY, nullptr);
}

parse_result_t parse_octal(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_OCTAL, NODE_OCTAL, nullptr);
}

parse_result_t parse_string(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_STRING, NODE_STRING, nullptr);
}

parse_result_t parse_char(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_CHAR, NODE_CHAR, nullptr);
}

parse_result_t parse_colon(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_COLON, NODE_COLON, nullptr);
}

parse_result_t parse_comma(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_COMMA, NODE_COMMA, nullptr);
}

parse_result_t parse_lbracket(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_LBRACKET, NODE_LBRACKET, nullptr);
}

parse_result_t parse_rbracket(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_RBRACKET, NODE_RBRACKET, nullptr);
}

parse_result_t parse_plus(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_PLUS, NODE_PLUS, nullptr);
}

parse_result_t parse_minus(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_MINUS, NODE_MINUS, nullptr);
}

parse_result_t parse_asterisk(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_ASTERISK, NODE_ASTERISK, nullptr);
}

parse_result_t parse_dot(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_DOT, NODE_DOT, nullptr);
}

parse_result_t parse_newline(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_NEWLINE, NODE_NEWLINE, nullptr);
}

parse_result_t parse_label_reference(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_LABEL_REFERENCE,
                       nullptr);
}

bool is_register_token(lexer_token_t *token) {
    for (size_t i = 0; registers[i] != nullptr; ++i)
        if (strcmp(token->value, registers[i]->name) == 0)
            return true;
    return false;
}

parse_result_t parse_register(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_REGISTER,
                       is_register_token);
}

bool is_section_token(lexer_token_t *token) {
    return strcmp(token->value, "section") == 0;
}

parse_result_t parse_section(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_SECTION,
                       is_section_token);
}

bool is_import_token(lexer_token_t *token) {
    return strcmp(token->value, "import") == 0;
}

parse_result_t parse_import(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_IMPORT, is_import_token);
}

bool is_export_token(lexer_token_t *token) {
    return strcmp(token->value, "export") == 0;
}

parse_result_t parse_export(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_EXPORT, is_export_token);
}
