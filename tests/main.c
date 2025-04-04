#include "munit.h"

extern const MunitSuite ast_test_suite;

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    MunitSuite master_suite = {"/oas", nullptr, nullptr, 1, MUNIT_SUITE_OPTION_NONE};

    MunitSuite suites[] = {ast_test_suite, nullptr};

    master_suite.suites = suites;

    return munit_suite_main(&master_suite, nullptr, argc, argv);
}
