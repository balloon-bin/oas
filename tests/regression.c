#include "../src/ast.h"
#include "../src/parser/parser.h"
#include "munit.h"

MunitResult test_regression_trivia_head(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    lexer_t *lex = &(lexer_t){};
    error_t *err = lexer_open(lex, "tests/input/regression/test_trivia_head.asm");
    munit_assert_null(err);

    tokenlist_t *list;
    err = tokenlist_alloc(&list);
    munit_assert_null(err);

    err = tokenlist_fill(list, lex);
    munit_assert_null(err);

    parse_result_t result = parse(list->head);
    munit_assert_null(result.err);
    munit_assert_null(result.next);

    ast_node_free(result.node);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_no_operands_eof(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    lexer_t *lex = &(lexer_t){};
    error_t *err = lexer_open(lex, "tests/input/regression/test_no_operands_eof.asm");
    munit_assert_null(err);

    tokenlist_t *list;
    err = tokenlist_alloc(&list);
    munit_assert_null(err);

    err = tokenlist_fill(list, lex);
    munit_assert_null(err);

    parse_result_t result = parse(list->head);
    munit_assert_null(result.err);
    munit_assert_null(result.next);

    // Both children should be instructions
    munit_assert_size(result.node->len, ==, 2);
    munit_assert_int(result.node->children[0]->id, ==, NODE_INSTRUCTION);
    munit_assert_int(result.node->children[1]->id, ==, NODE_INSTRUCTION);

    // And they should have empty operands
    munit_assert_size(result.node->children[0]->len, ==, 2);
    munit_assert_size(result.node->children[1]->len, ==, 2);
    munit_assert_size(result.node->children[0]->children[1]->len, ==, 0);
    munit_assert_size(result.node->children[1]->children[1]->len, ==, 0);

    ast_node_free(result.node);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitTest regression_tests[] = {
    {"/trivia_head",     test_regression_trivia_head, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/no_operands_eof", test_no_operands_eof,        nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {nullptr,            nullptr,                     nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr}
};
