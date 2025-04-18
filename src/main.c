#include "ast.h"
#include "encoder/encoder.h"
#include "error.h"
#include "lexer.h"
#include "parser/parser.h"
#include "tokenlist.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum mode {
    MODE_INVALID = -1,
    MODE_AST,
    MODE_TEXT,
    MODE_TOKENS,
    MODE_ENCODING,
} mode_t;

void print_tokens(tokenlist_t *list) {
    for (auto entry = list->head; entry; entry = entry->next) {
        auto token = &entry->token;
        lexer_token_print(token);
    }
}

void print_text(tokenlist_t *list) {
    for (auto entry = list->head; entry; entry = entry->next) {
        auto token = &entry->token;
        if (token->id == TOKEN_ERROR) {
            printf("%s\n", token->value);
            for (size_t i = 0; i < token->character_number; ++i)
                printf(" ");
            printf("^-- %s\n", token->explanation);
            return;
        } else {
            printf("%s", token->value);
        }
    }
}

error_t *print_ast(tokenlist_t *list) {
    parse_result_t result = parse(list->head);
    if (result.err)
        return result.err;

    ast_node_print(result.node);

    if (result.next != nullptr) {
        puts("First unparsed token:");
        lexer_token_print(&result.next->token);
    }
    ast_node_free(result.node);
    if (result.next != nullptr) {
        return errorf("did not parse entire input token stream");
    }
    return nullptr;
}

void print_hex(size_t len, uint8_t bytes[static len]) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", bytes[i]);
        if (i < len - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

error_t *print_encoding(tokenlist_t *list) {
    parse_result_t result = parse(list->head);
    if (result.err)
        return result.err;

    encoder_t *encoder;
    error_t *err = encoder_alloc(&encoder, result.node);
    if (err)
        goto cleanup_ast;

    err = encoder_encode(encoder);
    if (err)
        goto cleanup_ast;

    ast_node_t *root = result.node;
    for (size_t i = 0; i < root->len; ++i) {
        ast_node_t *node = root->children[i];
        if (node->id != NODE_INSTRUCTION)
            continue;

        print_hex(node->value.instruction.encoding.len,
                  node->value.instruction.encoding.buffer);
    }

    encoder_free(encoder);
    ast_node_free(result.node);
    return nullptr;

cleanup_ast:
    ast_node_free(result.node);
    return err;
}

int get_execution_mode(int argc, char *argv[]) {
    if (argc != 3)
        return MODE_INVALID;

    if (strcmp(argv[1], "tokens") == 0)
        return MODE_TOKENS;
    if (strcmp(argv[1], "text") == 0)
        return MODE_TEXT;
    if (strcmp(argv[1], "ast") == 0)
        return MODE_AST;
    if (strcmp(argv[1], "encoding") == 0)
        return MODE_ENCODING;
    return MODE_INVALID;
}

error_t *do_action(mode_t mode, tokenlist_t *list) {
    switch (mode) {
    case MODE_TOKENS:
        print_tokens(list);
        return nullptr;
    case MODE_TEXT:
        print_text(list);
        return nullptr;
    case MODE_AST:
        return print_ast(list);
    case MODE_ENCODING:
        return print_encoding(list);
    case MODE_INVALID:
        /* can't happen */
    }
    __builtin_unreachable();
}

int main(int argc, char *argv[]) {
    mode_t mode = get_execution_mode(argc, argv);
    if (mode == MODE_INVALID) {
        puts("Usage: oas [tokens|text|ast|encoding] <filename>");
        exit(1);
    }
    char *filename = argv[2];

    lexer_t *lex = &(lexer_t){};
    error_t *err = lexer_open(lex, filename);
    if (err)
        goto cleanup_error;

    tokenlist_t *list;
    err = tokenlist_alloc(&list);
    if (err)
        goto cleanup_lexer;

    err = tokenlist_fill(list, lex);
    if (err)
        goto cleanup_tokens;

    err = do_action(mode, list);
    if (err)
        goto cleanup_tokens;

    tokenlist_free(list);
    error_free(err);
    return 0;

cleanup_tokens:
    tokenlist_free(list);
cleanup_lexer:
    lexer_close(lex);
cleanup_error:
    puts(err->message);
    error_free(err);
    return 1;
}
