#include "../src/lexer.h"
#include "../src/error.h"
#include "munit.h"
#include <string.h>

void lexer_setup_memory_test(lexer_t *lex, const char *input) {
    munit_assert_null(lex->fp);
    FILE *stream = fmemopen((void *)input, strlen(input), "rb");
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
    munit_assert_int(token.line_number, ==, line);
    munit_assert_int(token.character_number, ==, column);
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

typedef struct token_data {
    lexer_token_id_t id;
    char *value;
    size_t line;
    size_t column;
} token_data_t;

typedef struct boundary {
    const char *input;
    token_data_t first;
    token_data_t second;
} boundary_t;

void test_lexer_boundary(boundary_t boundaries[]) {
    for (size_t i = 0; boundaries[i].input; ++i) {
        auto boundary = boundaries[i];
        auto first = boundary.first;
        auto second = boundary.second;

        lexer_t lex = {};
        lexer_setup_memory_test(&lex, boundary.input);
        lexer_expect_one_token(&lex, first.id, first.value, first.line, first.column);
        lexer_expect_one_token(&lex, second.id, second.value, second.line, second.column);
        lexer_expect_eof(&lex);
        lexer_close(&lex);
    }
}

MunitResult test_lexer_identifier_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"id:",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_COLON, ":", 0, 2}         },
        {"id[",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_LBRACKET, "[", 0, 2}      },
        {"id]",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_RBRACKET, "]", 0, 2}      },
        {"id+",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_PLUS, "+", 0, 2}          },
        {"id-",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_MINUS, "-", 0, 2}         },
        {"id*",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_ASTERISK, "*", 0, 2}      },
        {"id.",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_DOT, ".", 0, 2}           },
        {"id;comment", {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_COMMENT, ";comment", 0, 2}},
        {"id\n",       {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 2}      },
        {"id\r\n",     {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 2}    },
        {"id ",        {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 2}    },
        {"id\t",       {TOKEN_IDENTIFIER, "id", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 2}   },
        {nullptr,      {},                             {}                               },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_decimal(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_DECIMAL, "123");
    lexer_test_one_token(TOKEN_DECIMAL, "0");
    lexer_test_one_token(TOKEN_DECIMAL, "42");
    return MUNIT_OK;
}

MunitResult test_lexer_decimal_with_suffix(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_DECIMAL, "123:8");
    lexer_test_one_token(TOKEN_DECIMAL, "0:16");
    lexer_test_one_token(TOKEN_DECIMAL, "42:32");
    lexer_test_one_token(TOKEN_DECIMAL, "69:64");
    return MUNIT_OK;
}

MunitResult test_lexer_hexadecimal(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0x123");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0xDEAD");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0x0");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0xabcdef");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0xABCDEF");
    return MUNIT_OK;
}

MunitResult test_lexer_hexadecimal_with_suffix(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0x123:8");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0xDEAD:16");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0xABC:32");
    lexer_test_one_token(TOKEN_HEXADECIMAL, "0xffff:64");
    return MUNIT_OK;
}

MunitResult test_lexer_octal(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_OCTAL, "0o777");
    lexer_test_one_token(TOKEN_OCTAL, "0o0");
    lexer_test_one_token(TOKEN_OCTAL, "0o123");
    return MUNIT_OK;
}

MunitResult test_lexer_octal_with_suffix(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_OCTAL, "0o777:8");
    lexer_test_one_token(TOKEN_OCTAL, "0o123:16");
    lexer_test_one_token(TOKEN_OCTAL, "0o777:32");
    lexer_test_one_token(TOKEN_OCTAL, "0o123:64");
    return MUNIT_OK;
}

MunitResult test_lexer_binary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_BINARY, "0b101");
    lexer_test_one_token(TOKEN_BINARY, "0b0");
    lexer_test_one_token(TOKEN_BINARY, "0b1");
    lexer_test_one_token(TOKEN_BINARY, "0b01010101");
    return MUNIT_OK;
}

MunitResult test_lexer_binary_with_suffix(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_BINARY, "0b101:8");
    lexer_test_one_token(TOKEN_BINARY, "0b0:16");
    lexer_test_one_token(TOKEN_BINARY, "0b1:32");
    lexer_test_one_token(TOKEN_BINARY, "0b01010101:64");
    return MUNIT_OK;
}

