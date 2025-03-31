#include "error.h"
#include "lexer.h"
#include "tokenlist.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool print_token(lexer_token_t *token) {
    lexer_token_print(token);
    return true;
}

bool print_value(lexer_token_t *token) {
    if (token->id == TOKEN_ERROR) {
        printf("%s\n", token->value);
        for (size_t i = 0; i < token->character_number; ++i)
            printf(" ");
        printf("^-- %s\n", token->explanation);
    } else {
        printf("%s", token->value);
    }
    return token->id != TOKEN_ERROR;
}

int main(int argc, char *argv[]) {
    if (argc != 3 ||
        (strcmp(argv[1], "-tokens") != 0 && strcmp(argv[1], "-text") != 0)) {
        puts("Usage: oas -tokens <filename>");
        puts("Usage: oas -text <filename>");
        return 1;
    }

    bool (*print_fn)(lexer_token_t *);
    char *filename = argv[2];
    if (strcmp(argv[1], "-tokens") == 0) {
        print_fn = print_token;
    } else {
        print_fn = print_value;
    }

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

    for (auto entry = list->head; entry; entry = entry->next) {
        print_fn(&entry->token);
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
