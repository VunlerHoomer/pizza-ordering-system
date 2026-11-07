/* Owner B: Registration, login, profile editing and manager customer administration. */
#include "account.h"

AppStatus account_demo(void)
{
    /* TODO: agree on the business API, then replace this scaffold. */
    return APP_NOT_IMPLEMENTED;
}

AppStatus account_authenticate(UserRole role, unsigned int account_id,
                               const char *password, Session *out_session)
{
    if (out_session == 0) { return APP_INVALID_INPUT; }
    out_session->role = ROLE_NONE;
    out_session->account_id = 0;
    if (password == 0 || account_id == 0 ||
        (role != ROLE_CUSTOMER && role != ROLE_MANAGER)) {
        return APP_INVALID_INPUT;
    }
    /* TODO B: authenticate against actual account data. No default login. */
    return APP_NOT_IMPLEMENTED;
}

AppStatus account_register(void)
{
    /* TODO B: implement this operation; do not report success before it exists. */
    return APP_NOT_IMPLEMENTED;
}

AppStatus account_view_profile(CustomerId customer_id)
{
    (void)customer_id;
    /* TODO B: implement this operation; do not report success before it exists. */
    return APP_NOT_IMPLEMENTED;
}

AppStatus account_edit_profile(CustomerId customer_id)
{
    (void)customer_id;
    /* TODO B: implement this operation; do not report success before it exists. */
    return APP_NOT_IMPLEMENTED;
}

AppStatus account_manage_customers(AccountManagementAction action)
{
    (void)action;
    /* TODO B: implement this operation; do not report success before it exists. */
    return APP_NOT_IMPLEMENTED;
}
