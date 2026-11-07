#ifndef PIZZA_APP_H
#define PIZZA_APP_H

#include "types.h"

typedef enum {
    ACTION_BROWSE_MENU,
    ACTION_PLACE_ORDER,
    ACTION_ORDER_HISTORY,
    ACTION_DEPOSIT,
    ACTION_VIEW_PROFILE,
    ACTION_EDIT_PROFILE,
    ACTION_MANAGE_MENU,
    ACTION_MANAGE_PRICES,
    ACTION_ADD_CUSTOMER,
    ACTION_REMOVE_CUSTOMER,
    ACTION_EDIT_CUSTOMER,
    ACTION_SALES_STATISTICS
} AppAction;

void session_clear(Session *session);
int session_is_valid(const Session *session);
/* Authorizes before dispatch; customer identity comes only from the session. */
AppStatus app_dispatch(const Session *session, AppAction action);
const char *app_status_message(AppStatus status);

#endif
