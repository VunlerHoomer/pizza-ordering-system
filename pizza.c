/* 成员 C：菜单和价格放在同一个文件中。 */
#include <stdio.h>
#include "project.h"

int findOptionIndex(const struct MenuOption options[], int count, int id)
{
    int i;

    if (options == NULL || count < 0 || count > MAX_OPTIONS) {
        return -1;
    }
    for (i = 0; i < count; i++) {
        if (options[i].id == id) {
            return i;             /* 调用方还要检查 active。 */
        }
    }
    return -1;
}

void showPizzaMenu(const struct ShopData *data)
{
    if (data == NULL) { return; }
    printf("Pizza menu is not available yet.\n");
}

int editPizzaMenu(struct ShopData *data)
{
    if (data == NULL) { return 0; }
    printf("Menu editing is not available yet.\n");
    return 0;
}

int editPrices(struct ShopData *data)
{
    if (data == NULL) { return 0; }
    printf("Price editing is not available yet.\n");
    return 0;
}

int calculatePrice(const struct ShopData *data, int type, int size, int topping)
{
    if (data == NULL || type <= 0 || size <= 0 || topping <= 0) {
        return -1;
    }
    /* TODO：查找三个启用选项并返回价格（分），不在此读取输入或扣款。 */
    return -1;
}
