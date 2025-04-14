#include "encoder.h"
#include "../data/opcodes.h"
#include "symbols.h"
#include <assert.h>
#include <errno.h>
#include <string.h>

error_t *const err_encoder_invalid_register =
    &(error_t){.message = "Invalid register"};
error_t *const err_encoder_number_overflow =
    &(error_t){.message = "Number overflows the storage"};
error_t *const err_encoder_invalid_number_format =
    &(error_t){.message = "Invalid number format"};
error_t *const err_encoder_invalid_size_suffix =
    &(error_t){.message = "Invalid number size suffix"};
error_t *const err_encoder_unknown_symbol_reference =
    &(error_t){.message = "Referenced an unknown symbol"};

error_t *encoder_alloc(encoder_t **output) {
    *output = nullptr;
    encoder_t *encoder = calloc(1, sizeof(encoder_t));

    if (encoder == nullptr)
        return err_allocation_failed;

    error_t *err = symbol_table_alloc(&encoder->symbols);
    if (err) {
        free(encoder);
        return err;
    }

    *output = encoder;
    return nullptr;
}

void encoder_free(encoder_t *encoder) {
    if (encoder == nullptr)
        return;
    symbol_table_free(encoder->symbols);
    free(encoder);
}

bool encoder_is_symbols_node(ast_node_t *node) {
    switch (node->id) {
    case NODE_LABEL:
    case NODE_LABEL_REFERENCE:
    case NODE_EXPORT_DIRECTIVE:
    case NODE_IMPORT_DIRECTIVE:
        return true;
    default:
        return false;
    }
}

int encoder_get_number_base(ast_node_t *number) {
    switch (number->children[0]->id) {
    case NODE_BINARY:
        return 2;
    case NODE_OCTAL:
        return 8;
    case NODE_DECIMAL:
        return 10;
    case NODE_HEXADECIMAL:
        return 16;
    default:
        assert(false);
    }
    __builtin_unreachable();
}

bool is_valid_size_suffix(int bits) {
    switch (bits) {
    case 0:
    case 8:
    case 16:
    case 32:
    case 64:
        return true;
    default:
        return false;
    }
}

bool is_overflow(uint64_t value, int bits) {
    if (bits == 0 || bits >= 64)
        return false;

    uint64_t max_value = (1ULL << bits) - 1;
    return value > max_value;
}

operand_size_t encoder_get_size_mask(uint64_t value, int bits) {
    if (bits != 0)
        return bits_to_operand_size(bits);

    operand_size_t mask = OPERAND_SIZE_64;
    if (value < (1ULL << 8))
        mask |= OPERAND_SIZE_8;
    if (value < (1ULL << 16))
        mask |= OPERAND_SIZE_16;
    if (value < (1ULL << 32))
        mask |= OPERAND_SIZE_32;
    return mask;
}

error_t *encoder_set_number_value(ast_node_t *node) {
    assert(node->id == NODE_NUMBER);
    assert(node->children[0]);
    const char *number = node->children[0]->token_entry->token.value;
    int base = encoder_get_number_base(node);

    if (base != 10)
        number += 2; // all except base 10 use a 0x, 0o or 0b prefix

    char *endptr;
    errno = 0;
    uint64_t value = strtoull(number, &endptr, base);

    if (errno == ERANGE)
        return err_encoder_number_overflow;

    if (endptr == number)
        return err_encoder_invalid_number_format;

    int bits = 0;
    if (*endptr == ':') {
        const char *suffix = endptr + 1;

        bits = strtol(suffix, &endptr, 10);

        if (endptr == suffix)
            return err_encoder_invalid_number_format;
    }

    if (*endptr != '\0')
        return err_encoder_invalid_number_format;

    if (!is_valid_size_suffix(bits))
        return err_encoder_invalid_size_suffix;

    if (is_overflow(value, bits))
        return err_encoder_number_overflow;

    node->value.number.value = value;
    node->value.number.size = encoder_get_size_mask(value, bits);

    return nullptr;
}

error_t *encoder_set_register_value(ast_node_t *node) {
    assert(node->id == NODE_REGISTER);

    const char *value = node->token_entry->token.value;

    for (size_t i = 0; registers[i] != nullptr; ++i) {
        if (strcmp(value, registers[i]->name) == 0) {
            node->value.reg.id = registers[i]->id;
            node->value.reg.size = registers[i]->size;
            return nullptr;
        }
    }
    return err_encoder_invalid_register;
}

/**
 * Perform the initial pass over the AST. Records all symbols and sets the
 * values of registers and numbers.
 */
error_t *encoder_first_pass(encoder_t *encoder, ast_node_t *node) {
    error_t *err = nullptr;

    if (encoder_is_symbols_node(node))
        err = symbol_table_update(encoder->symbols, node);
    else if (node->id == NODE_NUMBER)
        err = encoder_set_number_value(node);
    else if (node->id == NODE_REGISTER)
        err = encoder_set_register_value(node);
    if (err)
        return err;

    for (size_t i = 0; i < node->len; ++i) {
        error_t *err = encoder_first_pass(encoder, node->children[i]);
        if (err)
            return err;
    }

    return nullptr;
}

opcode_data_t *encoder_find_opcode(ast_node_t *instruction) {
    for (size_t i = 0; opcodes[i] != nullptr; ++i) {
    }
    return nullptr;
}

error_t *encoder_check_symbols(encoder_t *encoder) {
    for (size_t i = 0; i < encoder->symbols->len; ++i)
        if (encoder->symbols->symbols[i].kind == SYMBOL_REFERENCE)
            return err_encoder_unknown_symbol_reference;
    return nullptr;
}

error_t *encoder_encode(encoder_t *encoder, ast_node_t *ast) {
    error_t *err = encoder_first_pass(encoder, ast);
    if (err)
        return err;
    err = encoder_check_symbols(encoder);
    if (err)
        return err;
    return nullptr;
}
