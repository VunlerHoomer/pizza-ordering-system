/* Owner A: development menu. Role-based business menus are still TODO. */
#include "ui.h"
#include "input.h"
#include "account.h"
#include "menu.h"
#include "order.h"
#include "wallet.h"
#include "stats.h"
#include "storage.h"
#include <stdio.h>

void run_app(void)
{
    int choice;
    puts("Pizza Ordering System - team starter");
    puts("Development scaffold only: business features and persistence are TODO.");
    for (;;) {
        AppStatus status = APP_NOT_IMPLEMENTED;
        puts("\n1. Accounts [B]\n2. Menu and pricing [C]\n3. Orders [D]");
        puts("4. Wallet [D]\n5. Sales statistics [E]\n6. Storage [E]\n0. Exit");
        if (!read_menu_choice(0, 6, &choice) || choice == 0) { break; }
        switch (choice) {
            case 1: status = account_demo(); break;
            case 2: status = menu_demo(); break;
            case 3: status = order_demo(); break;
            case 4: status = wallet_demo(); break;
            case 5: status = stats_demo(); break;
            case 6: status = storage_demo(); break;
            default: break;
        }
        if (status == APP_NOT_IMPLEMENTED) {
            puts("Not implemented yet. See docs/TEAM_PLAN.md for ownership.");
        }
    }
    puts("Goodbye.");
}
