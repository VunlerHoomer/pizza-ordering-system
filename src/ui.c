/* Member A: iterative role menus and application startup/shutdown. */
#include "ui.h"
#include "input.h"
#include "app.h"
#include "account.h"
#include "storage.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* EOF exits the application; zero/empty password cancels login. */
static InputResult login(UserRole role, Session *session)
{
    int number;
    char password[65];
    Session candidate = {ROLE_NONE, 0};
    AppStatus status;
    InputResult result;
    session_clear(session);
    result = input_read_int("Account number (0 to cancel): ", 0, INT_MAX, &number);
    if (result != INPUT_OK || number == 0) { return result; }
    result = input_read_text("Password (empty to cancel): ", password, sizeof password, 1);
    if (result != INPUT_OK || password[0] == '\0') {
        memset(password, 0, sizeof password);
        return result;
    }
    status = account_authenticate(role, (unsigned int)number, password, &candidate);
    memset(password, 0, sizeof password);
    if (status != APP_OK) {
        puts(app_status_message(status));
        return INPUT_OK;
    }
    /* Never grant access on a malformed or mismatched authentication result. */
    if (!session_is_valid(&candidate) || candidate.role != role ||
        candidate.account_id != (unsigned int)number) {
        puts("Login could not be completed: invalid authentication result.");
        return INPUT_OK;
    }
    *session = candidate;
    puts("Login successful.");
    return INPUT_OK;
}

static void print_home(void)
{
    puts("\n=== Pizza Ordering System ===");
    puts("1. Customer login\n2. Register customer account\n3. Shop manager login\n0. Exit");
}

static void print_role_menu(const Session *session)
{
    printf("\n=== %s | Account %u ===\n",
           session->role == ROLE_CUSTOMER ? "Customer" : "Shop manager", session->account_id);
    if (session->role == ROLE_CUSTOMER) {
        puts("1. Browse pizza menu\n2. Place an order\n3. View order history");
        puts("4. Deposit money\n5. View my profile\n6. Edit my profile\n0. Log out");
    } else {
        puts("1. Edit pizza menu\n2. Set pizza prices\n3. Add customer");
        puts("4. Remove customer\n5. Edit customer\n6. View sales statistics\n0. Log out");
    }
}

static int finish_storage(void)
{
    for (;;) {
        int retry;
        AppStatus status = storage_save();
        if (status == APP_OK) { return 0; }
        puts(app_status_message(status));
        puts("Data has NOT been confirmed saved. 1. Retry save  0. Exit without saving");
        if (input_read_int("Choice: ", 0, 1, &retry) != INPUT_OK || retry == 0) {
            fputs("Exiting with unsaved data.\n", stderr);
            return 1;
        }
    }
}

int run_app(void)
{
    static const AppAction customer_actions[] = {
        ACTION_BROWSE_MENU, ACTION_PLACE_ORDER, ACTION_ORDER_HISTORY,
        ACTION_DEPOSIT, ACTION_VIEW_PROFILE, ACTION_EDIT_PROFILE
    };
    static const AppAction manager_actions[] = {
        ACTION_MANAGE_MENU, ACTION_MANAGE_PRICES, ACTION_ADD_CUSTOMER,
        ACTION_REMOVE_CUSTOMER, ACTION_EDIT_CUSTOMER, ACTION_SALES_STATISTICS
    };
    Session session = {ROLE_NONE, 0};
    AppStatus load_status;
    InputResult input_status = INPUT_OK;
    int exit_status = 0;

    load_status = storage_load();
    if (load_status == APP_NOT_IMPLEMENTED) {
        puts("Data storage is unavailable in this build; changes cannot be saved.");
    } else if (load_status != APP_OK) {
        fprintf(stderr, "Cannot open shop data: %s\n", app_status_message(load_status));
        return 1; /* Do not overwrite data after a failed load. */
    }

    for (;;) {
        int choice;
        if (!session_is_valid(&session)) {
            session_clear(&session);
            print_home();
            input_status = input_read_int("Choice: ", 0, 3, &choice);
            if (input_status != INPUT_OK || choice == 0) { break; }
            if (choice == 2) {
                puts(app_status_message(account_register()));
            } else {
                input_status = login(choice == 1 ? ROLE_CUSTOMER : ROLE_MANAGER, &session);
                if (input_status != INPUT_OK) { break; }
            }
        } else {
            const AppAction *actions = session.role == ROLE_CUSTOMER ?
                                       customer_actions : manager_actions;
            print_role_menu(&session);
            input_status = input_read_int("Choice: ", 0, 6, &choice);
            if (input_status != INPUT_OK) { break; }
            if (choice == 0) {
                session_clear(&session);
                puts("Logged out.");
            } else {
                puts(app_status_message(app_dispatch(&session, actions[choice - 1])));
            }
        }
    }
    session_clear(&session);
    if (input_status == INPUT_ERROR) {
        fputs("Input could not be read.\n", stderr);
        exit_status = 1;
    }
    if (load_status == APP_OK && finish_storage() != 0) { exit_status = 1; }
    puts("Goodbye.");
    return exit_status;
}