MunitResult test_lexer_colon(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_COLON, ":");
    return MUNIT_OK;
}

MunitResult test_lexer_comma(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_COMMA, ",");
    return MUNIT_OK;
}

MunitResult test_lexer_lbracket(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_LBRACKET, "[");
    return MUNIT_OK;
}

MunitResult test_lexer_rbracket(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_RBRACKET, "]");
    return MUNIT_OK;
}

MunitResult test_lexer_plus(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_PLUS, "+");
    return MUNIT_OK;
}

MunitResult test_lexer_minus(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_MINUS, "-");
    return MUNIT_OK;
}

MunitResult test_lexer_asterisk(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_ASTERISK, "*");
    return MUNIT_OK;
}

MunitResult test_lexer_dot(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_DOT, ".");
    return MUNIT_OK;
}

MunitResult test_lexer_comment(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_COMMENT, ";This is a comment");
    lexer_test_one_token(TOKEN_COMMENT, "; Another comment");
    lexer_test_one_token(TOKEN_COMMENT, ";");
    return MUNIT_OK;
}

MunitResult test_lexer_whitespace(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    lexer_test_one_token(TOKEN_WHITESPACE, " ");
    lexer_test_one_token(TOKEN_WHITESPACE, "  ");
    lexer_test_one_token(TOKEN_WHITESPACE, "\t");
    lexer_test_one_token(TOKEN_WHITESPACE, " \t ");
    return MUNIT_OK;
}

MunitResult test_lexer_newlines(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    // Test simple newline
    lexer_t lex = {};
    lexer_setup_memory_test(&lex, "\n");
    lexer_expect_one_token(&lex, TOKEN_NEWLINE, "\n", 0, 0);
    lexer_expect_eof(&lex);
    lexer_close(&lex);

    // Test Windows-style newline
    lexer_t lex2 = {};
    lexer_setup_memory_test(&lex2, "\r\n");
    lexer_expect_one_token(&lex2, TOKEN_NEWLINE, "\r\n", 0, 0);
    lexer_expect_eof(&lex2);
    lexer_close(&lex2);

    return MUNIT_OK;
}

MunitResult test_lexer_line_numbers(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    lexer_t lex = {};
    lexer_setup_memory_test(&lex, "a\nb\nc");

    lexer_expect_one_token(&lex, TOKEN_IDENTIFIER, "a", 0, 0);
    lexer_expect_one_token(&lex, TOKEN_NEWLINE, "\n", 0, 1);
    lexer_expect_one_token(&lex, TOKEN_IDENTIFIER, "b", 1, 0);
    lexer_expect_one_token(&lex, TOKEN_NEWLINE, "\n", 1, 1);
    lexer_expect_one_token(&lex, TOKEN_IDENTIFIER, "c", 2, 0);
    lexer_expect_eof(&lex);
    lexer_close(&lex);

    return MUNIT_OK;
}

