#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

error_t *const err_errorf_alloc = &(error_t){
    .message = "Allocation failed during formatting of another error"};
error_t *const err_errorf_length = &(error_t){
    .message =
        "Formatting of another error failed to determine the error length"};
error_t *const err_eof =
    &(error_t){.message = "Read failed because EOF is reached"};

error_t *const err_unknown_read_failure =
    &(error_t){.message = "Unknown read error"};

error_t *const err_allocation_failed =
    &(error_t){.message = "Memory allocation failed"};

error_t *errorf(const char *fmt, ...) {
    error_t *err = calloc(1, sizeof(error_t));
    if (err == nullptr)
        return err_errorf_alloc;

    va_list args;
    va_list args_count;
    va_start(args, fmt);
    va_copy(args_count, args);

    int size = vsnprintf(nullptr, 0, fmt, args_count) + 1;
    va_end(args_count);
    if (size <= 0) {
        free(err);
        va_end(args);
        return err_errorf_length;
    }

    err->message = malloc(size);
    if (err->message == nullptr) {
        free(err);
        va_end(args);
        return err_errorf_alloc;
    }

    vsnprintf(err->message, size, fmt, args);
    va_end(args);
    err->is_heap_allocated = true;
    return err;
}
