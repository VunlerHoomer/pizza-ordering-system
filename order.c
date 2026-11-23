/* 成员 D：下单、充值和个人订单历史。通过 data 访问共享数据，使用传入的当前顾客编号。 */
#include <limits.h>
#include <stdio.h>
#include "project.h"

#define MAX_OPTION_ID 999999
#define MIN_QUANTITY 1
#define MAX_QUANTITY 10

/* 把以“分”为单位的金额打印为“元”，例如 1050 -> 10.50 yuan。 */
static void printMoney(int cents)
{
    if (cents < 0) {
        printf("-%d.%02d yuan", -(cents / 100), -(cents % 100));
    } else {
        printf("%d.%02d yuan", cents / 100, cents % 100);
    }
}

/* 逐个复制字符并保留结尾 '\0' 空间，用于保存成交时的选项名称快照。 */
static void copyText(char target[], int size, const char source[])
{
    int i = 0;

    if (target == NULL || source == NULL || size < 1) {
        return;
    }
    while (i < size - 1 && source[i] != '\0') {
        target[i] = source[i];
        i++;
    }
    target[i] = '\0';
}

/* 检查某一类菜单中是否至少有一个启用选项。 */
static int hasActiveOption(const struct MenuOption options[], int count)
{
    int i;

    if (options == NULL || count < 0 || count > MAX_OPTIONS) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (options[i].active == 1) {
            return 1;
        }
    }
    return 0;
}

/* 遍历已有订单找最大编号再加 1，保证订单编号不重复且不超过 INT_MAX。 */
static int findNextOrderId(const struct ShopData *data)
{
    int i;
    int maxId = 0;

    if (data == NULL || data->orderCount < 0 || data->orderCount > MAX_ORDERS) {
        return -1;
    }
    for (i = 0; i < data->orderCount; i++) {
        if (data->orders[i].id > maxId) {
            maxId = data->orders[i].id;
        }
    }
    if (maxId >= INT_MAX) {
        return -1;
    }
    return maxId + 1;
}

/* 读取并检查菜单选项编号（0 取消，EOF 返回 -1），返回在数组中的下标或 -1。 */
static int selectActiveOption(const char label[], const char prompt[],
                              const struct MenuOption options[], int count)
{
    int id;
    int idx;

    id = readInt(prompt, 0, MAX_OPTION_ID);
    if (id == 0) {
        printf("Order cancelled.\n");
        return -1;
    }
    if (id < 0) {
        return -1;
    }
    idx = findOptionIndex(options, count, id);
    if (idx < 0) {
        printf("%s %d does not exist.\n", label, id);
        return -1;
    }
    if (options[idx].active != 1) {
        printf("%s %d is not available.\n", label, id);
        return -1;
    }
    return idx;
}

