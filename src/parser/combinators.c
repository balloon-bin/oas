#include "combinators.h"

parse_result_t parse_any(tokenlist_entry_t *current, parser_t parsers[]) {
    parser_t parser;
    while ((parser = *parsers++)) {
        parse_result_t result = parser(current);
        if (result.err == nullptr)
            return result;
    }
    return parse_no_match();
}

// parse as many of the giver parsers objects in a row as possible, potentially
// allowing none wraps the found objects in a new ast node with the given note
// id
parse_result_t parse_many(tokenlist_entry_t *current, node_id_t id,
                          bool allow_none, parser_t parser) {
    ast_node_t *many;
    error_t *err = ast_node_alloc(&many);
    parse_result_t result;
    if (err)
        return parse_error(err);
    many->id = id;

    while (current) {
        result = parser(current);
        if (result.err == err_parse_no_match)
            break;
        if (result.err) {
            ast_node_free(many);
            return result;
        }
        err = ast_node_add_child(many, result.node);
        if (err) {
            ast_node_free(many);
            ast_node_free(result.node);
            return parse_error(err);
        }
        current = result.next;
    }

    if (!allow_none && many->len == 0) {
        ast_node_free(many);
        return parse_no_match();
    }
    return parse_success(many, current);
}

// Parse all tries to parse all parsers consecutively and if it succeeds it
// wraps the parsed nodes in a new parent node.
parse_result_t parse_consecutive(tokenlist_entry_t *current, node_id_t id,
                                 parser_t parsers[]) {
    ast_node_t *all;
    error_t *err = ast_node_alloc(&all);
    parse_result_t result;
    if (err)
        return parse_error(err);

    all->id = id;

    parser_t parser;
    while ((parser = *parsers++) && current) {
        result = parser(current);
        if (result.err) {
            ast_node_free(all);
            return result;
        }
        err = ast_node_add_child(all, result.node);
        if (err) {
            ast_node_free(result.node);
            ast_node_free(all);
            return parse_error(err);
        }
        current = result.next;
    }
    return parse_success(all, current);
}
