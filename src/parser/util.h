#ifndef INCLUDE_PARSER_UTIL_H_
#define INCLUDE_PARSER_UTIL_H_

#include "../ast.h"
#include "../error.h"
#include "../tokenlist.h"

typedef struct parse_result {
    error_t *err;
    tokenlist_entry_t *next;
    ast_node_t *node;
} parse_result_t;

typedef bool (*token_validator_t)(lexer_token_t *);

parse_result_t parse_error(error_t *err);
parse_result_t parse_no_match();
parse_result_t parse_success(ast_node_t *ast, tokenlist_entry_t *next);
parse_result_t parse_token(tokenlist_entry_t *current,
                           lexer_token_id_t token_id, node_id_t ast_id,
                           token_validator_t is_valid);
parse_result_t parse_result_wrap(node_id_t id, parse_result_t result);

extern error_t *const err_parse_no_match;

#endif // INCLUDE_PARSER_UTIL_H_
