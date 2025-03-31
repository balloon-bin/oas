#ifndef INCLUDE_SRC_TOKENLIST_H_
#define INCLUDE_SRC_TOKENLIST_H_
#include "lexer.h"

typedef struct tokenlist_entry tokenlist_entry_t;

struct tokenlist_entry {
    lexer_token_t token;
    tokenlist_entry_t *next;
    tokenlist_entry_t *prev;
};

typedef struct tokenlist {
    tokenlist_entry_t *head;
    tokenlist_entry_t *tail;
} tokenlist_t;

/**
 * @brief Allocate a new doubly linked list of lexer tokens
 */
error_t *tokenlist_alloc(tokenlist_t **list);

/**
 * Consume all tokens from the lexer and add them to the list
 */
error_t *tokenlist_fill(tokenlist_t *list, lexer_t *lex);

void tokenlist_free(tokenlist_t *list);

#endif // INCLUDE_SRC_TOKENLIST_H_
