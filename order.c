/* 成员 D：通过 data 访问同一份数据，使用 A 传入的当前顾客编号。 */
#include <stdio.h>
#include "project.h"

int placeOrder(struct ShopData *data, int customerId)
{
    if (data == NULL) { return 0; }
    /* TODO：选披萨、检查余额、确认订单、扣款和保存。 */
    printf("Ordering for account %d is not available yet.\n", customerId);
    return 0;
}

void showOrderHistory(const struct ShopData *data, int customerId)
{
    if (data == NULL) { return; }
    printf("Order history for account %d is not available yet.\n", customerId);
}

int depositMoney(struct ShopData *data, int customerId)
{
    if (data == NULL) { return 0; }
    /* TODO：更新并保存余额；保存失败恢复旧余额，返回 -1 让 A 结束程序。 */
    printf("Deposits for account %d are not available yet.\n", customerId);
    return 0;
}
