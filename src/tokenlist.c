#include "tokenlist.h"
#include "error.h"
#include "lexer.h"
#include <stdlib.h>

error_t *tokenlist_alloc(tokenlist_t **output) {
    *output = nullptr;

    tokenlist_t *list = calloc(1, sizeof(tokenlist_t));
    if (list == nullptr)
        return err_allocation_failed;

    list->head = nullptr;
    list->tail = nullptr;

    *output = list;
    return nullptr;
}

error_t *tokenlist_entry_alloc(tokenlist_entry_t **output) {
    *output = nullptr;

    tokenlist_entry_t *entry = calloc(1, sizeof(tokenlist_entry_t));
    if (entry == nullptr)
        return err_allocation_failed;

    entry->next = nullptr;
    entry->prev = nullptr;

    *output = entry;
    return nullptr;
}

void tokenlist_append(tokenlist_t *list, tokenlist_entry_t *entry) {
    if (list->head == nullptr) {
        list->head = entry;
        list->tail = entry;
        entry->next = nullptr;
        entry->prev = nullptr;
    } else {
        entry->prev = list->tail;
        entry->next = nullptr;
        list->tail->next = entry;
        list->tail = entry;
    }
}

void tokenlist_entry_free(tokenlist_entry_t *entry) {
    lexer_token_cleanup(&entry->token);
    free(entry);
}

void tokenlist_free(tokenlist_t *list) {
    if (list == nullptr)
        return;

    tokenlist_entry_t *current = list->head;
    while (current) {
        tokenlist_entry_t *next = current->next;
        tokenlist_entry_free(current);
        current = next;
    }

    free(list);
}

error_t *tokenlist_fill(tokenlist_t *list, lexer_t *lex) {
    error_t *err = nullptr;
    lexer_token_t token = {};
    while ((err = lexer_next(lex, &token)) == nullptr) {
        tokenlist_entry_t *entry;
        err = tokenlist_entry_alloc(&entry);
        if (err) {
            lexer_token_cleanup(&token);
            return err;
        }
        entry->token = token;
        tokenlist_append(list, entry);
    }
    if (err != err_eof)
        return err;
    return nullptr;
}
