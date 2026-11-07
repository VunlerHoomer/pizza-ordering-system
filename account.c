/* 成员 B：查找辅助函数已提供，登录和资料管理还需要补充。 */
#include <stdio.h>
#include "project.h"

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
    if (data == NULL || password == NULL || accountId <= 0 || password[0] == '\0' ||
        (role != CUSTOMER_ROLE && role != MANAGER_ROLE)) {
        return 0;
    }
    /* TODO：查找账号，核对密码、启用状态和真实角色，成功才返回 1。 */
    printf("Account login is not available yet.\n");
    return 0;
}

int registerCustomer(struct ShopData *data)
{
    if (data == NULL) { return 0; }
    /* TODO：新增顾客，分配不重复的账号，再保存。 */
    printf("Registration is not available yet.\n");
    return 0;
}

void showCustomer(const struct ShopData *data, int customerId)
{
    if (data == NULL) { return; }
    printf("Information for account %d is not available yet.\n", customerId);
}

int editCustomer(struct ShopData *data, int customerId)
{
    if (data == NULL) { return 0; }
    printf("Editing account %d is not available yet.\n", customerId);
    return 0;
}

int manageCustomer(struct ShopData *data, int operation)
{
    if (data == NULL) { return 0; }
    /* TODO：1 添加，2 停用，3 编辑；保存失败须回退本次改动并返回 -1。 */
    printf("Customer management option %d is not available yet.\n", operation);
    return 0;
}
