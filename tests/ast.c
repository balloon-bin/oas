#include "../src/ast.h"
#include "munit.h"

static MunitResult test_ast_node_alloc(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    ast_node_t *node = nullptr;
    error_t *err = ast_node_alloc(&node);

    munit_assert_ptr_not_null(node);
    munit_assert_ptr_null(err);

    ast_node_free(node);

    return MUNIT_OK;
}

static MunitTest ast_tests[] = {
    {"/node_alloc", test_ast_node_alloc, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {nullptr,       nullptr,             nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr}
};

const MunitSuite ast_test_suite = {"/ast", ast_tests, nullptr, 1, MUNIT_SUITE_OPTION_NONE};
