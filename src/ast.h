#ifndef INCLUDE_SRC_AST_H_
#define INCLUDE_SRC_AST_H_

#include "data/registers.h"
#include "error.h"
#include "lexer.h"
#include "tokenlist.h"
#include <assert.h>
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
    NODE_IMPORT_DIRECTIVE,
    NODE_EXPORT_DIRECTIVE,

    // Validated primitives
    NODE_REGISTER,
    NODE_SECTION,
    NODE_IMPORT,
    NODE_EXPORT,

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
    NODE_NEWLINE,
} node_id_t;

typedef struct ast_node ast_node_t;

constexpr size_t node_default_children_cap = 8;
/* 65K ought to be enough for anybody */
constexpr size_t node_max_children_cap = 1 << 16;

typedef struct number {
    uint64_t value;
    operand_size_t size;
} number_t;

typedef struct register_ {
    register_id_t id;
    operand_size_t size;
} register_t;

typedef struct opcode_encoding {
    uint8_t buffer[32];
    size_t len;
} opcode_encoding_t;

typedef struct instruction {
    bool has_reference;
    opcode_encoding_t encoding;
    int64_t address;
} instruction_t;

typedef struct reference {
    int64_t offset;
    int64_t address;
    operand_size_t size;
} reference_t;

typedef struct {
    int64_t address;
} label_t;

struct ast_node {
    node_id_t id;
    tokenlist_entry_t *token_entry;
    size_t len;
    size_t cap;
    ast_node_t **children;

    union {
        register_t reg;
        number_t number;
        instruction_t instruction;
        reference_t reference;
        label_t label;
    } value;
};

static inline register_t *ast_node_register_value(ast_node_t *node) {
    assert(node->id == NODE_REGISTER);
    return &node->value.reg;
}

static inline number_t *ast_node_number_value(ast_node_t *node) {
    assert(node->id == NODE_NUMBER);
    return &node->value.number;
}

static inline instruction_t *ast_node_instruction_value(ast_node_t *node) {
    assert(node->id == NODE_INSTRUCTION);
    return &node->value.instruction;
}

static inline reference_t *ast_node_reference_value(ast_node_t *node) {
    assert(node->id == NODE_LABEL_REFERENCE);
    return &node->value.reference;
}

static inline label_t *ast_node_label_value(ast_node_t *node) {
    assert(node->id == NODE_LABEL);
    return &node->value.label;
}

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

/**
 * Prune the children with a given id
 *
 * The tree is recursively visited and all child nodes of a given ID are pruned
 * completely. If a node has the giver id, it will get removed along wih all its
 * children, even if some of those children have different ids. The root node id
 * is never checked so the tree is guaranteed to remain and allocated valid.
 *
 * @param node The root of the tree you want to prune
 * @param id The id of the nodes you want to prune
 */
void ast_node_prune(ast_node_t *node, node_id_t id);

#endif // INCLUDE_SRC_AST_H_