/* 顾客下单：1 已保存成功，0 取消或检查不通过，-1 保存失败（已回滚，由调用方退出）。 */
int placeOrder(struct ShopData *data, int customerId)
{
    int customerIndex;
    int newOrderId;
    int typeIndex;
    int sizeIndex;
    int toppingIndex;
    int quantity;
    int unitPrice;
    int total;
    int oldBalance;
    int confirm;
    int orderPos;
    struct Order newOrder = {0};
    struct Order emptyOrder = {0};

    if (data == NULL || data->orderCount < 0 || data->orderCount > MAX_ORDERS) {
        return 0;
    }

    customerIndex = findCustomerIndex(data, customerId);
    if (customerId <= 0 || customerIndex < 0 ||
        data->customers[customerIndex].active != 1 ||
        data->customers[customerIndex].balance < 0) {
        printf("Customer account %d is not available for ordering.\n", customerId);
        return 0;
    }

    if (data->orderCount >= MAX_ORDERS) {
        printf("Order list is full (%d orders). Cannot place a new order.\n",
               MAX_ORDERS);
        return 0;
    }

    if (!hasActiveOption(data->types, data->typeCount) ||
        !hasActiveOption(data->sizes, data->sizeCount) ||
        !hasActiveOption(data->toppings, data->toppingCount)) {
        printf("Menu is incomplete. Cannot place an order right now.\n");
        return 0;
    }

    newOrderId = findNextOrderId(data);
    if (newOrderId < 0) {
        printf("No order number is left. Cannot place a new order.\n");
        return 0;
    }

    showPizzaMenu(data);
    printf("Current balance: ");
    printMoney(data->customers[customerIndex].balance);
    printf("\n");

    typeIndex = selectActiveOption("Base", "Select base ID (0 to cancel): ",
                                   data->types, data->typeCount);
    if (typeIndex < 0) {
        return 0;
    }
    sizeIndex = selectActiveOption("Size", "Select size ID (0 to cancel): ",
                                   data->sizes, data->sizeCount);
    if (sizeIndex < 0) {
        return 0;
    }
    toppingIndex = selectActiveOption("Topping", "Select topping ID (0 to cancel): ",
                                      data->toppings, data->toppingCount);
    if (toppingIndex < 0) {
        return 0;
    }

    quantity = readInt("Quantity (1 to 10, 0 to cancel): ", 0, MAX_QUANTITY);
    if (quantity == 0) {
        printf("Order cancelled.\n");
        return 0;
    }
    if (quantity < MIN_QUANTITY) {
        return 0;
    }

    unitPrice = calculatePrice(data, data->types[typeIndex].id,
                               data->sizes[sizeIndex].id,
                               data->toppings[toppingIndex].id);
    if (unitPrice < 0) {
        printf("Cannot calculate price for the selected options.\n");
        return 0;
    }

    /* 乘法前检查整数上限，防止总额溢出 int 范围。 */
    if (unitPrice > INT_MAX / quantity) {
        printf("Order total exceeds maximum supported amount.\n");
        return 0;
    }
    total = unitPrice * quantity;

    /* 先核对余额，余额不足直接返回，不进入确认也不扣款。 */
    oldBalance = data->customers[customerIndex].balance;
    if (oldBalance < total) {
        printf("Insufficient balance. Order total: ");
        printMoney(total);
        printf(", current balance: ");
        printMoney(oldBalance);
        printf(".\n");
        return 0;
    }

    printf("\n--- Order summary ---\n");
    printf("Base     : %s\n", data->types[typeIndex].name);
    printf("Size     : %s\n", data->sizes[sizeIndex].name);
    printf("Topping  : %s\n", data->toppings[toppingIndex].name);
    printf("Quantity : %d\n", quantity);
    printf("Unit     : ");
    printMoney(unitPrice);
    printf("\nTotal    : ");
    printMoney(total);
    printf("\nBalance  : ");
    printMoney(oldBalance);
    printf(" -> ");
    printMoney(oldBalance - total);
    printf("\n");

    confirm = readInt("Confirm order? (1 Yes, 0 Cancel): ", 0, 1);
    if (confirm == 0) {
        printf("Order cancelled.\n");
        return 0;
    }
    if (confirm != 1) {
        return 0;
    }

    newOrder.id = newOrderId;
    newOrder.customerId = customerId;
    newOrder.typeId = data->types[typeIndex].id;
    newOrder.sizeId = data->sizes[sizeIndex].id;
    newOrder.toppingId = data->toppings[toppingIndex].id;
    copyText(newOrder.typeName, (int)sizeof(newOrder.typeName),
             data->types[typeIndex].name);
    copyText(newOrder.sizeName, (int)sizeof(newOrder.sizeName),
             data->sizes[sizeIndex].name);
    copyText(newOrder.toppingName, (int)sizeof(newOrder.toppingName),
             data->toppings[toppingIndex].name);
    newOrder.quantity = quantity;
    newOrder.unitPrice = unitPrice;
    newOrder.total = total;

    orderPos = data->orderCount;
    data->orders[orderPos] = newOrder;
    data->orderCount++;
    data->customers[customerIndex].balance = oldBalance - total;

    if (saveData(data) == 1) {
        printf("Order %d placed successfully. Remaining balance: ", newOrder.id);
        printMoney(data->customers[customerIndex].balance);
        printf("\n");
        return 1;
    }

    /* 保存失败时恢复旧余额与订单数组状态，返回 -1 通知上层立即退出。 */
    data->customers[customerIndex].balance = oldBalance;
    data->orderCount = orderPos;
    data->orders[orderPos] = emptyOrder;
    printf("Save failed. Order was not placed and balance was restored.\n");
    return -1;
}

