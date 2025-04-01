#include "error.h"
#include "lexer.h"
#include "parser/parser.h"
#include "tokenlist.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum mode { MODE_AST, MODE_TEXT, MODE_TOKENS } mode_t;

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

void print_ast(tokenlist_t *list) {
    parse_result_t result = parse(list->head);
    if (result.err) {
        puts(result.err->message);
        error_free(result.err);
        return;
    }
    ast_node_print(result.node);

    if (result.next != nullptr) {
        puts("First unparsed token:");
        lexer_token_print(&result.next->token);
    }

    ast_node_free(result.node);
}

int get_execution_mode(int argc, char *argv[]) {
    if (argc != 3 || (strcmp(argv[1], "tokens") != 0 &&
                      strcmp(argv[1], "text") != 0 && strcmp(argv[1], "ast"))) {
        puts("Usage: oas [tokens|text|ast] <filename>");
        exit(1);
    }

    if (strcmp(argv[1], "tokens") == 0)
        return MODE_TOKENS;
    if (strcmp(argv[1], "text") == 0)
        return MODE_TEXT;
    return MODE_AST;
}

int main(int argc, char *argv[]) {
    mode_t mode = get_execution_mode(argc, argv);
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

    switch (mode) {
    case MODE_TOKENS:
        print_tokens(list);
        break;
    case MODE_TEXT:
        print_text(list);
        break;
    case MODE_AST:
        print_ast(list);
        break;
    }

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
