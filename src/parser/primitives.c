#include "primitives.h"
#include "../ast.h"
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

parse_result_t parse_label_reference(tokenlist_entry_t *current) {
    return parse_token(current, TOKEN_IDENTIFIER, NODE_LABEL_REFERENCE,
                       nullptr);
}

const char *registers[] = {
    // 64-bit registers
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10",
    "r11", "r12", "r13", "r14", "r15",
    // 32-bit registers
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d",
    "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
    // 16-bit registers
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w",
    "r11w", "r12w", "r13w", "r14w", "r15w",
    // 8-bit low registers
    "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b",
    "r11b", "r12b", "r13b", "r14b", "r15b", nullptr};

bool is_register_token(lexer_token_t *token) {
    for (size_t i = 0; registers[i] != nullptr; ++i)
        if (strcmp(token->value, registers[i]) == 0)
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

parse_result_t parse_section(tokenlist_entry_t *current) {}
