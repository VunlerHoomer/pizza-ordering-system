/* 成员 B：这里的函数暂时只保留接口，之后补上账户数组或文件操作。 */
#include <stdio.h>
#include "project.h"

int checkLogin(int accountId, const char password[], int role)
{
    if (accountId <= 0 || password[0] == '\0' ||
        (role != CUSTOMER_ROLE && role != MANAGER_ROLE)) {
        return 0;
    }
    /* TODO：查找账号，核对密码和真实角色。核对成功才返回 1。 */
    printf("Account login is not available yet.\n");
    return 0;
}

void registerCustomer(void)
{
    /* TODO：新增顾客，分配不重复的账号。 */
    printf("Registration is not available yet.\n");
}

void showCustomer(int customerId)
{
    printf("Information for account %d is not available yet.\n", customerId);
}

void editCustomer(int customerId)
{
    printf("Editing account %d is not available yet.\n", customerId);
}

void manageCustomer(int operation)
{
    /* TODO：1 添加顾客，2 删除顾客，3 编辑顾客。 */
    printf("Customer management option %d is not available yet.\n", operation);
}
