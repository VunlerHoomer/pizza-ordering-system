/* A：主菜单。顾客和店长登录后，去各自的菜单。 */
#include <stdio.h>
#include "project.h"

/* 登录成功就返回账号；0 表示没登录，-1 表示输入结束。 */
static int login(const struct ShopData *data, int role)
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

    /* 不能选了店长菜单就直接进去，还要让 B 检查账号和身份。 */
    if (checkLogin(data, accountId, password, role) == 1) {
        printf("Login successful.\n");
        return accountId;
    }
    printf("Login failed.\n");
    return 0;
}

/* 返回 1 回首页，0 结束输入，-1 表示保存出错，要退出程序。 */
static int customerMenu(struct ShopData *data, int customerId)
{
    int choice, result;

    while (1) {
        printf("\n--- Customer %d ---\n", customerId);
        printf("1. View pizza menu\n2. Place an order\n3. View order history\n");
        printf("4. Deposit money\n5. View my information\n6. Edit my information\n0. Log out\n");
        choice = readInt("Choice: ", 0, 6);
        if (choice == -1) {
            return 0;
        }
        result = 0;
        switch (choice) {
            case 1: showPizzaMenu(data); break;
            case 2: result = placeOrder(data, customerId); break;
            case 3: showOrderHistory(data, customerId); break;
            case 4: result = depositMoney(data, customerId); break;
            case 5: showCustomer(data, customerId); break;
            case 6: result = editCustomer(data, customerId); break;
            case 0:
                printf("Logged out.\n");
                return 1;
        }
        if (result == -1) {
            return -1;            /* 存不下来就退出，不继续做下一笔操作。 */
        }
    }
}

static int managerMenu(struct ShopData *data)
{
    int choice, result;

    while (1) {
        printf("\n--- Shop manager ---\n");
        printf("1. Edit pizza menu\n2. Change prices\n3. Add customer\n");
        printf("4. Remove customer\n5. Edit customer\n6. View sales statistics\n0. Log out\n");
        choice = readInt("Choice: ", 0, 6);
        if (choice == -1) {
            return 0;
        }
        result = 0;
        switch (choice) {
            case 1: result = editPizzaMenu(data); break;
            case 2: result = editPrices(data); break;
            case 3: result = manageCustomer(data, 1); break;
            case 4: result = manageCustomer(data, 2); break;
            case 5: result = manageCustomer(data, 3); break;
            case 6: showStatistics(data); break;
            case 0:
                printf("Logged out.\n");
                return 1;
        }
        if (result == -1) {
            return -1;
        }
    }
}

int main(void)
{
    /* 各个部分共用这一份数据，传地址过去就能修改原来的内容。 */
    struct ShopData data = {0};
    int choice, accountId;
    int running = 1;
    int result = 0;

    if (loadData(&data) != 1) {
        printf("Cannot load data. The program will close without saving.\n");
        return 1;
    }
    while (running) {
        printf("\n=== Pizza Ordering System ===\n");
        printf("1. Customer login\n2. Register\n3. Manager login\n0. Exit\n");
        choice = readInt("Choice: ", 0, 3);
        result = 0;
        switch (choice) {
            case 1:
                accountId = login(&data, CUSTOMER_ROLE);
                if (accountId == -1) {
                    running = 0;
                } else if (accountId > 0) {
                    result = customerMenu(&data, accountId);
                    if (result <= 0) {
                        running = 0;
                    }
                }
                break;
            case 2:
                result = registerCustomer(&data);
                break;
            case 3:
                accountId = login(&data, MANAGER_ROLE);
                if (accountId == -1) {
                    running = 0;
                } else if (accountId > 0) {
                    result = managerMenu(&data);
                    if (result <= 0) {
                        running = 0;
                    }
                }
                break;
            case 0:
            case -1:
                running = 0;
                break;
        }
        if (result == -1) {
            printf("Save failed. Stop using the program and check shop_backup.txt.\n");
            return 1;             /* 这里直接结束，不能再存一次把备份盖掉。 */
        }
    }

    if (ferror(stdin)) {
        printf("There was an input error.\n");
        return 1;
    }
    /* 正常退出时保存一次。如果出错，先保留文件，交给人检查。 */
    if (saveData(&data) != 1) {
        printf("Data was not saved. Check shop_backup.txt before restarting.\n");
        return 1;
    }
    printf("Goodbye.\n");
    return 0;
}
