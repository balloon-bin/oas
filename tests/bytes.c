#include "../src/bytes.h"
#include "munit.h"

MunitResult test_bytes_initializer(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    bytes_t *bytes = LOCAL_BYTES(16);
    munit_assert_size(bytes->len, ==, 0);
    munit_assert_size(bytes->cap, ==, 16);
    for (size_t i = 0; i < 16; ++i)
        munit_assert_uint8(bytes->buffer[i], ==, 0);
    return MUNIT_OK;
}

MunitResult test_bytes_append_uint8(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;
    bytes_t *bytes = LOCAL_BYTES(16);
    munit_assert_size(bytes->len, ==, 0);
    munit_assert_size(bytes->cap, ==, 16);
    for (size_t i = 0; i < 16; ++i) {
        error_t *err = bytes_append_uint8(bytes, (uint8_t)i);
        munit_assert_null(err);
        munit_assert_uint8(bytes->buffer[i], ==, (uint8_t)i);
    }

    error_t *err = bytes_append_uint8(bytes, 0xFF);
    munit_assert_ptr(err, ==, err_bytes_no_capacity);

    return MUNIT_OK;
}

MunitResult test_bytes_append_array(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    bytes_t *bytes = LOCAL_BYTES(16);
    munit_assert_size(bytes->len, ==, 0);
    munit_assert_size(bytes->cap, ==, 16);

    uint8_t test_array[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    size_t array_len = sizeof(test_array) / sizeof(test_array[0]);
    error_t *err = bytes_append_array(bytes, array_len, test_array);
    munit_assert_null(err);
    munit_assert_size(bytes->len, ==, array_len);

    for (size_t i = 0; i < array_len; ++i) {
        munit_assert_uint8(bytes->buffer[i], ==, test_array[i]);
    }

    uint8_t second_array[] = {0x06, 0x07, 0x08};
    size_t second_len = sizeof(second_array) / sizeof(second_array[0]);
    err = bytes_append_array(bytes, second_len, second_array);
    munit_assert_null(err);
    munit_assert_size(bytes->len, ==, array_len + second_len);
    for (size_t i = 0; i < second_len; ++i) {
        munit_assert_uint8(bytes->buffer[array_len + i], ==, second_array[i]);
    }

    uint8_t overflow_array[10] = {0}; // Array that would exceed capacity
    err = bytes_append_array(bytes, sizeof(overflow_array), overflow_array);
    munit_assert_ptr(err, ==, err_bytes_no_capacity);
    munit_assert_size(bytes->len, ==, array_len + second_len);

    return MUNIT_OK;
}

MunitResult test_bytes_append_bytes(const MunitParameter params[], void *data) {
    (void)params;
    (void)data;

    bytes_t *src = LOCAL_BYTES(8);
    bytes_t *dst = LOCAL_BYTES(16);

    // Fill source bytes with test data
    for (uint8_t i = 0; i < 5; ++i) {
        error_t *err = bytes_append_uint8(src, i + 1);
        munit_assert_null(err);
    }
    munit_assert_size(src->len, ==, 5);

    // Append source to destination
    error_t *err = bytes_append_bytes(dst, src);
    munit_assert_null(err);
    munit_assert_size(dst->len, ==, src->len);

    // Verify destination contents match source
    for (size_t i = 0; i < src->len; ++i) {
        munit_assert_uint8(dst->buffer[i], ==, src->buffer[i]);
    }

    // Fill source with more data and append again
    for (uint8_t i = 0; i < 3; ++i) {
        err = bytes_append_uint8(src, i + 6);
        munit_assert_null(err);
    }
    munit_assert_size(src->len, ==, 8);

    // Append updated source
    err = bytes_append_bytes(dst, src);
    munit_assert_null(err);
    munit_assert_size(dst->len, ==, 13); // 5 + 8

    // Test capacity boundary
    src->len = 4; // manually set length to barely not fit
    err = bytes_append_bytes(dst, src);
    munit_assert_ptr(err, ==, err_bytes_no_capacity);
    munit_assert_size(dst->len, ==, 13); // Length unchanged after error

    return MUNIT_OK;
}
MunitResult test_bytes_append_uint16(const MunitParameter params[], void *data) {
    bytes_t *bytes = LOCAL_BYTES(16);
    munit_assert_size(bytes->len, ==, 0);
    munit_assert_size(bytes->cap, ==, 16);

    bytes_append_uint16(bytes, 0xFFAA);
    munit_assert_size(bytes->len, ==, 2);
    munit_assert_uint8(bytes->buffer[0], ==, 0xAA);
    munit_assert_uint8(bytes->buffer[1], ==, 0xFF);

    return MUNIT_OK;
}
MunitResult test_bytes_append_uint32(const MunitParameter params[], void *data) {
    bytes_t *bytes = LOCAL_BYTES(16);
    munit_assert_size(bytes->len, ==, 0);
    munit_assert_size(bytes->cap, ==, 16);

    bytes_append_uint32(bytes, 0xAABBCCDD);
    munit_assert_size(bytes->len, ==, 4);
    munit_assert_uint8(bytes->buffer[0], ==, 0xDD);
    munit_assert_uint8(bytes->buffer[1], ==, 0xCC);
    munit_assert_uint8(bytes->buffer[2], ==, 0xBB);
    munit_assert_uint8(bytes->buffer[3], ==, 0xAA);
    return MUNIT_OK;
}
MunitResult test_bytes_append_uint64(const MunitParameter params[], void *data) {
    bytes_t *bytes = LOCAL_BYTES(16);
    munit_assert_size(bytes->len, ==, 0);
    munit_assert_size(bytes->cap, ==, 16);

    bytes_append_uint64(bytes, 0xAABBCCDDEEFF9988);
    munit_assert_size(bytes->len, ==, 8);
    munit_assert_uint8(bytes->buffer[0], ==, 0x88);
    munit_assert_uint8(bytes->buffer[1], ==, 0x99);
    munit_assert_uint8(bytes->buffer[2], ==, 0xFF);
    munit_assert_uint8(bytes->buffer[3], ==, 0xEE);
    munit_assert_uint8(bytes->buffer[4], ==, 0xDD);
    munit_assert_uint8(bytes->buffer[5], ==, 0xCC);
    munit_assert_uint8(bytes->buffer[6], ==, 0xBB);
    munit_assert_uint8(bytes->buffer[7], ==, 0xAA);
    return MUNIT_OK;
}

MunitTest bytes_tests[] = {
    {"/initializer",   test_bytes_initializer,   nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/append_uint8",  test_bytes_append_uint8,  nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/append_array",  test_bytes_append_array,  nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/append_bytes",  test_bytes_append_bytes,  nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/append_uint16", test_bytes_append_uint16, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/append_uint32", test_bytes_append_uint32, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {"/append_uint64", test_bytes_append_uint64, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr},
    {nullptr,          nullptr,                  nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr}
};
