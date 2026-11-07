/* Member A: session state, role guard, module routing and shared feedback. */
#include "app.h"
#include "account.h"
#include "menu.h"
#include "pricing.h"
#include "order.h"
#include "wallet.h"
#include "stats.h"
#include <stddef.h>

void session_clear(Session *session)
{
    if (session != NULL) {
        session->role = ROLE_NONE;
        session->account_id = 0;
    }
}

int session_is_valid(const Session *session)
{
    return session != NULL && session->account_id > 0 &&
           (session->role == ROLE_CUSTOMER || session->role == ROLE_MANAGER);
}

AppStatus app_dispatch(const Session *session, AppAction action)
{
    UserRole required;
    switch (action) {
        case ACTION_BROWSE_MENU:
        case ACTION_PLACE_ORDER:
        case ACTION_ORDER_HISTORY:
        case ACTION_DEPOSIT:
        case ACTION_VIEW_PROFILE:
        case ACTION_EDIT_PROFILE: required = ROLE_CUSTOMER; break;
        case ACTION_MANAGE_MENU:
        case ACTION_MANAGE_PRICES:
        case ACTION_ADD_CUSTOMER:
        case ACTION_REMOVE_CUSTOMER:
        case ACTION_EDIT_CUSTOMER:
        case ACTION_SALES_STATISTICS: required = ROLE_MANAGER; break;
        default: return APP_INVALID_INPUT;
    }
    if (!session_is_valid(session) || session->role != required) {
        return APP_FORBIDDEN;
    }
    switch (action) {
        case ACTION_BROWSE_MENU: return menu_show();
        case ACTION_PLACE_ORDER: return order_place(session->account_id);
        case ACTION_ORDER_HISTORY: return order_history(session->account_id);
        case ACTION_DEPOSIT: return wallet_deposit(session->account_id);
        case ACTION_VIEW_PROFILE: return account_view_profile(session->account_id);
        case ACTION_EDIT_PROFILE: return account_edit_profile(session->account_id);
        case ACTION_MANAGE_MENU: return menu_manage();
        case ACTION_MANAGE_PRICES: return pricing_manage();
        case ACTION_ADD_CUSTOMER: return account_manage_customers(ACCOUNT_ADD);
        case ACTION_REMOVE_CUSTOMER: return account_manage_customers(ACCOUNT_REMOVE);
        case ACTION_EDIT_CUSTOMER: return account_manage_customers(ACCOUNT_EDIT);
        case ACTION_SALES_STATISTICS: return stats_show();
        default: return APP_INVALID_INPUT;
    }
}

const char *app_status_message(AppStatus status)
{
    switch (status) {
        case APP_OK: return "Operation completed.";
        case APP_NOT_IMPLEMENTED: return "This feature is not available yet.";
        case APP_INVALID_INPUT: return "The supplied information is invalid.";
        case APP_IO_ERROR: return "A read or write failed. Please try again.";
        case APP_AUTH_FAILED: return "Incorrect account number, password or account role.";
        case APP_FORBIDDEN: return "Access denied for this account.";
        case APP_CANCELLED: return "Operation cancelled.";
        case APP_DATA_ERROR: return "Stored data is invalid. Please contact the shop manager.";
        default: return "An unexpected error occurred.";
    }
}
