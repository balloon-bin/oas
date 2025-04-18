#include "symbols.h"
#include "../error.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

constexpr size_t symbol_table_default_cap = 64;
constexpr size_t symbol_table_max_cap = 1 << 16;

error_t *const err_symbol_table_invalid_node = &(error_t){
    .message = "Unexpected node id when adding symbol to symbol table"};
error_t *const err_symbol_table_max_cap = &(error_t){
    .message = "Failed to increase symbol table length, max capacity reached"};
error_t *const err_symbol_table_incompatible_symbols =
    &(error_t){.message = "Failed to update symbol with incompatible kind"};

error_t *symbol_table_alloc(symbol_table_t **output) {
    *output = nullptr;

    symbol_table_t *table = calloc(1, sizeof(symbol_table_t));
    if (table == nullptr)
        return err_allocation_failed;

    table->symbols = calloc(symbol_table_default_cap, sizeof(symbol_t));
    if (table->symbols == nullptr) {
        free(table);
        return err_allocation_failed;
    }

    table->cap = symbol_table_default_cap;
    table->len = 0;

    *output = table;
    return nullptr;
}

void symbol_table_free(symbol_table_t *table) {
    free(table->symbols);
    free(table);
}

error_t *symbol_table_grow_cap(symbol_table_t *table) {
    if (table->cap >= symbol_table_max_cap)
        return err_symbol_table_max_cap;

    size_t new_cap = table->cap * 2;
    symbol_t *new_symbols = realloc(table->symbols, new_cap * sizeof(symbol_t));
    if (new_symbols == nullptr)
        return err_allocation_failed;

    table->symbols = new_symbols;
    table->cap = new_cap;

    return nullptr;
}

error_t *symbol_table_get_node_info(ast_node_t *node, symbol_kind_t *kind,
                                    char **name) {
    switch (node->id) {
    case NODE_LABEL:
        *kind = SYMBOL_LOCAL;
        *name = node->children[0]->token_entry->token.value;
        return nullptr;
    case NODE_LABEL_REFERENCE:
        *kind = SYMBOL_REFERENCE;
        *name = node->token_entry->token.value;
        return nullptr;
    case NODE_IMPORT_DIRECTIVE:
        *kind = SYMBOL_IMPORT;
        *name = node->children[1]->token_entry->token.value;
        return nullptr;
    case NODE_EXPORT_DIRECTIVE:
        *kind = SYMBOL_EXPORT;
        *name = node->children[1]->token_entry->token.value;
        return nullptr;
    default:
        return err_symbol_table_invalid_node;
    }
    __builtin_unreachable();
}

/*
old  \  new  | REFERENCE | LOCAL    | IMPORT   | EXPORT   |
-------------|-----------|----------|----------|----------|
REFERENCE    |           | replace  | replace  | replace  |
-------------|-----------|----------|----------|----------|
LOCAL        |           |          |   ERR    | replace  |
-------------|-----------|----------|----------|----------|
IMPORT       |           |          |          |   ERR    |
-------------|-----------|----------|----------|----------|
EXPORT       |           |          |   ERR    |          |
-------------|-----------|----------|----------|----------|
*/

bool symbol_table_should_upgrade(symbol_kind_t old, symbol_kind_t new) {
    if (old == SYMBOL_REFERENCE)
        return new != SYMBOL_REFERENCE;
    if (old == SYMBOL_LOCAL)
        return new == SYMBOL_EXPORT;
    return false;
}

bool symbol_table_should_error(symbol_kind_t old, symbol_kind_t new) {
    if (new == SYMBOL_IMPORT)
        return old == SYMBOL_LOCAL || old == SYMBOL_EXPORT;
    if (new == SYMBOL_EXPORT)
        return old == SYMBOL_IMPORT;
    return false;
}

/**
 * @pre The symbol _must not_ already be in the table.
 */
error_t *symbol_table_add(symbol_table_t *table, char *name, symbol_kind_t kind,
                          ast_node_t *statement) {
    if (table->len >= table->cap) {
        error_t *err = symbol_table_grow_cap(table);
        if (err)
            return err;
    }

    table->symbols[table->len] = (symbol_t){
        .name = name,
        .kind = kind,
        .statement = statement,
    };

    table->len += 1;

    return nullptr;
}

error_t *symbol_table_update(symbol_table_t *table, ast_node_t *node,
                             ast_node_t *statement) {
    char *name;
    symbol_kind_t kind;
    error_t *err = symbol_table_get_node_info(node, &kind, &name);
    if (err)
        return err;

    if (kind != SYMBOL_LOCAL)
        statement = nullptr;

    symbol_t *symbol = symbol_table_lookup(table, name);
    if (!symbol)
        return symbol_table_add(table, name, kind, statement);
    if (symbol_table_should_error(symbol->kind, kind))
        return err_symbol_table_incompatible_symbols;
    if (symbol_table_should_upgrade(symbol->kind, kind)) {
        symbol->kind = kind;
    }

    if (kind == SYMBOL_LOCAL && symbol->statement == nullptr)
        symbol->statement = statement;

    return nullptr;
}

symbol_t *symbol_table_lookup(symbol_table_t *table, const char *name) {
    for (size_t i = 0; i < table->len; ++i) {
        if (strcmp(table->symbols[i].name, name) == 0)
            return &table->symbols[i];
    }
    return nullptr;
}
