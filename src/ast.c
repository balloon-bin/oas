#include "ast.h"
#include "error.h"
#include <assert.h>
#include <string.h>

error_t *const err_ast_children_cap = &(error_t){
    .message = "Failed to increase ast node children, max capacity reached"};

error_t *ast_node_alloc(ast_node_t **output) {
    *output = nullptr;

    ast_node_t *node = calloc(1, sizeof(ast_node_t));
    if (node == nullptr)
        return err_allocation_failed;

    *output = node;
    return nullptr;
}

void ast_node_free_value(ast_node_t *node) {
    // TODO: decide how value ownership will work and clean it up here
}

void ast_node_free(ast_node_t *node) {
    if (node == nullptr)
        return;
    if (node->children) {
        for (size_t i = 0; i < node->len; ++i)
            ast_node_free(node->children[i]);
        free(node->children);
    }

    ast_node_free_value(node);

    memset(node, 0, sizeof(ast_node_t));
    free(node);
}

/**
 * @pre node->children must be nullptr
 */
error_t *ast_node_alloc_children(ast_node_t *node) {
    node->children = calloc(node_default_children_cap, sizeof(ast_node_t *));
    if (node->children == nullptr)
        return err_allocation_failed;

    node->cap = node_default_children_cap;
    return nullptr;
}

error_t *ast_node_grow_cap(ast_node_t *node) {
    if (node->cap >= node_max_children_cap) {
        return err_ast_children_cap;
    }

    size_t new_cap = node->cap * 2;
    if (new_cap > node_max_children_cap) {
        new_cap = node_max_children_cap;
    }

    ast_node_t **new_children =
        realloc(node->children, new_cap * sizeof(ast_node_t *));
    if (new_children == nullptr) {
        return err_allocation_failed;
    }

    node->children = new_children;
    node->cap = new_cap;

    return nullptr;
}

error_t *ast_node_add_child(ast_node_t *node, ast_node_t *child) {
    error_t *err = nullptr;
    if (node->children == nullptr)
        err = ast_node_alloc_children(node);
    else if (node->len >= node->cap)
        err = ast_node_grow_cap(node);
    if (err)
        return err;

    node->children[node->len] = child;
    node->len += 1;

    return nullptr;
}

const char *ast_node_id_to_cstr(node_id_t id) {
    switch (id) {
    case NODE_INVALID:
        return "NODE_INVALID";
    case NODE_PROGRAM:
        return "NODE_PROGRAM";
    case NODE_STATEMENT:
        return "NODE_STATEMENT";
    case NODE_LABEL:
        return "NODE_LABEL";
    case NODE_DIRECTIVE:
        return "NODE_DIRECTIVE";
    case NODE_INSTRUCTION:
        return "NODE_INSTRUCTION";
    case NODE_OPERANDS:
        return "NODE_OPERANDS";
    case NODE_OPERAND:
        return "NODE_OPERAND";
    case NODE_IMMEDIATE:
        return "NODE_IMMEDIATE";
    case NODE_MEMORY:
        return "NODE_MEMORY";
    case NODE_NUMBER:
        return "NODE_NUMBER";
    case NODE_LABEL_REFERENCE:
        return "NODE_LABEL_REFERENCE";
    case NODE_MEMORY_EXPRESSION:
        return "NODE_MEMORY_EXPRESSION";
    case NODE_REGISTER_EXPRESSION:
        return "NODE_REGISTER_EXPRESSION";
    case NODE_REGISTER_INDEX:
        return "NODE_REGISTER_INDEX";
    case NODE_REGISTER_OFFSET:
        return "NODE_REGISTER_OFFSET";
    case NODE_PLUS_OR_MINUS:
        return "NODE_PLUS_OR_MINUS";
    case NODE_SECTION_DIRECTIVE:
        return "NODE_SECTION_DIRECTIVE";
    case NODE_REGISTER:
        return "NODE_REGISTER";
    case NODE_SECTION:
        return "NODE_SECTION";
    case NODE_IDENTIFIER:
        return "NODE_IDENTIFIER";
    case NODE_DECIMAL:
        return "NODE_DECIMAL";
    case NODE_HEXADECIMAL:
        return "NODE_HEXADECIMAL";
    case NODE_OCTAL:
        return "NODE_OCTAL";
    case NODE_BINARY:
        return "NODE_BINARY";
    case NODE_CHAR:
        return "NODE_CHAR";
    case NODE_STRING:
        return "NODE_STRING";
    case NODE_COLON:
        return "NODE_COLON";
    case NODE_COMMA:
        return "NODE_COMMA";
    case NODE_LBRACKET:
        return "NODE_LBRACKET";
    case NODE_RBRACKET:
        return "NODE_RBRACKET";
    case NODE_PLUS:
        return "NODE_PLUS";
    case NODE_MINUS:
        return "NODE_MINUS";
    case NODE_ASTERISK:
        return "NODE_ASTERISK";
    case NODE_DOT:
        return "NODE_DOT";
    }
    assert(!"Unreachable, weird node id" && id);
    __builtin_unreachable();
}

static void ast_node_print_internal(ast_node_t *node, int indent) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("%s", ast_node_id_to_cstr(node->id));

    if (node->token_entry && node->token_entry->token.value) {
        printf(" \"%s\"", node->token_entry->token.value);
    }
    printf("\n");

    for (size_t i = 0; i < node->len; i++) {
        ast_node_print_internal(node->children[i], indent + 1);
    }
}

void ast_node_print(ast_node_t *node) {
    ast_node_print_internal(node, 0);
}
