#include "ui.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        puts("Usage: pizza-ordering-system [--help | --version]");
        puts("Run without arguments to open the customer/manager console.");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Pizza Ordering System 0.2.0 (member A framework)");
        return 0;
    }
    if (argc != 1) {
        fputs("Unknown argument. Use --help for usage.\n", stderr);
        return 2;
    }
#ifdef PIZZA_DEMO
    puts("=== DEMO ONLY: fake accounts, no real orders, payments or persistence ===");
    puts("Customer: 1001 / customer-demo | Manager: 9001 / manager-demo");
#endif
    return run_app();
}
