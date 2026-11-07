#ifndef PIZZA_TYPES_H
#define PIZZA_TYPES_H

#include <stdint.h>

/* Shared foundation. Agree on Customer/Order structures before implementing them. */
typedef int64_t Money; /* Integer cents; never floating-point currency. */
typedef unsigned int CustomerId;
typedef unsigned int OrderId;

typedef enum {
    APP_OK = 0,
    APP_NOT_IMPLEMENTED,
    APP_INVALID_INPUT,
    APP_IO_ERROR
} AppStatus;

#endif
