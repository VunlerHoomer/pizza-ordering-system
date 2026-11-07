/* 成员 E：文件读写和统计仍待实现，不能用假成功让程序继续。 */
#include <stdio.h>
#include "project.h"

int loadData(struct ShopData *data)
{
    if (data == NULL) { return 0; }
    /* TODO：加载并校验 shop.txt，全部成功才返回 1。 */
    printf("File loading has not been implemented yet.\n");
    return 0;
}

int saveData(const struct ShopData *data)
{
    if (data == NULL) { return 0; }
    /* TODO：先备份再写文件。只有确实保存成功才返回 1。 */
    printf("File saving has not been implemented yet.\n");
    return 0;
}

void showStatistics(const struct ShopData *data)
{
    if (data == NULL) { return; }
    printf("Sales statistics are not available yet.\n");
}
