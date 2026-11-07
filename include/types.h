#ifndef PIZZA_TYPES_H
#define PIZZA_TYPES_H

#include <stdint.h>

/* Shared foundation. Customer/Order storage records remain owned by B/D/E. */
typedef int64_t Money; /* Integer cents; never floating-point currency. */
typedef unsigned int CustomerId;
typedef unsigned int OrderId;

typedef enum {
    APP_OK = 0,
    APP_NOT_IMPLEMENTED,
    APP_INVALID_INPUT,
    APP_IO_ERROR,
    APP_AUTH_FAILED,
    APP_FORBIDDEN,
    APP_CANCELLED,
    APP_DATA_ERROR
} AppStatus;

typedef enum {
    ROLE_NONE = 0,
    ROLE_CUSTOMER,
    ROLE_MANAGER
} UserRole;

/* A owns session lifetime; B supplies the authenticated role and identity. */
typedef struct {
    UserRole role;
    unsigned int account_id; /* Nonzero for both customers and managers. */
} Session;

#endif
