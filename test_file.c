/* 文件模块的简单测试，只能在新建的空文件夹运行。
 * 单独编译，不要和 main.c 一起编译。
 */
#include <stdio.h>
#include <string.h>
#include "project.h"

static int check(int correct, const char message[])
{
    if (correct) {
        printf("PASS: %s\n", message);
        return 0;
    }
    printf("FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    struct ShopData data = {0};
    struct ShopData saved = {0};
    FILE *file;
    const char *names[] = {
        "project.h", "shop.txt", "shop_backup.txt", "shop.tmp", "shop_backup.tmp"
    };
    int i, failed = 0;

    /* 不在项目目录或已有数据的目录执行，避免改动真实数据。 */
    for (i = 0; i < 5; i++) {
        file = fopen(names[i], "rb");
        if (file != NULL) {
            fclose(file);
            printf("Please run this test in a NEW EMPTY folder.\n");
            return 1;
        }
    }

    file = fopen("shop.txt", "w");
    if (file == NULL) { return 1; }
    fprintf(file, "PIZZA_SHOP_V1\nCUSTOMERS\n1\n");
    fprintf(file, "1\nTest Customer\n00123456789\ntest123\n10000\n1\n");
    fprintf(file, "TYPES\n1\n1\nThin\n2000\n1\n");
    fprintf(file, "SIZES\n1\n1\n12 inch\n500\n1\n");
    fprintf(file, "TOPPINGS\n1\n1\nBBQ Chicken\n1200\n1\n");
    fprintf(file, "ORDERS\n0\nEND\n");
    if (ferror(file)) { fclose(file); return 1; }
    if (fclose(file) != 0) { return 1; }

    if (loadData(&data) != 1) {
        printf("FAIL: cannot load test data.\n");
        return 1;
    }
    failed += check(data.customerCount == 1, "load customer count");
    failed += check(strcmp(data.customers[0].phone, "00123456789") == 0,
                    "keep leading zero in phone number");
    failed += check(calculatePrice(&data, 1, 1, 1) == 3700,
                    "C and E use the same menu data");

    /* 模拟一笔已经确认的订单，只检查保存，不代替 D 的业务实现。 */
    data.orders[0].id = 1;
    data.orders[0].customerId = 1;
    data.orders[0].typeId = 1;
    data.orders[0].sizeId = 1;
    data.orders[0].toppingId = 1;
    strcpy(data.orders[0].typeName, "Thin");
    strcpy(data.orders[0].sizeName, "12 inch");
    strcpy(data.orders[0].toppingName, "BBQ Chicken");
    data.orders[0].quantity = 2;
    data.orders[0].unitPrice = 3700;
    data.orders[0].total = 7400;
    data.orderCount = 1;
    data.customers[0].balance = 2600;
    failed += check(saveData(&data) == 1, "save balance and order");
    if (loadData(&saved) != 1) { return 1; }
    failed += check(saved.customers[0].balance == 2600 && saved.orderCount == 1,
                    "read saved balance and order");

    data.types[0].price = 2500;
    failed += check(saveData(&data) == 1, "save a new menu price");
    if (loadData(&saved) != 1) { return 1; }
    failed += check(saved.types[0].price == 2500 && saved.orders[0].total == 7400,
                    "new price does not change old order");
    failed += check(saveData(&saved) == 1, "save unchanged data");

    showStatistics(&saved);
    printf("Expected statistics: 1 order, 2 pizzas, 74.00 yuan.\n");

    /* 最后用错误总额验证拒绝保存；失败后本进程不能继续保存。 */
    saved.orders[0].total = 7401;
    failed += check(saveData(&saved) == 0, "reject an incorrect order total");
    saved.orders[0].total = 7400;
    failed += check(saveData(&saved) == 0, "do not retry after a save failure");

    printf("Test finished: %d failed checks.\n", failed);
    printf("Test data remain in this folder for inspection.\n");
    return failed != 0;
}
