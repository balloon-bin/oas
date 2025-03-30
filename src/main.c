#include "error.h"
#include "lexer.h"

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

    lexer_t lex = {0};
    lexer_token_t token;
    error_t *err = lexer_open(&lex, filename);
    if (err) {
        puts(err->message);
        error_free(err);
        return 1;
    }

    bool keep_going = true;
    while (keep_going && (err = lexer_next(&lex, &token)) == nullptr) {
        keep_going = print_fn(&token);
        free(token.value);
    }

    if (err && err != err_eof) {
        puts(err->message);
    }
    error_free(err);
    return 0;
}
