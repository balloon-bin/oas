#ifndef INCLUDE_SRC_BYTES_H_
#define INCLUDE_SRC_BYTES_H_

#include "error.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern error_t *const err_bytes_no_capacity;

typedef struct bytes {
    size_t len;
    size_t cap;
    uint8_t buffer[];
} bytes_t;

#define LOCAL_BYTES_ANONYMOUS(N)                                               \
    &(struct {                                                                 \
        size_t len;                                                            \
        size_t cap;                                                            \
        uint8_t buffer[(N)];                                                   \
    }) {                                                                       \
        0, (N), {}                                                             \
    }

#define LOCAL_BYTES(N) (bytes_t *)LOCAL_BYTES_ANONYMOUS(N);

static inline error_t *bytes_append_uint8(bytes_t *bytes, uint8_t value) {
    if (bytes->len >= bytes->cap)
        return err_bytes_no_capacity;
    bytes->buffer[bytes->len++] = value;
    return nullptr;
}

static inline error_t *bytes_append_array(bytes_t *dst, size_t n,
                                          uint8_t buffer[static n]) {
    if (dst->len + n >= dst->cap)
        return err_bytes_no_capacity;
    memcpy(dst->buffer + dst->len, buffer, n);
    dst->len += n;
    return nullptr;
}

static inline error_t *bytes_append_bytes(bytes_t *dst, bytes_t *src) {
    return bytes_append_array(dst, src->len, src->buffer);
}

static inline error_t *bytes_append_uint16(bytes_t *dst, uint16_t value) {
    return bytes_append_array(dst, sizeof(value), (uint8_t *)&value);
}

static inline error_t *bytes_append_uint32(bytes_t *dst, uint32_t value) {
    return bytes_append_array(dst, sizeof(value), (uint8_t *)&value);
}

static inline error_t *bytes_append_uint64(bytes_t *dst, uint64_t value) {
    return bytes_append_array(dst, sizeof(value), (uint8_t *)&value);
}

#endif // INCLUDE_SRC_BYTES_H_
