#ifndef INCLUDE_PARSER_COMBINATORS_H_
#define INCLUDE_PARSER_COMBINATORS_H_

#include "util.h"

typedef parse_result_t (*parser_t)(tokenlist_entry_t *);

parse_result_t parse_any(tokenlist_entry_t *current, parser_t parsers[]);

// parse as many of the giver parsers objects in a row as possible, potentially
// allowing none wraps the found objects in a new ast node with the given note
// id
parse_result_t parse_many(tokenlist_entry_t *current, node_id_t id,
                          bool allow_none, parser_t parser);

// Parse all tries to parse all parsers consecutively and if it succeeds it
// wraps the parsed nodes in a new parent node.
parse_result_t parse_consecutive(tokenlist_entry_t *current, node_id_t id,
                                 parser_t parsers[]);

#endif // INCLUDE_PARSER_COMBINATORS_H_
