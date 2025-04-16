#include "bytes.h"
#include "error.h"

error_t *const err_bytes_no_capacity = &(error_t){
    .message = "Not enough capacity in bytes buffer",
};
