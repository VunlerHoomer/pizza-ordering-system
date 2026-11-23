/* B：注册、登录、查看和修改顾客资料。 */
#include <stdio.h>
#include <string.h>
#include "project.h"

/* 账号 1 留给店长，顾客从 2 开始。
 * sample.txt 中已经放了店长账号，改编号时要一起改样例和说明。
 */
#define MANAGER_ID 1
#define MAX_CUSTOMER_ID 999999

/* 把“分”显示成“元”，例如 1050 -> 10.50 元 */
static void printMoney(int cents)
{
    printf("%d.%02d 元", cents / 100, cents % 100);
}

/* 从 2 开始找一个没用过的编号。找不到就返回 -1。 */
static int findNewId(const struct ShopData *data)
{
    int id;

    for (id = MANAGER_ID + 1; id <= MAX_CUSTOMER_ID; id++) {
        if (findCustomerIndex(data, id) < 0) {
            return id;
        }
    }
    return -1;
}

int findCustomerIndex(const struct ShopData *data, int customerId)
{
    int i;

    if (data == NULL || data->customerCount < 0 || data->customerCount > MAX_CUSTOMERS) {
        return -1;
    }
    for (i = 0; i < data->customerCount; i++) {
        if (data->customers[i].id == customerId) {
            return i;             /* 停用的也算已有账号，不能把编号分给新人。 */
        }
    }
    return -1;
}

int checkLogin(const struct ShopData *data, int accountId,
               const char password[], int role)
{
    int pos;

    if (data == NULL || password == NULL || accountId <= 0 || password[0] == '\0' ||
        (role != CUSTOMER_ROLE && role != MANAGER_ROLE)) {
        return 0;
    }

    pos = findCustomerIndex(data, accountId);
    if (pos < 0) {
        return 0;                 /* 账号不存在 */
    }
    if (strcmp(data->customers[pos].password, password) != 0) {
        return 0;                 /* 密码错误 */
    }
    if (data->customers[pos].active != 1) {
        return 0;                 /* 账号已停用 */
    }

    /* 还要检查是不是选对了入口，顾客不能从店长入口登录。 */
    if (role == MANAGER_ROLE && data->customers[pos].id != MANAGER_ID) {
        return 0;
    }
    if (role == CUSTOMER_ROLE && data->customers[pos].id == MANAGER_ID) {
        return 0;
    }

    return 1;
}

int registerCustomer(struct ShopData *data)
{
    struct Customer newCustomer = {0};
    struct Customer emptyCustomer = {0};
    char name[sizeof(newCustomer.name)];
    char phone[sizeof(newCustomer.phone)];
    char password[sizeof(newCustomer.password)];
    int id;
    int pos;

    if (data == NULL) {
        return 0;
    }
    if (data->customerCount >= MAX_CUSTOMERS) {
        printf("顾客已满 %d 人，不能再注册。\n", MAX_CUSTOMERS);
        return 0;
    }

    id = findNewId(data);
    if (id < 0) {
        printf("没有可用的顾客编号。\n");
        return 0;
    }

    if (!readText("姓名：", name, (int)sizeof(name))) {
        return 0;
    }
    if (!readText("电话：", phone, (int)sizeof(phone))) {
        return 0;
    }
    if (!readText("密码：", password, (int)sizeof(password))) {
        return 0;
    }
    if (name[0] == '\0' || phone[0] == '\0' || password[0] == '\0') {
        printf("姓名、电话、密码都不能为空。\n");
        return 0;
    }

    newCustomer.id = id;
    /* readText 已按同样大小的数组限制了长度，这里可以直接复制。 */
    strcpy(newCustomer.name, name);
    strcpy(newCustomer.phone, phone);
    strcpy(newCustomer.password, password);
    newCustomer.balance = 0;
    newCustomer.active = 1;

    pos = data->customerCount;
    data->customers[pos] = newCustomer;
    data->customerCount++;

    if (saveData(data) != 1) {
        /* 没存下来，把人数减回去，刚加的位置清空。 */
        data->customerCount--;
        data->customers[pos] = emptyCustomer;
        return -1;
    }

    printf("注册成功，你的账号是 %d，请记住。\n", id);
    return 1;
}

