#ifndef INCLUDE_ENCODER_ENCODER_H_
#define INCLUDE_ENCODER_ENCODER_H_

#include "symbols.h"

typedef struct encoder {
    symbol_table_t *symbols;
} encoder_t;

error_t *encoder_alloc(encoder_t **encoder);
error_t *encoder_encode(encoder_t *encoder, ast_node_t *ast);
void encoder_free(encoder_t *encoder);

extern error_t *const err_encoder_invalid_register;
extern error_t *const err_encoder_number_overflow;
extern error_t *const err_encoder_invalid_number_format;
extern error_t *const err_encoder_invalid_size_suffix;
extern error_t *const err_encoder_unknown_symbol_reference;

#endif // INCLUDE_ENCODER_ENCODER_H_
