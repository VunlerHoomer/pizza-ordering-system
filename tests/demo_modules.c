/* TEST DOUBLE ONLY. Never link this file into the normal application. */
#include "demo_modules.h"
#include "account.h"
#include "menu.h"
#include "pricing.h"
#include "order.h"
#include "wallet.h"
#include "stats.h"
#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int demo_call_count = 0;
CustomerId demo_last_customer = 0;
const char *demo_last_operation = "";

static int flag(const char *name, const char *value)
{
    const char *setting = getenv(name);
    return setting != NULL && strcmp(setting, value) == 0;
}

static AppStatus record(const char *operation, CustomerId customer_id)
{
    ++demo_call_count;
    demo_last_customer = customer_id;
    demo_last_operation = operation;
    printf("[DEMO] %s customer=%u (no business data changed)\n", operation, customer_id);
    if (flag("PIZZA_TEST_ACTION", "io_error")) { return APP_IO_ERROR; }
    if (flag("PIZZA_TEST_ACTION", "cancelled")) { return APP_CANCELLED; }
    return APP_NOT_IMPLEMENTED;
}

AppStatus account_authenticate(UserRole role, unsigned int account_id,
                               const char *password, Session *out_session)
{
    if (out_session == NULL || password == NULL) { return APP_INVALID_INPUT; }
    out_session->role = ROLE_NONE;
    out_session->account_id = 0;
    if ((role == ROLE_CUSTOMER && account_id == 1001 && strcmp(password, "customer-demo") == 0) ||
        (role == ROLE_MANAGER && account_id == 9001 && strcmp(password, "manager-demo") == 0)) {
        out_session->role = role;
        out_session->account_id = account_id;
        /* Exercise A's validation of an incorrectly implemented auth provider. */
        if (flag("PIZZA_TEST_AUTH", "wrong_role")) {
            out_session->role = role == ROLE_CUSTOMER ? ROLE_MANAGER : ROLE_CUSTOMER;
        }
        if (flag("PIZZA_TEST_AUTH", "wrong_id")) { ++out_session->account_id; }
        if (flag("PIZZA_TEST_AUTH", "zero_id")) { out_session->account_id = 0; }
        return APP_OK;
    }
    return APP_AUTH_FAILED;
}

AppStatus account_register(void) { return record("account_register", 0); }
AppStatus account_view_profile(CustomerId id) { return record("account_view_profile", id); }
AppStatus account_edit_profile(CustomerId id) { return record("account_edit_profile", id); }
AppStatus account_manage_customers(AccountManagementAction action)
{
    switch (action) {
        case ACCOUNT_ADD: return record("account_add_customer", 0);
        case ACCOUNT_REMOVE: return record("account_remove_customer", 0);
        case ACCOUNT_EDIT: return record("account_edit_customer", 0);
        default: return APP_INVALID_INPUT;
    }
}
AppStatus menu_show(void) { return record("menu_show", 0); }
AppStatus menu_manage(void) { return record("menu_manage", 0); }
AppStatus pricing_manage(void) { return record("pricing_manage", 0); }
AppStatus order_place(CustomerId id) { return record("order_place", id); }
AppStatus order_history(CustomerId id) { return record("order_history", id); }
AppStatus wallet_deposit(CustomerId id) { return record("wallet_deposit", id); }
AppStatus stats_show(void) { return record("stats_show", 0); }

AppStatus storage_load(void)
{
    puts("[DEMO] storage_load (no files read)");
    if (flag("PIZZA_TEST_LOAD", "io_error")) { return APP_IO_ERROR; }
    if (flag("PIZZA_TEST_LOAD", "data_error")) { return APP_DATA_ERROR; }
    return APP_OK;
}

AppStatus storage_save(void)
{
    static unsigned int attempts = 0;
    ++attempts;
    puts("[DEMO] storage_save (no files written)");
    if (flag("PIZZA_TEST_SAVE", "always") ||
        (flag("PIZZA_TEST_SAVE", "once") && attempts == 1)) { return APP_IO_ERROR; }
    return APP_OK;
}
