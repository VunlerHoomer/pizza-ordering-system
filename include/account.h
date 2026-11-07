#ifndef PIZZA_ACCOUNT_H
#define PIZZA_ACCOUNT_H

#include "types.h"

/* Temporary integration entry point; business API is TODO. */
AppStatus account_demo(void);

/* Member A integration contract; implementation owned by B. */
typedef enum {
    ACCOUNT_ADD,
    ACCOUNT_REMOVE,
    ACCOUNT_EDIT
} AccountManagementAction;

/* B must validate credentials AND role. On failure, out_session stays logged out. */
AppStatus account_authenticate(UserRole role, unsigned int account_id,
                               const char *password, Session *out_session);
AppStatus account_register(void);
AppStatus account_view_profile(CustomerId customer_id);
AppStatus account_edit_profile(CustomerId customer_id);
/* A checks the manager session before calling this handler. */
AppStatus account_manage_customers(AccountManagementAction action);

#endif
