#ifndef INCLUDE_SRC_ERROR_H_
#define INCLUDE_SRC_ERROR_H_

#include <stdlib.h>

typedef struct error {
    char *message;
    bool is_heap_allocated;
} error_t;

error_t *errorf(const char *fmt, ...);
static inline void error_free(error_t *err) {
    if (err == nullptr)
        return;
    if (!err->is_heap_allocated)
        return;
    free(err->message);
    free(err);
}

/* Some global errors */
extern error_t *err_allocation_failed;

#endif // INCLUDE_SRC_ERROR_H_
