#include "opcodes.h"

// clang-format off
opcode_data_t *const opcodes[] = {
    // RET
    &(opcode_data_t) {
        .mnemonic = "ret",
        .opcode = 0xC3,
        .opcode_extension = opcode_extension_none,
        .operand_count = 0,
    },
    // RET imm16
    &(opcode_data_t) {
        .mnemonic = "ret",
        .opcode = 0xC2,
        .opcode_extension = opcode_extension_none,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_16 },
        },
    },
    // PUSH imm8
    &(opcode_data_t) {
        .mnemonic = "push",
        .opcode = 0x6A,
        .opcode_extension = opcode_extension_none,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_8},
        },
    },
    // PUSH imm16
    &(opcode_data_t) {
        .mnemonic = "push",
        .opcode = 0x68,
        .opcode_extension = opcode_extension_none,
        .operand_size_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_16},
        },
    },
    // PUSH imm32
    &(opcode_data_t) {
        .mnemonic = "push",
        .opcode = 0x68,
        .opcode_extension = opcode_extension_none,
        .operand_size_prefix = false,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_32},
        },
    },
    // PUSH reg16, 
    &(opcode_data_t) {
        .mnemonic = "push",
        .opcode = 0x50,
        .opcode_extension = opcode_extension_none,
        .encoding_class = ENCODING_OPCODE_REGISTER,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_16 },
        },
    },
    // PUSH reg64
    &(opcode_data_t) {
        .mnemonic = "push",
        .opcode = 0x50,
        .opcode_extension = opcode_extension_none,
        .encoding_class = ENCODING_OPCODE_REGISTER,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_64 },
        },
    },
    // NOT reg16
    &(opcode_data_t) {
        .mnemonic = "not",
        .opcode = 0xF7,
        .opcode_extension = 2,
        .operand_size_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_16 },
        },
    },
    // NOT reg32
    &(opcode_data_t) {
        .mnemonic = "not",
        .opcode = 0xF7,
        .opcode_extension = 2,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_32 },
        },
    },
    // NOT reg64
    &(opcode_data_t) {
        .mnemonic = "not",
        .opcode = 0xF7,
        .opcode_extension = 2,
        .rex_w_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_64 },
        },
    },

    // NEG reg16
    &(opcode_data_t) {
        .mnemonic = "neg",
        .opcode = 0xF7,
        .opcode_extension = 3,
        .operand_size_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_16 },
        },
    },
    // NEG reg32
    &(opcode_data_t) {
        .mnemonic = "neg",
        .opcode = 0xF7,
        .opcode_extension = 3,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_32 },
        },
    },
    // NEG reg64
    &(opcode_data_t) {
        .mnemonic = "neg",
        .opcode = 0xF7,
        .opcode_extension = 3,
        .rex_w_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_64 },
        },
    },
    // CALL rel32
    &(opcode_data_t) {
        .mnemonic = "call",
        .opcode = 0xE8,
        .opcode_extension = opcode_extension_none,
        .encoding_class = ENCODING_DEFAULT,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_32 },
        },
    },
    // CALL reg64
    &(opcode_data_t) {
        .mnemonic = "call",
        .opcode = 0xFF,
        .opcode_extension = 2,
        .encoding_class = ENCODING_DEFAULT,
        .rex_w_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_64 },
        },
    },
    // CALL mem64
    &(opcode_data_t) {
        .mnemonic = "call",
        .opcode = 0xFF,
        .opcode_extension = 2,
        .encoding_class = ENCODING_DEFAULT,
        .rex_w_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_MEMORY, .size = OPERAND_SIZE_64 },
        },
    },
    // JMP rel8 (short jump)
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xEB,
        .opcode_extension = opcode_extension_none,
        .encoding_class = ENCODING_DEFAULT,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_8 },
        },
    },

    // JMP rel16
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xE9,
        .opcode_extension = opcode_extension_none,
        .encoding_class = ENCODING_DEFAULT,
        .operand_size_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_16 },
        },
    },

    // JMP reg16
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xFF,
        .opcode_extension = 4,
        .encoding_class = ENCODING_DEFAULT,
        .operand_size_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_16 },
        },
    },

    // JMP rel32 (near jump)
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xE9,
        .opcode_extension = opcode_extension_none,
        .encoding_class = ENCODING_DEFAULT,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_IMMEDIATE, .size = OPERAND_SIZE_32 },
        },
    },

    // JMP reg32
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xFF,
        .opcode_extension = 4,
        .encoding_class = ENCODING_DEFAULT,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_32 },
        },
    },

    // JMP reg64
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xFF,
        .opcode_extension = 4,
        .encoding_class = ENCODING_DEFAULT,
        .rex_w_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_REGISTER, .size = OPERAND_SIZE_64 },
        },
    },

    // JMP mem64
    &(opcode_data_t) {
        .mnemonic = "jmp",
        .opcode = 0xFF,
        .opcode_extension = 4,
        .encoding_class = ENCODING_DEFAULT,
        .rex_w_prefix = true,
        .operand_count = 1,
        .operands = {
            { .kind = OPERAND_MEMORY, .size = OPERAND_SIZE_64 },
        },
    },
    nullptr,
};

