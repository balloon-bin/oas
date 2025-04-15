#ifndef INCLUDE_ENCODER_ENCODER_H_
#define INCLUDE_ENCODER_ENCODER_H_

#include "symbols.h"

typedef struct encoder {
    symbol_table_t *symbols;
} encoder_t;

constexpr uint8_t modrm_mod_memory = 0b00'000'000;
constexpr uint8_t modrm_mod_memory_displacement8 = 0b01'000'000;
constexpr uint8_t modrm_mod_memory_displacement32 = 0b10'000'000;
constexpr uint8_t modrm_mod_register = 0b11'000'000;

constexpr uint8_t modrm_reg_mask = 0b00'111'000;
constexpr uint8_t modrm_rm_mask = 0b00'000'111;
constexpr uint8_t modrm_mod_mask = 0b11'000'000;

error_t *encoder_alloc(encoder_t **encoder);
error_t *encoder_encode(encoder_t *encoder, ast_node_t *ast);
void encoder_free(encoder_t *encoder);

extern error_t *const err_encoder_invalid_register;
extern error_t *const err_encoder_number_overflow;
extern error_t *const err_encoder_invalid_number_format;
extern error_t *const err_encoder_invalid_size_suffix;
extern error_t *const err_encoder_unknown_symbol_reference;
extern error_t *const err_encoder_no_encoding_found;
extern error_t *const err_encoder_not_implemented;
extern error_t *const err_encoder_unexpected_length;

#endif // INCLUDE_ENCODER_ENCODER_H_
