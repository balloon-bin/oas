#ifndef INCLUDE_SRC_AST_H_
#define INCLUDE_SRC_AST_H_

#include "error.h"
#include "lexer.h"
#include "tokenlist.h"
#include <stddef.h>
#include <stdint.h>

extern error_t *const err_ast_children_cap;

typedef enum node_id {
    NODE_INVALID,

    NODE_PROGRAM,
    NODE_STATEMENT,
    NODE_LABEL,
    NODE_DIRECTIVE,
    NODE_INSTRUCTION,
    NODE_OPERANDS,
    NODE_OPERAND,
    NODE_IMMEDIATE,
    NODE_MEMORY,
    NODE_NUMBER,
    NODE_LABEL_REFERENCE,
    NODE_MEMORY_EXPRESSION,
    NODE_REGISTER_EXPRESSION,
    NODE_REGISTER_INDEX,
    NODE_REGISTER_OFFSET,
    NODE_PLUS_OR_MINUS,
    NODE_SECTION_DIRECTIVE,

    // Validated primitives
    NODE_REGISTER,
    NODE_SECTION,

    // Primitive nodes
    NODE_IDENTIFIER,
    NODE_DECIMAL,
    NODE_HEXADECIMAL,
    NODE_OCTAL,
    NODE_BINARY,
    NODE_CHAR,
    NODE_STRING,
    NODE_COLON,
    NODE_COMMA,
    NODE_LBRACKET,
    NODE_RBRACKET,
    NODE_PLUS,
    NODE_MINUS,
    NODE_ASTERISK,
    NODE_DOT,
} node_id_t;

typedef struct ast_node ast_node_t;

constexpr size_t node_default_children_cap = 8;
/* 65K ought to be enough for anybody */
constexpr size_t node_max_children_cap = 1 << 16;

struct ast_node {
    node_id_t id;
    tokenlist_entry_t *token_entry;
    size_t len;
    size_t cap;
    ast_node_t **children;

    union {
        struct {
            uint64_t value;
            int size;
        } integer;
        char *name;
    } value;
};

/**
 * @brief Allocates a new AST node
 *
 * Creates and initializes a new AST node with default (zero) values.
 *
 * @param[out] output Pointer to store the allocated node
 * @return error_t* nullptr on success, allocation error on failure
 */
error_t *ast_node_alloc(ast_node_t **node);

/**
 * @brief Frees an AST node and all its children recursively
 *
 * Recursively frees all children of the node, then frees the node itself.
 * If node is nullptr, the function returns without doing anything.
 *
 * @param node The node to free
 */
void ast_node_free(ast_node_t *node);

/**
 * @brief Adds a child node to a parent node
 *
 * Adds the specified child node to the parent's children array.
 * If this is the first child, the function allocates the children array.
 * If the children array is full, the function increases its capacity.
 *
 * @param node The parent node to add the child to
 * @param child The child node to add
 * @return error_t* nullptr on success, allocation error on failure,
 *                  or err_node_children_cap if maximum capacity is reached
 */
error_t *ast_node_add_child(ast_node_t *node, ast_node_t *child);

/**
 * @brief Prints an AST starting from the given node
 *
 * Prints a representation of the AST with indentation to show structure.
 * Each node's type is shown, and if a node has an associated token value,
 * that value is printed in quotes.
 *
 * @param node The root node of the AST to print
 */
void ast_node_print(ast_node_t *node);

#endif // INCLUDE_SRC_AST_H_
