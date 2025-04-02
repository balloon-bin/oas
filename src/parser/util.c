#include "util.h"
#include "../tokenlist.h"

error_t *err_parse_no_match =
    &(error_t){.message = "parsing failed to find the correct token sequence"};

parse_result_t parse_error(error_t *err) {
    return (parse_result_t){.err = err};
}

parse_result_t parse_no_match() {
    return parse_error(err_parse_no_match);
}

parse_result_t parse_success(ast_node_t *ast, tokenlist_entry_t *next) {
    next = tokenlist_skip_trivia(next);
    return (parse_result_t){.node = ast, .next = next};
}

parse_result_t parse_token(tokenlist_entry_t *current,
                           lexer_token_id_t token_id, node_id_t ast_id,
                           token_validator_t is_valid) {
    if (current->token.id != token_id ||
        (is_valid && !is_valid(&current->token)))
        return parse_no_match();

    ast_node_t *node;
    error_t *err = ast_node_alloc(&node);
    if (err)
        return parse_error(err);
    node->id = ast_id;
    node->token_entry = current;

    return parse_success(node, current->next);
}

parse_result_t parse_result_wrap(node_id_t id, parse_result_t result) {
    if (result.err)
        return result;

    ast_node_t *node;
    error_t *err = ast_node_alloc(&node);
    if (err) {
        ast_node_free(result.node);
        return parse_error(err);
    }
    node->id = id;

    err = ast_node_add_child(node, result.node);
    if (err) {
        ast_node_free(result.node);
        return parse_error(err);
    }

    return parse_success(node, result.next);
}
