#include "parser.h"
#include "../ast.h"
#include "../lexer.h"
#include "../tokenlist.h"
#include "combinators.h"
#include "primitives.h"
#include "util.h"

parse_result_t parse_number(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_octal, parse_decimal, parse_hexadecimal,
                          parse_binary, nullptr};
    parse_result_t result = parse_any(current, parsers);
    return parse_result_wrap(NODE_NUMBER, result);
}

parse_result_t parse_plus_or_minus(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_plus, parse_minus, nullptr};
    return parse_any(current, parsers);
}

parse_result_t parse_register_index(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_plus, parse_register, parse_asterisk,
                          parse_number, nullptr};
    return parse_consecutive(current, NODE_REGISTER_INDEX, parsers);
}

parse_result_t parse_register_offset(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_plus_or_minus, parse_number, nullptr};
    return parse_consecutive(current, NODE_REGISTER_OFFSET, parsers);
}

parse_result_t parse_register_expression(tokenlist_entry_t *current) {
    parse_result_t result;

    ast_node_t *expr;
    error_t *err = ast_node_alloc(&expr);
    if (err)
        return parse_error(err);
    expr->id = NODE_REGISTER_EXPRESSION;

    // <register>
    result = parse_register(current);
    if (result.err) {
        ast_node_free(expr);
        return result;
    }
    err = ast_node_add_child(expr, result.node);
    if (err) {
        ast_node_free(result.node);
        ast_node_free(expr);
        return parse_error(err);
    }
    current = result.next;

    // <register_index>?
    result = parse_register_index(current);
    if (result.err) {
        error_free(result.err);
    } else {
        err = ast_node_add_child(expr, result.node);
        if (err) {
            ast_node_free(result.node);
            ast_node_free(expr);
            return parse_error(err);
        }
        current = result.next;
    }

    // <register_offset>?
    result = parse_register_offset(current);
    if (result.err) {
        error_free(result.err);
    } else {
        err = ast_node_add_child(expr, result.node);
        if (err) {
            ast_node_free(result.node);
            ast_node_free(expr);
            return parse_error(err);
        }
        current = result.next;
    }
    return parse_success(expr, current);
}

parse_result_t parse_immediate(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_number, parse_identifier, nullptr};
    parse_result_t result = parse_any(current, parsers);
    return parse_result_wrap(NODE_IMMEDIATE, result);
}

parse_result_t parse_memory_expression(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_register_expression, parse_identifier, nullptr};
    return parse_any(current, parsers);
}

parse_result_t parse_memory(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_lbracket, parse_memory_expression,
                          parse_rbracket, nullptr};
    return parse_consecutive(current, NODE_MEMORY, parsers);
}

parse_result_t parse_operand(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_register, parse_memory, parse_immediate,
                          nullptr};
    return parse_any(current, parsers);
}

parse_result_t parse_operands(tokenlist_entry_t *current) {
    return parse_list(current, NODE_OPERANDS, true, TOKEN_COMMA, parse_operand);
}

parse_result_t parse_label(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_identifier, parse_colon, nullptr};
    return parse_consecutive(current, NODE_LABEL, parsers);
}

parse_result_t parse_section_directive(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_section, parse_identifier, nullptr};
    return parse_consecutive(current, NODE_SECTION_DIRECTIVE, parsers);
}

parse_result_t parse_directive(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_dot, parse_section_directive, nullptr};
    return parse_consecutive(current, NODE_DIRECTIVE, parsers);
}

parse_result_t parse_instruction(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_identifier, parse_operands, nullptr};
    return parse_consecutive(current, NODE_INSTRUCTION, parsers);
}

parse_result_t parse_statement(tokenlist_entry_t *current) {
    parser_t parsers[] = {parse_label, parse_directive, parse_instruction,
                          nullptr};
    return parse_any(current, parsers);
}

parse_result_t parse(tokenlist_entry_t *current) {
    return parse_many(current, NODE_PROGRAM, true, parse_statement);
}
