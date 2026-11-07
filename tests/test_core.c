/* Member A: parser boundaries and authorization checks, linked with test doubles. */
#include "app.h"
#include "input.h"
#include "demo_modules.h"
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static void test_parsers(void)
{
    int number = 77;
    Money cents = 77;
    const char *invalid_int[] = {"", " ", "1x", "1.5", "1 2", "--1",
        "999999999999999999999999999999999999", "0x10", "-1", "7"};
    const char *invalid_money[] = {"", " ", "-1", "+1", "1e2", "NaN", "inf", ".5",
        "1.", "1.001", "1,00", "1 2", "12x", "92233720368547758.08",
        "999999999999999999999999999999999999999999"};
    size_t i;
    assert(input_parse_int(" 6 ", 0, 6, &number) && number == 6);
    assert(input_parse_int("-2147483648", INT_MIN, INT_MAX, &number) && number == INT_MIN);
    assert(input_parse_int("2147483647", INT_MIN, INT_MAX, &number) && number == INT_MAX);
    for (i = 0; i < sizeof invalid_int / sizeof invalid_int[0]; ++i) {
        number = 77;
        assert(!input_parse_int(invalid_int[i], 0, 6, &number) && number == 77);
    }
    assert(!input_parse_int(NULL, 0, 6, &number));
    assert(!input_parse_int("1", 0, 6, NULL));
    assert(!input_parse_int("1", 6, 0, &number));
    assert(input_parse_money(" 12.3 ", 0, INT64_MAX, &cents) && cents == 1230);
    assert(input_parse_money("0.01", 1, INT64_MAX, &cents) && cents == 1);
    assert(input_parse_money("00012", 0, INT64_MAX, &cents) && cents == 1200);
    assert(input_parse_money("92233720368547758.07", 0, INT64_MAX, &cents) && cents == INT64_MAX);
    assert(input_parse_money("0", 0, INT64_MAX, &cents) && cents == 0);
    assert(!input_parse_money("0", 1, INT64_MAX, &cents));
    assert(!input_parse_money("100.01", 1, 10000, &cents));
    for (i = 0; i < sizeof invalid_money / sizeof invalid_money[0]; ++i) {
        cents = 77;
        assert(!input_parse_money(invalid_money[i], 0, INT64_MAX, &cents) && cents == 77);
    }
    assert(!input_parse_money(NULL, 0, 1, &cents));
    assert(!input_parse_money("1", 0, 1, NULL));
    assert(!input_parse_money("1", 2, 1, &cents));
    assert(!input_parse_money("1", -1, 100, &cents));
}

static void test_permissions(void)
{
    Session customer = {ROLE_CUSTOMER, 123};
    Session manager = {ROLE_MANAGER, 456};
    Session invalid[] = {{ROLE_NONE, 0}, {ROLE_CUSTOMER, 0}, {ROLE_MANAGER, 0}, {(UserRole)99, 123}};
    int action;
    size_t i;
    for (action = ACTION_BROWSE_MENU; action <= ACTION_SALES_STATISTICS; ++action) {
        unsigned int before = demo_call_count;
        const Session *allowed = action <= ACTION_EDIT_PROFILE ? &customer : &manager;
        const Session *denied = action <= ACTION_EDIT_PROFILE ? &manager : &customer;
        assert(app_dispatch(NULL, (AppAction)action) == APP_FORBIDDEN);
        assert(app_dispatch(denied, (AppAction)action) == APP_FORBIDDEN);
        for (i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
            assert(app_dispatch(&invalid[i], (AppAction)action) == APP_FORBIDDEN);
        }
        assert(demo_call_count == before); /* Rejection must not invoke a module. */
        assert(app_dispatch(allowed, (AppAction)action) == APP_NOT_IMPLEMENTED);
        assert(demo_call_count == before + 1);
        if (action >= ACTION_PLACE_ORDER && action <= ACTION_EDIT_PROFILE) {
            assert(demo_last_customer == customer.account_id);
        }
    }
    assert(app_dispatch(&manager, (AppAction)999) == APP_INVALID_INPUT);
    session_clear(&customer);
    assert(customer.role == ROLE_NONE && customer.account_id == 0);
    assert(!session_is_valid(&customer));
    assert(app_dispatch(&customer, ACTION_PLACE_ORDER) == APP_FORBIDDEN);
    session_clear(NULL);
    assert(strstr(app_status_message((AppStatus)999), "unexpected") != NULL);
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        InputResult result;
        if (strcmp(argv[1], "text") == 0) {
            char text[5];
            result = input_read_text("Text: ", text, sizeof text, 0);
            printf("RESULT %d %s\n", result, result == INPUT_OK ? text : "");
            return 0;
        }
        if (strcmp(argv[1], "money") == 0) {
            Money money = 0;
            result = input_read_money("Amount: ", 1, 100000, &money);
            printf("RESULT %d %" PRId64 "\n", result, money);
            return 0;
        }
        if (strcmp(argv[1], "confirm") == 0) {
            int yes = 0;
            result = input_confirm("Confirm [y/n]: ", &yes);
            printf("RESULT %d %d\n", result, yes);
            return 0;
        }
        if (strcmp(argv[1], "int") == 0) {
            int value = 0;
            result = input_read_int("Number: ", 0, 6, &value);
            printf("RESULT %d %d\n", result, value);
            return 0;
        }
        return 2;
    }
    test_parsers();
    test_permissions();
    puts("PASS: parser boundaries, all role guards, identity forwarding and logout.");
    return 0;
}
