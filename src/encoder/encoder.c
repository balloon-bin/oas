#include "encoder.h"
#include "../bytes.h"
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
error_t *const err_encoder_no_encoding_found =
    &(error_t){.message = "No encoding found for instruction"};
error_t *const err_encoder_not_implemented =
    &(error_t){.message = "Implementation for this opcode is missing"};
error_t *const err_encoder_unexpected_length =
    &(error_t){.message = "Unexpectedly long encoding"};

error_t *encoder_alloc(encoder_t **output, ast_node_t *ast) {
    *output = nullptr;
    encoder_t *encoder = calloc(1, sizeof(encoder_t));

    if (encoder == nullptr)
        return err_allocation_failed;

    encoder->ast = ast;

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
 * Set the opcode extension in the modrm field
 */
static inline uint8_t modrm_extension(uint8_t modrm, uint8_t extension) {
    assert(extension != opcode_extension_none);
    assert((extension & 0b111) == extension);
    return (modrm & ~modrm_reg_mask) | extension << 3;
}

/**
 * Return the rex bit for reg field in modrm
 */
static inline uint8_t modrm_reg_rex(uint8_t rex, register_id_t id) {
    if (id & 0b1000)
        rex |= rex_prefix_r;
    return rex;
}

/**
 * update modrm reg field with the given register, must be used alongside
 * modrm_reg_rex
 */
static inline uint8_t modrm_reg(uint8_t modrm, register_id_t id) {
    return (modrm & ~modrm_reg_mask) | (id & 0b111) << 3;
}

/**
 * Return the rex bit for rm field in modrm
 */
static inline uint8_t modrm_rm_rex(uint8_t rex, register_id_t id) {
    if (id & 0b1000)
        rex |= rex_prefix_b;
    return rex;
}

/**
 * update modrm rm field with the given register, must be used alongside
 * modrm_rm_rex
 */
static inline uint8_t modrm_rm(uint8_t modrm, register_id_t id) {
    assert((modrm & modrm_mod_mask) == modrm_mod_register);
    return (modrm & ~modrm_rm_mask) | (id & 0b111);
}

error_t *encoder_collect_info(encoder_t *encoder, ast_node_t *node,
                              ast_node_t *statement) {
    error_t *err = nullptr;

    if (encoder_is_symbols_node(node))
        err = symbol_table_update(encoder->symbols, node, statement);
    else if (node->id == NODE_NUMBER)
        err = encoder_set_number_value(node);
    else if (node->id == NODE_REGISTER)
        err = encoder_set_register_value(node);
    if (err)
        return err;

    for (size_t i = 0; i < node->len; ++i) {
        error_t *err =
            encoder_collect_info(encoder, node->children[i], statement);
        if (err)
            return err;
    }

    return nullptr;
}

/**
 * Perform the initial pass over the AST.
 *
 * - Collect information about the operands
 *   - parse and set number values
 *   - set the register values
 *   - determine if label references are used by an instruction
 * - encode instructions that don't use label references
 * - determine estimated addresses of each statement
 *
 */
error_t *encoder_first_pass(encoder_t *encoder) {
    ast_node_t *root = encoder->ast;
    assert(root->id == NODE_PROGRAM);

    for (size_t i = 0; i < root->len; ++i) {
        ast_node_t *statement = root->children[i];
        error_t *err = encoder_collect_info(encoder, statement, statement);
        if (err)
            return err;
    }

    return nullptr;
}

bool is_operand_match(operand_info_t *info, ast_node_t *operand) {
    switch (info->kind) {
    case OPERAND_REGISTER:
        return operand->id == NODE_REGISTER &&
               operand->value.reg.size == info->size;
    case OPERAND_MEMORY:
        return operand->id == NODE_MEMORY;
    case OPERAND_IMMEDIATE: {
        if (operand->id != NODE_IMMEDIATE)
            return false;
        ast_node_t *child = operand->children[0];

        if (child->id == NODE_NUMBER)
            return (child->value.number.size & info->size) > 0;
        else if (child->id == NODE_LABEL_REFERENCE)
            return info->size == OPERAND_SIZE_32;
        // FIXME: first pass should give us information about the distance of
        // the label reference so we can pick a size more appropriately instead
        // of just defaulting to 32 bits
        break;
    } // end OPERAND_IMMEDIATE case
    }
    assert(false && "unreachable");
    __builtin_unreachable();
}

bool is_opcode_match(opcode_data_t *opcode, const char *mnemonic,
                     ast_node_t *operands) {
    if (strcmp(opcode->mnemonic, mnemonic) != 0)
        return false;

    if (opcode->operand_count != operands->len)
        return false;

    for (size_t i = 0; i < operands->len; ++i) {
        if (!is_operand_match(&opcode->operands[i], operands->children[i]))
            return false;
    }

    return true;
}

error_t *encoder_get_opcode_data(ast_node_t *instruction, ast_node_t *operands,
                                 opcode_data_t **opcode_out) {
    const char *mnemonic = instruction->children[0]->token_entry->token.value;

    for (size_t i = 0; opcodes[i]; ++i) {
        opcode_data_t *opcode = opcodes[i];
        if (is_opcode_match(opcode, mnemonic, operands)) {
            *opcode_out = opcode;
            return nullptr;
        }
    }
    return err_encoder_no_encoding_found;
}

error_t *encode_two_operand(encoder_t *encoder, opcode_data_t *opcode,
                            ast_node_t *operands, bytes_t *encoding,
                            uint8_t *rex) {
    (void)encoder;
    (void)opcode;
    (void)operands;
    (void)encoding;
    (void)rex;
    assert(encoding->len >= 1 && "must have 1+ opcode byte in buffer already");
    return err_encoder_not_implemented;
}

error_t *encode_one_register_in_opcode(encoder_t *encoder,
                                       opcode_data_t *opcode,
                                       ast_node_t *operands, bytes_t *encoding,
                                       uint8_t *rex) {
    (void)encoder;
    (void)opcode;

    register_id_t id = operands->children[0]->value.reg.id;
    encoding->buffer[encoding->len - 1] |= id & 0b111;
    if ((id & 0b1000) > 0) {
        *rex |= rex_prefix_r;
    }
    return nullptr;
}

error_t *encode_one_register(encoder_t *encoder, opcode_data_t *opcode,
                             ast_node_t *operands, bytes_t *encoding,
                             uint8_t *rex) {
    (void)encoder;
    assert(operands->len == 1);
    assert(operands->children[0]->id == NODE_REGISTER);

    register_id_t id = operands->children[0]->value.reg.id;

    uint8_t modrm = modrm_mod_register;

    if (opcode->opcode_extension != opcode_extension_none) {
        // register goes in rm field, extension goes in mod field
        modrm = modrm_extension(modrm, opcode->opcode_extension);
        modrm = modrm_rm(modrm, id);
        *rex = modrm_rm_rex(*rex, id);
    } else {
        // register goes in reg field
        // NOTE:
        // it's actually likely this case just doesn't exist at all and all
        // opcodes that take one register in modr/m _all_ have extended opcdes
        modrm = modrm_reg(modrm, id);
        *rex = modrm_reg_rex(*rex, id);
    }

    return bytes_append_uint8(encoding, modrm);
}

error_t *encode_one_immediate(encoder_t *encoder, opcode_data_t *opcode,
                              ast_node_t *operands, bytes_t *encoding,
                              uint8_t *rex) {
    (void)encoder;
    (void)opcode;
    (void)rex;
    assert(operands->len == 1);
    assert(operands->children[0]->id == NODE_IMMEDIATE);
    assert(operands->children[0]->len == 1);
    ast_node_t *immediate = operands->children[0]->children[0];
    assert(immediate->id == NODE_NUMBER ||
           immediate->id == NODE_LABEL_REFERENCE);

    if (immediate->id == NODE_NUMBER) {
        uint64_t value = immediate->value.number.value;
        operand_size_t size = opcode->operands[0].size;
        error_t *err = nullptr;
        switch (size) {
        case OPERAND_SIZE_8:
            err = bytes_append_uint8(encoding, value);
            break;
        case OPERAND_SIZE_16:
            err = bytes_append_uint16(encoding, value);
            break;
        case OPERAND_SIZE_32:
            err = bytes_append_uint32(encoding, value);
            break;
        case OPERAND_SIZE_64:
            err = bytes_append_uint64(encoding, value);
            break;
        default:
            assert(false && "intentionally unhandled");
        }
        return err;
    } else {
        // FIXME: this still assumes references are always 32 bit
        uint32_t value = 0xDEADBEEF;
        return bytes_append_uint32(encoding, value);
    }
}

error_t *encode_one_memory(encoder_t *encoder, opcode_data_t *opcode,
                           ast_node_t *operands, bytes_t *encoding,
                           uint8_t *rex) {
    (void)encoder;
    (void)opcode;
    (void)operands;
    (void)encoding;
    (void)rex;
    return err_encoder_not_implemented;
}

error_t *encode_one_operand(encoder_t *encoder, opcode_data_t *opcode,
                            ast_node_t *operands, bytes_t *encoding,
                            uint8_t *rex) {
    switch (opcode->operands[0].kind) {
    case OPERAND_REGISTER:
        if (opcode->encoding_class == ENCODING_OPCODE_REGISTER)
            return encode_one_register_in_opcode(encoder, opcode, operands,
                                                 encoding, rex);
        else
            return encode_one_register(encoder, opcode, operands, encoding,
                                       rex);
    case OPERAND_MEMORY:
        return encode_one_memory(encoder, opcode, operands, encoding, rex);
    case OPERAND_IMMEDIATE:
        return encode_one_immediate(encoder, opcode, operands, encoding, rex);
    }
}

error_t *encoder_encode_instruction(encoder_t *encoder,
                                    ast_node_t *instruction) {
    ast_node_t *operands = instruction->children[1];

    opcode_data_t *opcode = nullptr;
    error_t *err = encoder_get_opcode_data(instruction, operands, &opcode);
    if (err)
        return err;

    uint8_t rex = 0;
    bytes_t *encoding = LOCAL_BYTES(32);

    if (opcode->opcode > 0xFF &&
        (err = bytes_append_uint8(encoding, opcode->opcode >> 8)))
        return err;
    if ((err = bytes_append_uint8(encoding, opcode->opcode & 0xFF)))
        return err;

    // NOTE:operand encoders all expect the opcode to be in the buffer already.
    // Some of them rely on this to encode the register value in the opcode
    // byte.
    switch (opcode->operand_count) {
    case 0:
        break;
    case 1:
        err = encode_one_operand(encoder, opcode, operands, encoding, &rex);
        break;
    case 2:
        err = encode_two_operand(encoder, opcode, operands, encoding, &rex);
        break;
    default:
        err = err_encoder_not_implemented;
    }
    if (err)
        return err;

    // produce the actual encoding output in the NODE_INSTRUCTION value
    uint8_t *output = instruction->value.instruction.encoding.buffer;
    size_t output_len = 0;

    // Handle prefixes
    if (opcode->rex_w_prefix)
        rex = rex_prefix_w;
    if (opcode->address_size_prefix)
        output[output_len++] = memory_size_prefix;
    if (opcode->operand_size_prefix)
        output[output_len++] = operand_size_prefix;
    if (rex > 0)
        output[output_len++] = rex;

    // copy the encoded opcode and operands
    if (encoding->len > 20)
        return err_encoder_unexpected_length;
    memcpy(output + output_len, encoding->buffer, encoding->len);
    output_len += encoding->len;

    instruction->value.instruction.encoding.len = output_len;

    return nullptr;
}

/**
 * Perform the second pass that performs actual encoding. Will use
 * placeholder values for label references because instruction size has not
 * yet been determined.
 */
error_t *encoder_second_pass(encoder_t *encoder) {
    ast_node_t *root = encoder->ast;

    for (size_t i = 0; i < root->len; ++i) {
        if (root->children[i]->id != NODE_INSTRUCTION)
            continue;
        ast_node_t *instruction = root->children[i];
        error_t *err = encoder_encode_instruction(encoder, instruction);
        if (err)
            return err;
    }
    return nullptr;
}

opcode_data_t *encoder_find_opcode(ast_node_t *instruction) {
    for (size_t i = 0; opcodes[i] != nullptr; ++i) {
        const char *mnemonic =
            instruction->children[0]->token_entry->token.value;
        ast_node_t *operands = instruction->children[1];
        if (is_opcode_match(opcodes[i], mnemonic, operands))
            return opcodes[i];
    }
    return nullptr;
}

error_t *encoder_check_symbols(encoder_t *encoder) {
    for (size_t i = 0; i < encoder->symbols->len; ++i)
        if (encoder->symbols->symbols[i].kind == SYMBOL_REFERENCE)
            return err_encoder_unknown_symbol_reference;
    return nullptr;
}

error_t *encoder_encode(encoder_t *encoder) {
    error_t *err = encoder_first_pass(encoder);
    if (err)
        return err;
    err = encoder_check_symbols(encoder);
    if (err)
        return err;
    return encoder_second_pass(encoder);
}