MunitResult test_lexer_decimal_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"123,",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_COMMA, ",", 0, 3}      },
        {"123:",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_COLON, ":", 0, 3}      },
        {"123[",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_LBRACKET, "[", 0, 3}   },
        {"123]",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_RBRACKET, "]", 0, 3}   },
        {"123+",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_PLUS, "+", 0, 3}       },
        {"123-",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_MINUS, "-", 0, 3}      },
        {"123*",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_ASTERISK, "*", 0, 3}   },
        {"123.",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_DOT, ".", 0, 3}        },
        {"123;",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_COMMENT, ";", 0, 3}    },
        {"123\n",   {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 3}   },
        {"123\r\n", {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 3} },
        {"123 ",    {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 3} },
        {"123\t",   {TOKEN_DECIMAL, "123", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 3}},
        {nullptr,   {},                           {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_hexadecimal_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"0x123,",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_COMMA, ",", 0, 5}      },
        {"0x123:",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_COLON, ":", 0, 5}      },
        {"0x123[",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_LBRACKET, "[", 0, 5}   },
        {"0x123]",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_RBRACKET, "]", 0, 5}   },
        {"0x123+",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_PLUS, "+", 0, 5}       },
        {"0x123-",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_MINUS, "-", 0, 5}      },
        {"0x123*",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_ASTERISK, "*", 0, 5}   },
        {"0x123.",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_DOT, ".", 0, 5}        },
        {"0x123;",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_COMMENT, ";", 0, 5}    },
        {"0x123\n",   {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 5}   },
        {"0x123\r\n", {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 5} },
        {"0x123 ",    {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 5} },
        {"0x123\t",   {TOKEN_HEXADECIMAL, "0x123", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 5}},
        {nullptr,     {},                                 {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_octal_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"0o123,",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_COMMA, ",", 0, 5}      },
        {"0o123:",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_COLON, ":", 0, 5}      },
        {"0o123[",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_LBRACKET, "[", 0, 5}   },
        {"0o123]",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_RBRACKET, "]", 0, 5}   },
        {"0o123+",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_PLUS, "+", 0, 5}       },
        {"0o123-",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_MINUS, "-", 0, 5}      },
        {"0o123*",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_ASTERISK, "*", 0, 5}   },
        {"0o123.",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_DOT, ".", 0, 5}        },
        {"0o123;",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_COMMENT, ";", 0, 5}    },
        {"0o123\n",   {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 5}   },
        {"0o123\r\n", {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 5} },
        {"0o123 ",    {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 5} },
        {"0o123\t",   {TOKEN_OCTAL, "0o123", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 5}},
        {nullptr,     {},                           {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_binary_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"0b101,",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_COMMA, ",", 0, 5}      },
        {"0b101:",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_COLON, ":", 0, 5}      },
        {"0b101[",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_LBRACKET, "[", 0, 5}   },
        {"0b101]",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_RBRACKET, "]", 0, 5}   },
        {"0b101+",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_PLUS, "+", 0, 5}       },
        {"0b101-",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_MINUS, "-", 0, 5}      },
        {"0b101*",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_ASTERISK, "*", 0, 5}   },
        {"0b101.",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_DOT, ".", 0, 5}        },
        {"0b101;",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_COMMENT, ";", 0, 5}    },
        {"0b101\n",   {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 5}   },
        {"0b101\r\n", {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 5} },
        {"0b101 ",    {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 5} },
        {"0b101\t",   {TOKEN_BINARY, "0b101", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 5}},
        {nullptr,     {},                            {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_colon_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {":,",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {"::",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {":[",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {":]",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {":+",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {":-",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {":*",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {":.",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {":;",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {":\n",   {TOKEN_COLON, ":", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {":\r\n", {TOKEN_COLON, ":", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {": ",    {TOKEN_COLON, ":", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {":\t",   {TOKEN_COLON, ":", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                       {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_comma_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {",,",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {",:",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {",[",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {",]",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {",+",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {",-",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {",*",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {",.",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {",;",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {",\n",   {TOKEN_COMMA, ",", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {",\r\n", {TOKEN_COMMA, ",", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {", ",    {TOKEN_COMMA, ",", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {",\t",   {TOKEN_COMMA, ",", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                       {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_lbracket_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"[,",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {"[:",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {"[[",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {"[]",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {"[+",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {"[-",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {"[*",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {"[.",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {"[;",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {"[\n",   {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {"[\r\n", {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {"[ ",    {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {"[\t",   {TOKEN_LBRACKET, "[", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                          {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_rbracket_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"],",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {"]:",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {"][",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {"]]",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {"]+",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {"]-",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {"]*",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {"].",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {"];",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {"]\n",   {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {"]\r\n", {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {"] ",    {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {"]\t",   {TOKEN_RBRACKET, "]", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                          {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_plus_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"+,",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {"+:",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {"+[",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {"+]",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {"++",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {"+-",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {"+*",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {"+.",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {"+;",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {"+\n",   {TOKEN_PLUS, "+", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {"+\r\n", {TOKEN_PLUS, "+", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {"+ ",    {TOKEN_PLUS, "+", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {"+\t",   {TOKEN_PLUS, "+", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                      {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_minus_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"-,",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {"-:",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {"-[",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {"-]",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {"-+",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {"--",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {"-*",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {"-.",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {"-;",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {"-\n",   {TOKEN_MINUS, "-", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {"-\r\n", {TOKEN_MINUS, "-", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {"- ",    {TOKEN_MINUS, "-", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {"-\t",   {TOKEN_MINUS, "-", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                       {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_asterisk_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"*,",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {"*:",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {"*[",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {"*]",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {"*+",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {"*-",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {"**",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {"*.",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {"*;",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {"*\n",   {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {"*\r\n", {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {"* ",    {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {"*\t",   {TOKEN_ASTERISK, "*", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                          {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_dot_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {".,",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_COMMA, ",", 0, 1}      },
        {".:",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_COLON, ":", 0, 1}      },
        {".[",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}   },
        {".]",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}   },
        {".+",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_PLUS, "+", 0, 1}       },
        {".-",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_MINUS, "-", 0, 1}      },
        {".*",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}   },
        {"..",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_DOT, ".", 0, 1}        },
        {".;",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}    },
        {".\n",   {TOKEN_DOT, ".", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}   },
        {".\r\n", {TOKEN_DOT, ".", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1} },
        {". ",    {TOKEN_DOT, ".", 0, 0}, {TOKEN_WHITESPACE, " ", 0, 1} },
        {".\t",   {TOKEN_DOT, ".", 0, 0}, {TOKEN_WHITESPACE, "\t", 0, 1}},
        {nullptr, {},                     {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_comment_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {";comment\n",   {TOKEN_COMMENT, ";comment", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 8}  },
        {";comment\r\n", {TOKEN_COMMENT, ";comment", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 8}},
        {nullptr,        {},                                {}                           },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_whitespace_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {" ,",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_COMMA, ",", 0, 1}     },
        {" :",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_COLON, ":", 0, 1}     },
        {" [",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_LBRACKET, "[", 0, 1}  },
        {" ]",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_RBRACKET, "]", 0, 1}  },
        {" +",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_PLUS, "+", 0, 1}      },
        {" -",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_MINUS, "-", 0, 1}     },
        {" *",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_ASTERISK, "*", 0, 1}  },
        {" .",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_DOT, ".", 0, 1}       },
        {" ;",    {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_COMMENT, ";", 0, 1}   },
        {" \n",   {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 1}  },
        {" \r\n", {TOKEN_WHITESPACE, " ", 0, 0}, {TOKEN_NEWLINE, "\r\n", 0, 1}},
        {nullptr, {},                            {}                           },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_newline_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"\n,",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_COMMA, ",", 1, 0}      },
        {"\n:",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_COLON, ":", 1, 0}      },
        {"\n[",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_LBRACKET, "[", 1, 0}   },
        {"\n]",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_RBRACKET, "]", 1, 0}   },
        {"\n+",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_PLUS, "+", 1, 0}       },
        {"\n-",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_MINUS, "-", 1, 0}      },
        {"\n*",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_ASTERISK, "*", 1, 0}   },
        {"\n.",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_DOT, ".", 1, 0}        },
        {"\n;",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_COMMENT, ";", 1, 0}    },
        {"\n\n",   {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_NEWLINE, "\n", 1, 0}   },
        {"\n\r\n", {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_NEWLINE, "\r\n", 1, 0} },
        {"\n ",    {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_WHITESPACE, " ", 1, 0} },
        {"\n\t",   {TOKEN_NEWLINE, "\n", 0, 0}, {TOKEN_WHITESPACE, "\t", 1, 0}},
        {nullptr,  {},                          {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_crlf_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"\r\n,",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_COMMA, ",", 1, 0}      },
        {"\r\n:",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_COLON, ":", 1, 0}      },
        {"\r\n[",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_LBRACKET, "[", 1, 0}   },
        {"\r\n]",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_RBRACKET, "]", 1, 0}   },
        {"\r\n+",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_PLUS, "+", 1, 0}       },
        {"\r\n-",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_MINUS, "-", 1, 0}      },
        {"\r\n*",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_ASTERISK, "*", 1, 0}   },
        {"\r\n.",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_DOT, ".", 1, 0}        },
        {"\r\n;",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_COMMENT, ";", 1, 0}    },
        {"\r\n\n",   {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_NEWLINE, "\n", 1, 0}   },
        {"\r\n\r\n", {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_NEWLINE, "\r\n", 1, 0} },
        {"\r\n ",    {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_WHITESPACE, " ", 1, 0} },
        {"\r\n\t",   {TOKEN_NEWLINE, "\r\n", 0, 0}, {TOKEN_WHITESPACE, "\t", 1, 0}},
        {nullptr,    {},                            {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitResult test_lexer_number_boundary(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    boundary_t boundaries[] = {
        {"0x123:8,",     {TOKEN_HEXADECIMAL, "0x123:8", 0, 0},  {TOKEN_COMMA, ",", 0, 7}      },
        {"0x123:16:",    {TOKEN_HEXADECIMAL, "0x123:16", 0, 0}, {TOKEN_COLON, ":", 0, 8}      },
        {"0o777:32[",    {TOKEN_OCTAL, "0o777:32", 0, 0},       {TOKEN_LBRACKET, "[", 0, 8}   },
        {"0b101:64]",    {TOKEN_BINARY, "0b101:64", 0, 0},      {TOKEN_RBRACKET, "]", 0, 8}   },
        {"0x123:8+",     {TOKEN_HEXADECIMAL, "0x123:8", 0, 0},  {TOKEN_PLUS, "+", 0, 7}       },
        {"0x123:16-",    {TOKEN_HEXADECIMAL, "0x123:16", 0, 0}, {TOKEN_MINUS, "-", 0, 8}      },
        {"0o777:32*",    {TOKEN_OCTAL, "0o777:32", 0, 0},       {TOKEN_ASTERISK, "*", 0, 8}   },
        {"0b101:64.",    {TOKEN_BINARY, "0b101:64", 0, 0},      {TOKEN_DOT, ".", 0, 8}        },
        {"0x123:8;",     {TOKEN_HEXADECIMAL, "0x123:8", 0, 0},  {TOKEN_COMMENT, ";", 0, 7}    },
        {"0x123:16\n",   {TOKEN_HEXADECIMAL, "0x123:16", 0, 0}, {TOKEN_NEWLINE, "\n", 0, 8}   },
        {"0o777:32\r\n", {TOKEN_OCTAL, "0o777:32", 0, 0},       {TOKEN_NEWLINE, "\r\n", 0, 8} },
        {"0b101:64 ",    {TOKEN_BINARY, "0b101:64", 0, 0},      {TOKEN_WHITESPACE, " ", 0, 8} },
        {"0x123:8\t",    {TOKEN_HEXADECIMAL, "0x123:8", 0, 0},  {TOKEN_WHITESPACE, "\t", 0, 7}},
        {nullptr,        {},                                    {}                            },
    };

    test_lexer_boundary(boundaries);

    return MUNIT_OK;
}

MunitTest lexer_tests[] = {
    {"/identifier",              test_lexer_identifier,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/identifier_boundary",     test_lexer_identifier_boundary,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/decimal",                 test_lexer_decimal,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/decimal_boundary",        test_lexer_decimal_boundary,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/hexadecimal",             test_lexer_hexadecimal,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/hexadecimal_with_suffix", test_lexer_hexadecimal_with_suffix, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/hexadecimal_boundary",    test_lexer_hexadecimal_boundary,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/octal",                   test_lexer_octal,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/octal_with_suffix",       test_lexer_octal_with_suffix,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/octal_boundary",          test_lexer_octal_boundary,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/binary",                  test_lexer_binary,                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/binary_with_suffix",      test_lexer_binary_with_suffix,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/binary_boundary",         test_lexer_binary_boundary,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/number_boundary",         test_lexer_number_boundary,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/colon",                   test_lexer_colon,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/colon_boundary",          test_lexer_colon_boundary,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/comma",                   test_lexer_comma,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/comma_boundary",          test_lexer_comma_boundary,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/lbracket",                test_lexer_lbracket,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/lbracket_boundary",       test_lexer_lbracket_boundary,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/rbracket",                test_lexer_rbracket,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/rbracket_boundary",       test_lexer_rbracket_boundary,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/plus",                    test_lexer_plus,                    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/plus_boundary",           test_lexer_plus_boundary,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/minus",                   test_lexer_minus,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/minus_boundary",          test_lexer_minus_boundary,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/asterisk",                test_lexer_asterisk,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/asterisk_boundary",       test_lexer_asterisk_boundary,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/dot",                     test_lexer_dot,                     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/dot_boundary",            test_lexer_dot_boundary,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/comment",                 test_lexer_comment,                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/comment_boundary",        test_lexer_comment_boundary,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/whitespace",              test_lexer_whitespace,              NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/whitespace_boundary",     test_lexer_whitespace_boundary,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/newlines",                test_lexer_newlines,                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/newline_boundary",        test_lexer_newline_boundary,        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/crlf_boundary",           test_lexer_crlf_boundary,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/line_numbers",            test_lexer_line_numbers,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL,                       NULL,                               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};
