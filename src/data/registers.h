#ifndef INCLUDE_DATA_REGISTERS_H_
#define INCLUDE_DATA_REGISTERS_H_

typedef enum operand_size {
    OPERAND_SIZE_INVALID = 0,

    OPERAND_SIZE_8 = 1 << 0,
    OPERAND_SIZE_16 = 1 << 1,
    OPERAND_SIZE_32 = 1 << 2,
    OPERAND_SIZE_64 = 1 << 3,

    OPERAND_SIZE_80 = 1 << 4,
    OPERAND_SIZE_128 = 1 << 5,
    OPERAND_SIZE_256 = 1 << 6,
    OPERAND_SIZE_512 = 1 << 7,
} operand_size_t;

static inline operand_size_t bits_to_operand_size(int bits) {
    switch (bits) {
    case 8:
        return OPERAND_SIZE_8;
    case 16:
        return OPERAND_SIZE_16;
    case 32:
        return OPERAND_SIZE_32;
    case 64:
        return OPERAND_SIZE_64;
    case 80:
        return OPERAND_SIZE_80;
    case 128:
        return OPERAND_SIZE_128;
    case 256:
        return OPERAND_SIZE_256;
    case 512:
        return OPERAND_SIZE_512;
    default:
        return OPERAND_SIZE_INVALID;
    }
}

typedef enum register_id {
    // Special registers
    REG_RIP = -1,

    // General purpose registers
    REG_A = 0x0000,
    REG_C,
    REG_D,
    REG_B,
    REG_SP,
    REG_BP,
    REG_SI,
    REG_DI,

    REG_8,
    REG_9,
    REG_10,
    REG_11,
    REG_12,
    REG_13,
    REG_14,
    REG_15,

    REG_ST0 = 0x1000,
    REG_ST1,
    REG_ST2,
    REG_ST3,
    REG_ST4,
    REG_ST5,
    REG_ST6,
    REG_ST7,
} register_id_t;

typedef struct register_data {
    const char *name;
    register_id_t id;
    operand_size_t size;
} register_data_t;

extern register_data_t *const registers[];

#endif // INCLUDE_DATA_REGISTERS_H_
