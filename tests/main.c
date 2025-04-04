#include "munit.h"

extern MunitTest ast_tests[];
extern MunitTest lexer_tests[];

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    MunitSuite suites[] = {
        {"/ast",   ast_tests,   nullptr, 1, MUNIT_SUITE_OPTION_NONE},
        {"/lexer", lexer_tests, nullptr, 1, MUNIT_SUITE_OPTION_NONE},
        {nullptr,  nullptr,     nullptr, 0, MUNIT_SUITE_OPTION_NONE},
    };

    MunitSuite master_suite = {"/oas", nullptr, suites, 1, MUNIT_SUITE_OPTION_NONE};

    return munit_suite_main(&master_suite, nullptr, argc, argv);
}
