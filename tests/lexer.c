#include "../src/lexer.h"
#include "../src/error.h"
#include "munit.h"
#include <string.h>

void lexer_setup_memory_test(lexer_t *lex, const char *input) {
    munit_assert_null(lex->fp);
    FILE *stream = fmemopen((void *)input, strlen(input), "r");
    munit_assert_not_null(stream);
    lex->fp = stream;
    lex->line_number = 0;
    lex->character_number = 0;
    lex->buffer_count = 0;
}

void lexer_expect_one_token(lexer_t *lex, lexer_token_id_t id, const char *value, size_t line, size_t column) {
    lexer_token_t token = {};

    error_t *err = lexer_next(lex, &token);
    munit_assert_null(err);

    munit_assert_int(token.id, ==, id);
    munit_assert_string_equal(token.value, value);
    munit_assert_int(token.line_number, ==, 0);
    munit_assert_int(token.character_number, ==, 0);
    lexer_token_cleanup(&token);
}

void lexer_expect_eof(lexer_t *lex) {
    lexer_token_t token = {};
    error_t *err = lexer_next(lex, &token);
    munit_assert_ptr_equal(err, err_eof);
}

void lexer_test_one_token(lexer_token_id_t id, const char *value) {
    lexer_t lex = {};
    lexer_setup_memory_test(&lex, value);
    lexer_expect_one_token(&lex, id, value, 0, 0);
    lexer_expect_eof(&lex);
    lexer_close(&lex);
}

MunitResult test_lexer_identifier(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_IDENTIFIER, "identifier");
    lexer_test_one_token(TOKEN_IDENTIFIER, "_identifier");
    lexer_test_one_token(TOKEN_IDENTIFIER, "_identifier123_55");
    return MUNIT_OK;
}

MunitTest lexer_tests[] = {
    {"/identifier", test_lexer_identifier, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL,          NULL,                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite lexer_test_suite = {"/lexer", lexer_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE};
