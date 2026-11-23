/* 成员 B：账户管理 */
#include <stdio.h>
#include <string.h>
#include "project.h"

/*
 * project.h 的 Customer 没有角色字段，checkLogin 只能按账号编号区分店长和顾客。
 * 约定：店长 id = 1，顾客 id 从 2 开始自动分配。
 * sample.txt 已放入店长示例账号。改店长编号时，样例和说明也要一起改。
 */
#define MANAGER_ID 1
#define MAX_CUSTOMER_ID 999999

/* 把“分”显示成“元”，例如 1050 -> 10.50 元 */
static void printMoney(int cents)
{
    printf("%d.%02d 元", cents / 100, cents % 100);
}

/* 分配一个未被占用的顾客编号；找不到返回 -1 */
static int allocateCustomerId(const struct ShopData *data)
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
            return i;             /* 停用记录也能找到，供历史和管理功能使用。 */
        }
    }
    return -1;
}

int checkLogin(const struct ShopData *data, int accountId,
               const char password[], int role)
{
    int index;

    if (data == NULL || password == NULL || accountId <= 0 || password[0] == '\0' ||
        (role != CUSTOMER_ROLE && role != MANAGER_ROLE)) {
        return 0;
    }

    index = findCustomerIndex(data, accountId);
    if (index < 0) {
        return 0;                 /* 账号不存在 */
    }
    if (strcmp(data->customers[index].password, password) != 0) {
        return 0;                 /* 密码错误 */
    }
    if (data->customers[index].active != 1) {
        return 0;                 /* 账号已停用 */
    }

    /* 核对真实角色：店长只能用 MANAGER_ROLE 登录，顾客只能用 CUSTOMER_ROLE 登录 */
    if (role == MANAGER_ROLE && data->customers[index].id != MANAGER_ID) {
        return 0;
    }
    if (role == CUSTOMER_ROLE && data->customers[index].id == MANAGER_ID) {
        return 0;
    }

    return 1;
}

int registerCustomer(struct ShopData *data)
{
    struct Customer c;
    char name[sizeof(c.name)];
    char phone[sizeof(c.phone)];
    char password[sizeof(c.password)];
    int id;
    int index;

    if (data == NULL) {
        return 0;
    }
    if (data->customerCount >= MAX_CUSTOMERS) {
        printf("顾客已满 %d 人，不能再注册。\n", MAX_CUSTOMERS);
        return 0;
    }

    id = allocateCustomerId(data);
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

    memset(&c, 0, sizeof(c));
    c.id = id;
    strncpy(c.name, name, sizeof(c.name) - 1);
    c.name[sizeof(c.name) - 1] = '\0';
    strncpy(c.phone, phone, sizeof(c.phone) - 1);
    c.phone[sizeof(c.phone) - 1] = '\0';
    strncpy(c.password, password, sizeof(c.password) - 1);
    c.password[sizeof(c.password) - 1] = '\0';
    c.balance = 0;
    c.active = 1;

    index = data->customerCount;
    data->customers[index] = c;
    data->customerCount++;

    if (saveData(data) != 1) {
        /* 保存失败：撤销本次新增 */
        data->customerCount--;
        memset(&data->customers[index], 0, sizeof(struct Customer));
        return -1;
    }

    printf("注册成功，你的账号是 %d，请记住。\n", id);
    return 1;
}

void showCustomer(const struct ShopData *data, int customerId)
{
    int index;

    if (data == NULL) {
        return;
    }

    index = findCustomerIndex(data, customerId);
    if (index < 0) {
        printf("顾客不存在。\n");
        return;
    }

    printf("编号：%d\n", data->customers[index].id);
    printf("姓名：%s\n", data->customers[index].name);
    printf("电话：%s\n", data->customers[index].phone);
    printf("余额：");
    printMoney(data->customers[index].balance);
    printf("\n状态：%s\n", data->customers[index].active ? "启用" : "停用");
}

int editCustomer(struct ShopData *data, int customerId)
{
    struct Customer old;
    char name[sizeof(old.name)];
    char phone[sizeof(old.phone)];
    char password[sizeof(old.password)];
    int index;
    int changed = 0;

    if (data == NULL) {
        return 0;
    }

    index = findCustomerIndex(data, customerId);
    if (index < 0) {
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

    old = data->customers[index];

    if (name[0] != '\0' && strcmp(name, old.name) != 0) {
        strncpy(data->customers[index].name, name,
                sizeof(data->customers[index].name) - 1);
        data->customers[index].name[sizeof(data->customers[index].name) - 1] = '\0';
        changed = 1;
    }
    if (phone[0] != '\0' && strcmp(phone, old.phone) != 0) {
        strncpy(data->customers[index].phone, phone,
                sizeof(data->customers[index].phone) - 1);
        data->customers[index].phone[sizeof(data->customers[index].phone) - 1] = '\0';
        changed = 1;
    }
    if (password[0] != '\0' && strcmp(password, old.password) != 0) {
        strncpy(data->customers[index].password, password,
                sizeof(data->customers[index].password) - 1);
        data->customers[index].password[sizeof(data->customers[index].password) - 1] = '\0';
        changed = 1;
    }

    if (!changed) {
        printf("没有修改。\n");
        return 0;
    }

    if (saveData(data) != 1) {
        data->customers[index] = old;   /* 保存失败：恢复原值 */
        return -1;
    }

    return 1;
}

int manageCustomer(struct ShopData *data, int operation)
{
    struct Customer old;
    int id;
    int index;

    if (data == NULL) {
        return 0;
    }

    /* 1 添加：复用注册逻辑，自动分配编号 */
    if (operation == 1) {
        return registerCustomer(data);
    }

    /* 2 停用：余额必须为 0，保留记录，只改 active */
    if (operation == 2) {
        id = readInt("要停用的顾客编号：", 1, MAX_CUSTOMER_ID);
        if (id == -1) {
            return 0;
        }
        if (id == MANAGER_ID) {
            printf("不能停用店长账号。\n");
            return 0;
        }

        index = findCustomerIndex(data, id);
        if (index < 0) {
            printf("顾客不存在。\n");
            return 0;
        }
        if (data->customers[index].active != 1) {
            printf("该顾客已经是停用状态。\n");
            return 0;
        }
        if (data->customers[index].balance != 0) {
            printf("该顾客余额不为 0，不能停用。\n");
            return 0;
        }

        old = data->customers[index];
        data->customers[index].active = 0;

        if (saveData(data) != 1) {
            data->customers[index] = old;
            return -1;
        }
        return 1;
    }

    /* 3 编辑：复用 editCustomer */
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
