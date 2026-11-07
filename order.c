/* 成员 D：下单、历史和充值，都使用 A 传入的当前顾客编号。 */
#include <stdio.h>
#include "project.h"

void placeOrder(int customerId)
{
    /* TODO：选披萨、计算价格、检查余额、确认订单和扣款。 */
    printf("Ordering for account %d is not available yet.\n", customerId);
}

void showOrderHistory(int customerId)
{
    printf("Order history for account %d is not available yet.\n", customerId);
}

void depositMoney(int customerId)
{
    /* TODO：检查充值金额，再更新并保存余额。 */
    printf("Deposits for account %d are not available yet.\n", customerId);
}
