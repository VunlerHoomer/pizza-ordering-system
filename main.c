/* 成员 A：程序入口、登录入口、顾客菜单和店长菜单。 */
#include <stdio.h>
#include "project.h"

/* 返回账号表示登录成功，0 表示取消或失败，-1 表示输入结束。 */
static int login(int role)
{
    int accountId;
    char password[32];

    accountId = readInt("Account number (0 to cancel): ", 0, 999999);
    if (accountId <= 0) {
        return accountId;
    }
    if (!readText("Password (empty to cancel): ", password, sizeof(password))) {
        return -1;
    }
    if (password[0] == '\0') {
        return 0;
    }

    /* 选择了店长入口不代表有权限，必须由 B 核对真实角色。 */
    if (checkLogin(accountId, password, role) == 1) {
        printf("Login successful.\n");
        return accountId;
    }
    printf("Login failed.\n");
    return 0;
}

/* 返回 1 表示退出登录，返回 0 表示输入结束。 */
static int customerMenu(int customerId)
{
    int choice;

    while (1) {
        printf("\n--- Customer %d ---\n", customerId);
        printf("1. View pizza menu\n2. Place an order\n3. View order history\n");
        printf("4. Deposit money\n5. View my information\n6. Edit my information\n0. Log out\n");
        choice = readInt("Choice: ", 0, 6);
        if (choice == -1) {
            return 0;
        }
        switch (choice) {
            case 1: showPizzaMenu(); break;
            case 2: placeOrder(customerId); break;
            case 3: showOrderHistory(customerId); break;
            case 4: depositMoney(customerId); break;
            case 5: showCustomer(customerId); break;
            case 6: editCustomer(customerId); break;
            case 0:
                printf("Logged out.\n");
                return 1;
        }
    }
}

static int managerMenu(void)
{
    int choice;

    while (1) {
        printf("\n--- Shop manager ---\n");
        printf("1. Edit pizza menu\n2. Change prices\n3. Add customer\n");
        printf("4. Remove customer\n5. Edit customer\n6. View sales statistics\n0. Log out\n");
        choice = readInt("Choice: ", 0, 6);
        if (choice == -1) {
            return 0;
        }
        switch (choice) {
            case 1: editPizzaMenu(); break;
            case 2: editPrices(); break;
            case 3: manageCustomer(1); break;
            case 4: manageCustomer(2); break;
            case 5: manageCustomer(3); break;
            case 6: showStatistics(); break;
            case 0:
                printf("Logged out.\n");
                return 1;
        }
    }
}

int main(void)
{
    int choice;
    int accountId;
    int running = 1;
    int loaded;

    loaded = loadData();
    if (loaded == 0) {
        printf("Cannot load data. The program will close to avoid overwriting it.\n");
        return 1;
    }
    if (loaded == -1) {
        printf("File saving is not available in this unfinished version.\n");
    }

    while (running) {
        printf("\n=== Pizza Ordering System ===\n");
        printf("1. Customer login\n2. Register\n3. Manager login\n0. Exit\n");
        choice = readInt("Choice: ", 0, 3);
        switch (choice) {
            case 1:
                accountId = login(CUSTOMER_ROLE);
                if (accountId == -1) {
                    running = 0;
                } else if (accountId > 0) {
                    running = customerMenu(accountId);
                }
                break;
            case 2:
                registerCustomer();
                break;
            case 3:
                accountId = login(MANAGER_ROLE);
                if (accountId == -1) {
                    running = 0;
                } else if (accountId > 0) {
                    running = managerMenu();
                }
                break;
            case 0:
            case -1:
                running = 0;
                break;
        }
    }

    /* 只有加载成功后才保存。保存失败时可以选择重试。 */
    if (loaded == 1) {
        while (saveData() != 1) {
            printf("Save failed. 1. Retry  0. Exit without saving\n");
            if (readInt("Choice: ", 0, 1) != 1) {
                printf("Data was not saved.\n");
                return 1;
            }
        }
    }
    if (ferror(stdin)) {
        printf("There was an input error.\n");
        return 1;
    }
    printf("Goodbye.\n");
    return 0;
}