void showCustomer(const struct ShopData *data, int customerId)
{
    int pos;

    if (data == NULL) {
        return;
    }

    pos = findCustomerIndex(data, customerId);
    if (pos < 0) {
        printf("顾客不存在。\n");
        return;
    }

    printf("编号：%d\n", data->customers[pos].id);
    printf("姓名：%s\n", data->customers[pos].name);
    printf("电话：%s\n", data->customers[pos].phone);
    printf("余额：");
    printMoney(data->customers[pos].balance);
    printf("\n状态：%s\n", data->customers[pos].active ? "启用" : "停用");
}

int editCustomer(struct ShopData *data, int customerId)
{
    struct Customer oldCustomer;
    char name[sizeof(oldCustomer.name)];
    char phone[sizeof(oldCustomer.phone)];
    char password[sizeof(oldCustomer.password)];
    int pos;
    int changed = 0;

    if (data == NULL) {
        return 0;
    }

    pos = findCustomerIndex(data, customerId);
    if (pos < 0) {
        printf("顾客不存在。\n");
        return 0;
    }

    printf("直接回车表示该项不改。\n");
    if (!readText("新姓名：", name, (int)sizeof(name))) {
        return 0;
    }
    if (!readText("新电话：", phone, (int)sizeof(phone))) {
        return 0;
    }
    if (!readText("新密码：", password, (int)sizeof(password))) {
        return 0;
    }

    /* 先留住原资料，保存失败时要改回去。 */
    oldCustomer = data->customers[pos];

    /* 输入长度已经检查过；空行表示不改，不空才复制。 */
    if (name[0] != '\0' && strcmp(name, oldCustomer.name) != 0) {
        strcpy(data->customers[pos].name, name);
        changed = 1;
    }
    if (phone[0] != '\0' && strcmp(phone, oldCustomer.phone) != 0) {
        strcpy(data->customers[pos].phone, phone);
        changed = 1;
    }
    if (password[0] != '\0' && strcmp(password, oldCustomer.password) != 0) {
        strcpy(data->customers[pos].password, password);
        changed = 1;
    }

    if (!changed) {
        printf("没有修改。\n");
        return 0;
    }

    if (saveData(data) != 1) {
        data->customers[pos] = oldCustomer;   /* 没存成功，整条资料改回原来的。 */
        return -1;
    }

    return 1;
}

int manageCustomer(struct ShopData *data, int operation)
{
    struct Customer oldCustomer;
    int id;
    int pos;

    if (data == NULL) {
        return 0;
    }

    /* 添加顾客和自己注册一样，用同一个函数。 */
    if (operation == 1) {
        return registerCustomer(data);
    }

    /* 停用前先看余额。记录还留着，只把 active 改成 0。 */
    if (operation == 2) {
        id = readInt("要停用的顾客编号：", 1, MAX_CUSTOMER_ID);
        if (id == -1) {
            return 0;
        }
        if (id == MANAGER_ID) {
            printf("不能停用店长账号。\n");
            return 0;
        }

        pos = findCustomerIndex(data, id);
        if (pos < 0) {
            printf("顾客不存在。\n");
            return 0;
        }
        if (data->customers[pos].active != 1) {
            printf("该顾客已经是停用状态。\n");
            return 0;
        }
        if (data->customers[pos].balance != 0) {
            printf("该顾客余额不为 0，不能停用。\n");
            return 0;
        }

        oldCustomer = data->customers[pos];
        data->customers[pos].active = 0;

        if (saveData(data) != 1) {
            data->customers[pos] = oldCustomer;
            return -1;
        }
        return 1;
    }

    /* 选好顾客后，调用上面的修改资料函数。 */
    if (operation == 3) {
        id = readInt("要编辑的顾客编号：", 1, MAX_CUSTOMER_ID);
        if (id == -1) {
            return 0;
        }
        return editCustomer(data, id);
    }

    printf("无效操作。\n");
    return 0;
}
