#include "../src/encoder/symbols.h"
#include "../src/ast.h"
#include "../src/error.h"
#include "../src/lexer.h"
#include "../src/parser/parser.h"
#include "munit.h"
#include <string.h>

void symbols_setup_test(ast_node_t **node, tokenlist_t **list, char *path) {
    lexer_t *lex = &(lexer_t){};
    lexer_open(lex, path);
    tokenlist_alloc(list);
    tokenlist_fill(*list, lex);
    parse_result_t result = parse((*list)->head);
    lexer_close(lex);

    *node = result.node;
}

MunitResult test_symbol_table_alloc(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    symbol_table_t *table = nullptr;
    error_t *err = symbol_table_alloc(&table);

    munit_assert_ptr_not_null(table);
    munit_assert_ptr_null(err);
    munit_assert_size(table->cap, ==, 64); // Default capacity
    munit_assert_size(table->len, ==, 0);
    munit_assert_ptr_not_null(table->symbols);

    symbol_table_free(table);
    return MUNIT_OK;
}

MunitResult test_symbol_table_lookup_empty(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    symbol_table_t *table = nullptr;
    symbol_table_alloc(&table);

    symbol_t *symbol = symbol_table_lookup(table, "nonexistent");
    munit_assert_ptr_null(symbol);

    symbol_table_free(table);
    return MUNIT_OK;
}

