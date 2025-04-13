#ifndef INCLUDE_DATA_OPCODES_H_
#define INCLUDE_DATA_OPCODES_H_

#include "../data/registers.h"
#include <stddef.h>
#include <stdint.h>

constexpr uint8_t rex_prefix = 0x40;
constexpr uint8_t rex_prefix_w = 0x48;
constexpr uint8_t rex_prefix_r = 0x44;
constexpr uint8_t rex_prefix_x = 0x42;
constexpr uint8_t rex_prefix_b = 0x41;

constexpr uint8_t operand_size_prefix = 0x66;
constexpr uint8_t memory_size_prefix = 0x67;
constexpr uint8_t lock_prefix = 0xF0;
constexpr uint8_t repne_prefix = 0xF2;
constexpr uint8_t rep_prefix = 0xF3;

typedef enum encoding_class {
    ENCODING_DEFAULT,         // use modrm+sib for registers and memory, append
                              // immediates
    ENCODING_OPCODE_REGISTER, // encode the register in the last 3 bits of the
                              // opcode
} encoding_class_t;

typedef enum operand_kind {
    OPERAND_REGISTER,
    OPERAND_MEMORY,
    OPERAND_IMMEDIATE,
} operand_kind_t;

typedef struct operand_info {
    operand_kind_t kind;
    operand_size_t size;
} operand_info_t;

constexpr uint8_t opcode_extension_none = 0xFF;

typedef struct opcode_data {
    const char *mnemonic;

    uint16_t opcode;
    uint8_t opcode_extension; // 3 bits for the opcode extension in the reg
                              // field of a modr/m byte
    encoding_class_t encoding_class;
    bool operand_size_prefix;
    bool address_size_prefix;
    bool rex_w_prefix;
    size_t operand_count;
    operand_info_t operands[3];
} opcode_data_t;

extern opcode_data_t *const opcodes[];

#endif // INCLUDE_DATA_OPCODES_H_
