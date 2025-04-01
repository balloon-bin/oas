#ifndef INCLUDE_PARSER_PRIMITIVES_H_
#define INCLUDE_PARSER_PRIMITIVES_H_

#include "util.h"

parse_result_t parse_identifier(tokenlist_entry_t *current);
parse_result_t parse_decimal(tokenlist_entry_t *current);
parse_result_t parse_hexadecimal(tokenlist_entry_t *current);
parse_result_t parse_binary(tokenlist_entry_t *current);
parse_result_t parse_octal(tokenlist_entry_t *current);
parse_result_t parse_string(tokenlist_entry_t *current);
parse_result_t parse_char(tokenlist_entry_t *current);
parse_result_t parse_colon(tokenlist_entry_t *current);
parse_result_t parse_comma(tokenlist_entry_t *current);
parse_result_t parse_lbracket(tokenlist_entry_t *current);
parse_result_t parse_rbracket(tokenlist_entry_t *current);
parse_result_t parse_plus(tokenlist_entry_t *current);
parse_result_t parse_minus(tokenlist_entry_t *current);
parse_result_t parse_asterisk(tokenlist_entry_t *current);
parse_result_t parse_dot(tokenlist_entry_t *current);
parse_result_t parse_label_reference(tokenlist_entry_t *current);

/* These are "primitives" with a different name and some extra validation on top
 * for example, register is just an identifier but it only matches a limited set
 * of values
 */
parse_result_t parse_register(tokenlist_entry_t *current);
parse_result_t parse_section(tokenlist_entry_t *current);

#endif // INCLUDE_PARSER_PRIMITIVES_H_
