/* 成员 C：菜单和价格放在同一个文件中。 */
#include <stdio.h>
#include "project.h"

void showPizzaMenu(void)
{
    printf("Pizza menu is not available yet.\n");
}

void editPizzaMenu(void)
{
    printf("Menu editing is not available yet.\n");
}

void editPrices(void)
{
    printf("Price editing is not available yet.\n");
}

int calculatePrice(int type, int size, int topping)
{
    /* TODO：根据这三个选项计算价格，返回值的单位为分。 */
    printf("Price for type %d, size %d, topping %d is not available yet.\n",
           type, size, topping);
    return -1;
}
