#include "ast.h"
#include "error.h"
#include <string.h>

error_t *err_node_children_cap = &(error_t){
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
        return err_node_children_cap;
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