/* 查询当前顾客的历史订单，只显示成交时保存的名称、单价和总额。 */
void showOrderHistory(const struct ShopData *data, int customerId)
{
    int i;
    int matchCount = 0;
    const struct Order *order;

    if (data == NULL || data->orderCount < 0 || data->orderCount > MAX_ORDERS) {
        return;
    }
    if (customerId <= 0 || findCustomerIndex(data, customerId) < 0) {
        printf("Customer account %d does not exist.\n", customerId);
        return;
    }

    printf("\n--- Order history for customer %d ---\n", customerId);
    for (i = 0; i < data->orderCount; i++) {
        order = &data->orders[i];
        if (order->customerId != customerId) {
            continue;
        }
        matchCount++;
        printf("Order #%d: %s | %s | %s x%d | Unit: ",
               order->id, order->typeName, order->sizeName,
               order->toppingName, order->quantity);
        printMoney(order->unitPrice);
        printf(" | Total: ");
        printMoney(order->total);
        printf("\n");
    }
    if (matchCount == 0) {
        printf("  No orders yet.\n");
    } else {
        printf("Total orders shown: %d\n", matchCount);
    }
}

/* 顾客充值：输入正整数元（0 取消），检查分转换和余额累加不溢出后保存。 */
int depositMoney(struct ShopData *data, int customerId)
{
    int customerIndex;
    int oldBalance;
    int maxAddYuan;
    int yuan;
    int addCents;

    if (data == NULL) {
        return 0;
    }
    if (customerId <= 0) {
        printf("Customer account %d is not available.\n", customerId);
        return 0;
    }
    customerIndex = findCustomerIndex(data, customerId);
    if (customerIndex < 0 || data->customers[customerIndex].active != 1 ||
        data->customers[customerIndex].balance < 0) {
        printf("Customer account %d is not available for deposit.\n", customerId);
        return 0;
    }

    oldBalance = data->customers[customerIndex].balance;
    maxAddYuan = (INT_MAX - oldBalance) / 100;
    if (maxAddYuan < 1) {
        printf("Balance has reached the maximum limit. Cannot deposit more.\n");
        return 0;
    }

    printf("Current balance: ");
    printMoney(oldBalance);
    printf("\nMaximum deposit: %d yuan\n", maxAddYuan);

    yuan = readInt("Deposit amount in whole yuan (0 to cancel): ", 0, maxAddYuan);
    if (yuan == 0) {
        printf("Deposit cancelled.\n");
        return 0;
    }
    if (yuan < 0) {
        return 0;
    }

    /* 转换为分和累加余额前检查上限，确保不会发生有符号整数溢出。 */
    if (yuan > INT_MAX / 100) {
        printf("Deposit amount exceeds maximum limit.\n");
        return 0;
    }
    addCents = yuan * 100;
    if (addCents > INT_MAX - oldBalance) {
        printf("New balance exceeds maximum limit.\n");
        return 0;
    }

    data->customers[customerIndex].balance = oldBalance + addCents;
    if (saveData(data) == 1) {
        printf("Deposited ");
        printMoney(addCents);
        printf(". New balance: ");
        printMoney(data->customers[customerIndex].balance);
        printf("\n");
        return 1;
    }

    /* 保存失败时恢复原余额，返回 -1 让上层结束程序且不再覆盖备份。 */
    data->customers[customerIndex].balance = oldBalance;
    printf("Save failed. Balance was not changed.\n");
    return -1;
}