MunitResult test_symbol_add_reference(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    ast_node_t *root;
    tokenlist_t *list;
    symbol_table_t *table = nullptr;
    symbols_setup_test(&root, &list, "tests/input/symbols.asm");
    symbol_table_alloc(&table);

    ast_node_t *reference = root->children[3]->children[1]->children[0]->children[0];
    ast_node_t *statement = root->children[3]; // The containing statement
    munit_assert_int(reference->id, ==, NODE_LABEL_REFERENCE);
    munit_assert_size(table->len, ==, 0);

    error_t *err = symbol_table_update(table, reference, statement);
    munit_assert_null(err);
    munit_assert_size(table->len, ==, 1);

    symbol_t *symbol = symbol_table_lookup(table, "test");
    munit_assert_not_null(symbol);
    munit_assert_int(SYMBOL_REFERENCE, ==, symbol->kind);
    // For references, the statement should be nullptr
    munit_assert_ptr_null(symbol->statement);
    munit_assert_string_equal(symbol->name, "test");

    symbol_table_free(table);
    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_symbol_add_label(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    ast_node_t *root;
    tokenlist_t *list;
    symbol_table_t *table = nullptr;
    symbols_setup_test(&root, &list, "tests/input/symbols.asm");
    symbol_table_alloc(&table);

    ast_node_t *label = root->children[2];
    munit_assert_int(label->id, ==, NODE_LABEL);
    munit_assert_size(table->len, ==, 0);

    error_t *err = symbol_table_update(table, label, label);
    munit_assert_null(err);
    munit_assert_size(table->len, ==, 1);

    symbol_t *symbol = symbol_table_lookup(table, "test");
    munit_assert_not_null(symbol);
    munit_assert_int(SYMBOL_LOCAL, ==, symbol->kind);
    munit_assert_ptr_equal(label, symbol->statement);
    munit_assert_string_equal(symbol->name, "test");

    symbol_table_free(table);
    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_symbol_add_import(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    ast_node_t *root;
    tokenlist_t *list;
    symbol_table_t *table = nullptr;
    symbols_setup_test(&root, &list, "tests/input/symbols.asm");
    symbol_table_alloc(&table);

    ast_node_t *import_directive = root->children[0]->children[1];
    ast_node_t *statement = root->children[0]; // The containing statement
    munit_assert_int(import_directive->id, ==, NODE_IMPORT_DIRECTIVE);
    munit_assert_size(table->len, ==, 0);

    error_t *err = symbol_table_update(table, import_directive, statement);
    munit_assert_null(err);
    munit_assert_size(table->len, ==, 1);

    symbol_t *symbol = symbol_table_lookup(table, "test");
    munit_assert_not_null(symbol);
    munit_assert_int(SYMBOL_IMPORT, ==, symbol->kind);
    // For import directives, the statement should be nullptr
    munit_assert_ptr_null(symbol->statement);
    munit_assert_string_equal(symbol->name, "test");

    symbol_table_free(table);
    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

void test_symbol_update(const char *name, ast_node_t *first, symbol_kind_t first_kind, ast_node_t *first_statement,
                        ast_node_t *second, symbol_kind_t second_kind, ast_node_t *second_statement,
                        bool should_succeed, bool should_update, ast_node_t *expected_statement) {
    symbol_table_t *table = nullptr;
    symbol_table_alloc(&table);

    // Add the first symbol
    error_t *err = symbol_table_update(table, first, first_statement);
    munit_assert_null(err);
    munit_assert_size(table->len, ==, 1);

    // Verify first symbol state
    symbol_t *symbol = symbol_table_lookup(table, name);
    munit_assert_not_null(symbol);
    munit_assert_int(first_kind, ==, symbol->kind);
    munit_assert_string_equal(symbol->name, name);

    // Check statement based on symbol kind
    if (first_kind == SYMBOL_LOCAL) {
        munit_assert_ptr_equal(first_statement, symbol->statement);
    } else {
        munit_assert_ptr_null(symbol->statement);
    }

    // Attempt the second update
    err = symbol_table_update(table, second, second_statement);

    // Check if update succeeded as expected
    if (should_succeed) {
        munit_assert_null(err);
    } else {
        munit_assert_ptr_equal(err, err_symbol_table_incompatible_symbols);
        symbol_table_free(table);
        return;
    }

    // Verify symbol after second update
    symbol = symbol_table_lookup(table, name);
    munit_assert_not_null(symbol);

    // Check if kind updated as expected
    if (should_update) {
        munit_assert_int(second_kind, ==, symbol->kind);
    } else {
        munit_assert_int(first_kind, ==, symbol->kind);
    }

    // Simply check against the expected statement value
    munit_assert_ptr_equal(expected_statement, symbol->statement);

    symbol_table_free(table);
}

MunitResult test_symbol_upgrade_valid(const MunitParameter params[], void *data) {
    ast_node_t *root;
    tokenlist_t *list;

    symbols_setup_test(&root, &list, "tests/input/symbols.asm");

    ast_node_t *reference = root->children[3]->children[1]->children[0]->children[0];
    ast_node_t *reference_statement = root->children[3];
    ast_node_t *label = root->children[2];
    ast_node_t *import_directive = root->children[0]->children[1];
    ast_node_t *import_statement = root->children[0];
    ast_node_t *export_directive = root->children[1]->children[1];
    ast_node_t *export_statement = root->children[1];

    // real upgrades
    test_symbol_update("test", reference, SYMBOL_REFERENCE, reference_statement, label, SYMBOL_LOCAL, label, true, true,
                       label);
    test_symbol_update("test", reference, SYMBOL_REFERENCE, reference_statement, import_directive, SYMBOL_IMPORT,
                       import_statement, true, true, nullptr);
    test_symbol_update("test", reference, SYMBOL_REFERENCE, reference_statement, export_directive, SYMBOL_EXPORT,
                       export_statement, true, true, nullptr);
    test_symbol_update("test", label, SYMBOL_LOCAL, label, export_directive, SYMBOL_EXPORT, export_statement, true,
                       true, label);

    // identity upgrades
    test_symbol_update("test", reference, SYMBOL_REFERENCE, reference_statement, reference, SYMBOL_REFERENCE,
                       reference_statement, true, false, nullptr);
    test_symbol_update("test", label, SYMBOL_LOCAL, label, label, SYMBOL_LOCAL, label, true, false, label);
    test_symbol_update("test", import_directive, SYMBOL_IMPORT, import_statement, import_directive, SYMBOL_IMPORT,
                       import_statement, true, false, nullptr);
    test_symbol_update("test", export_directive, SYMBOL_EXPORT, export_statement, export_directive, SYMBOL_EXPORT,
                       export_statement, true, false, nullptr);

    // downgrades that are allowed and ignored
    test_symbol_update("test", label, SYMBOL_LOCAL, label, reference, SYMBOL_REFERENCE, reference_statement, true,
                       false, label);
    test_symbol_update("test", import_directive, SYMBOL_IMPORT, import_statement, reference, SYMBOL_REFERENCE,
                       reference_statement, true, false, nullptr);
    test_symbol_update("test", export_directive, SYMBOL_EXPORT, export_statement, reference, SYMBOL_REFERENCE,
                       reference_statement, true, false, nullptr);
    test_symbol_update("test", export_directive, SYMBOL_EXPORT, export_statement, label, SYMBOL_LOCAL, label, true,
                       false, label);
    test_symbol_update("test", import_directive, SYMBOL_IMPORT, import_statement, label, SYMBOL_LOCAL, label, true,
                       false, label);

    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_symbol_upgrade_invalid(const MunitParameter params[], void *data) {
    ast_node_t *root;
    tokenlist_t *list;

    symbols_setup_test(&root, &list, "tests/input/symbols.asm");

    ast_node_t *reference = root->children[3]->children[1]->children[0]->children[0];
    ast_node_t *reference_statement = root->children[3];
    ast_node_t *label = root->children[2];
    ast_node_t *import_directive = root->children[0]->children[1];
    ast_node_t *import_statement = root->children[0];
    ast_node_t *export_directive = root->children[1]->children[1];
    ast_node_t *export_statement = root->children[1];

    // invalid upgrades
    test_symbol_update("test", label, SYMBOL_LOCAL, label, import_directive, SYMBOL_IMPORT, import_statement, false,
                       false, nullptr);
    test_symbol_update("test", export_directive, SYMBOL_EXPORT, export_statement, import_directive, SYMBOL_IMPORT,
                       import_statement, false, false, nullptr);
    test_symbol_update("test", import_directive, SYMBOL_IMPORT, import_statement, export_directive, SYMBOL_EXPORT,
                       export_statement, false, false, nullptr);

    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_symbol_add_export(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    ast_node_t *root;
    tokenlist_t *list;
    symbol_table_t *table = nullptr;
    symbols_setup_test(&root, &list, "tests/input/symbols.asm");
    symbol_table_alloc(&table);

    ast_node_t *export_directive = root->children[1]->children[1];
    ast_node_t *statement = root->children[1]; // The containing statement
    munit_assert_int(export_directive->id, ==, NODE_EXPORT_DIRECTIVE);
    munit_assert_size(table->len, ==, 0);

    error_t *err = symbol_table_update(table, export_directive, statement);
    munit_assert_null(err);
    munit_assert_size(table->len, ==, 1);

    symbol_t *symbol = symbol_table_lookup(table, "test");
    munit_assert_not_null(symbol);
    munit_assert_int(SYMBOL_EXPORT, ==, symbol->kind);
    // For export directives, the statement should be nullptr
    munit_assert_ptr_null(symbol->statement);
    munit_assert_string_equal(symbol->name, "test");

    symbol_table_free(table);
    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_symbol_table_growth(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    ast_node_t *root;
    tokenlist_t *list;
    symbol_table_t *table = nullptr;

    // Set up with our manysymbols.asm file
    symbols_setup_test(&root, &list, "tests/input/manysymbols.asm");
    symbol_table_alloc(&table);

    // Initial capacity should be the default (64)
    munit_assert_size(table->cap, ==, 64);
    munit_assert_size(table->len, ==, 0);

    // Add the first 64 labels (indices 0-63)
    size_t initial_cap = table->cap;
    for (size_t i = 0; i < 64; i++) {
        ast_node_t *label = root->children[i];
        munit_assert_int(label->id, ==, NODE_LABEL);

        error_t *err = symbol_table_update(table, label, label);
        munit_assert_null(err);
        munit_assert_size(table->len, ==, i + 1);

        // Capacity should remain the same for the first 64 labels
        munit_assert_size(table->cap, ==, initial_cap);
    }

    // Now add the 65th label (index 64), which should trigger growth
    ast_node_t *final_label = root->children[64];
    munit_assert_int(final_label->id, ==, NODE_LABEL);

    error_t *err = symbol_table_update(table, final_label, final_label);
    munit_assert_null(err);
    munit_assert_size(table->len, ==, 65);

    // Capacity should have doubled
    munit_assert_size(table->cap, ==, initial_cap * 2);

    // Validate we can look up all the symbols
    for (size_t i = 0; i <= 64; i++) {
        char name[10];
        sprintf(name, "lbl_%zu", i);

        symbol_t *symbol = symbol_table_lookup(table, name);
        munit_assert_not_null(symbol);
        munit_assert_int(SYMBOL_LOCAL, ==, symbol->kind);
        munit_assert_string_equal(symbol->name, name);
        munit_assert_ptr_equal(symbol->statement, root->children[i]);
    }

    symbol_table_free(table);
    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitResult test_symbol_invalid_node(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    ast_node_t *root;
    tokenlist_t *list;
    symbol_table_t *table = nullptr;
    symbols_setup_test(&root, &list, "tests/input/symbols.asm");
    symbol_table_alloc(&table);

    munit_assert_size(table->len, ==, 0);
    error_t *err = symbol_table_update(table, root, root);
    munit_assert_ptr_equal(err, err_symbol_table_invalid_node);
    munit_assert_size(table->len, ==, 0);

    symbol_table_free(table);
    ast_node_free(root);
    tokenlist_free(list);
    return MUNIT_OK;
}

MunitTest symbols_tests[] = {
    {"/table_alloc",        test_symbol_table_alloc,        nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/table_lookup_empty", test_symbol_table_lookup_empty, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/add_reference",      test_symbol_add_reference,      nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/add_label",          test_symbol_add_label,          nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/add_import",         test_symbol_add_import,         nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/add_export",         test_symbol_add_export,         nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/upgrade_valid",      test_symbol_upgrade_valid,      nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/upgrade_invalid",    test_symbol_upgrade_invalid,    nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/table_growth",       test_symbol_table_growth,       nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/invalid_node",       test_symbol_invalid_node,       nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {nullptr,               nullptr,                        nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr}
};
