#ifndef INCLUDE_ENCODER_SYMBOLS_H_
#define INCLUDE_ENCODER_SYMBOLS_H_

#include "../ast.h"

extern error_t *const err_symbol_table_invalid_node;
extern error_t *const err_symbol_table_max_cap;
extern error_t *const err_symbol_table_incompatible_symbols;

typedef enum symbol_kind {
    SYMBOL_REFERENCE,
    SYMBOL_LOCAL,
    SYMBOL_EXPORT,
    SYMBOL_IMPORT,
} symbol_kind_t;

/**
 * Represent a symbol in the program
 *
 * Symbols with the same name can only be in the table once. IMPORT or EXPORT
 * symbols take precedence over REFERENCE symbols. If any reference symbols
 * remain after the first encoding pass this indicates an error. Trying to add
 * an IMPORT or EXPORT symbol if the same name already exists as the other kind
 * is an error.
 *
 * This symbol table never taken ownership of the name string, it's lifted
 * straight from the node->token.value.
 */
typedef struct symbol {
    char *name;
    symbol_kind_t kind;
    ast_node_t *node;
} symbol_t;

typedef struct symbol_table {
    size_t cap;
    size_t len;
    symbol_t *symbols;
} symbol_table_t;

error_t *symbol_table_alloc(symbol_table_t **table);
void symbol_table_free(symbol_table_t *table);
error_t *symbol_table_update(symbol_table_t *table, ast_node_t *node);
symbol_t *symbol_table_lookup(symbol_table_t *table, const char *name);

#endif // INCLUDE_ENCODER_SYMBOLS_H_
