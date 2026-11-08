/* 成员 C：披萨菜单和价格。菜单改动立即保存，保存失败时恢复内存中的原数据。 */
#include <stdio.h>
#include <string.h>
#include "project.h"

/* 单项价格上限（分）：1700 表示 17.00 元，也用于限制报价的加法范围。 */
#define MAX_PRICE_CENTS 999999
/* 选项编号只用一次，停用后不回收，因此也限制在六位数字内。 */
#define MAX_OPTION_ID 999999

/* 三个菜单类别，与 showPizzaMenu 的显示顺序一致。 */
#define LIST_TYPE 1
#define LIST_SIZE 2
#define LIST_TOPPING 3

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

static const char *listTitle(int list)
{
    if (list == LIST_TYPE) { return "Pizza bases"; }
    if (list == LIST_SIZE) { return "Sizes"; }
    if (list == LIST_TOPPING) { return "Toppings"; }
    return "";
}

/* 只读地取出某一类别的数组和记录数；类别无效或计数异常时返回 NULL。 */
static const struct MenuOption *getListConst(const struct ShopData *data,
                                            int list, int *count)
{
    const struct MenuOption *options;
    int n;

    if (data == NULL || count == NULL) {
        return NULL;
    }
    if (list == LIST_TYPE) {
        options = data->types;
        n = data->typeCount;
    } else if (list == LIST_SIZE) {
        options = data->sizes;
        n = data->sizeCount;
    } else if (list == LIST_TOPPING) {
        options = data->toppings;
        n = data->toppingCount;
    } else {
        return NULL;
    }
    if (n < 0 || n > MAX_OPTIONS) {
        return NULL;              /* 计数异常时不遍历数组，避免越界。 */
    }
    *count = n;
    return options;
}

/* 可修改地取出某一类别，供店长编辑使用。 */
static struct MenuOption *getList(struct ShopData *data, int list, int *count)
{
    struct MenuOption *options;
    int n;

    if (data == NULL || count == NULL) {
        return NULL;
    }
    if (list == LIST_TYPE) {
        options = data->types;
        n = data->typeCount;
    } else if (list == LIST_SIZE) {
        options = data->sizes;
        n = data->sizeCount;
    } else if (list == LIST_TOPPING) {
        options = data->toppings;
        n = data->toppingCount;
    } else {
        return NULL;
    }
    if (n < 0 || n > MAX_OPTIONS) {
        return NULL;
    }
    *count = n;
    return options;
}

static void setListCount(struct ShopData *data, int list, int count)
{
    if (data == NULL) {
        return;
    }
    if (list == LIST_TYPE) {
        data->typeCount = count;
    } else if (list == LIST_SIZE) {
        data->sizeCount = count;
    } else if (list == LIST_TOPPING) {
        data->toppingCount = count;
    }
}

/* 逐个字符复制，为结尾的 '\0' 留位置，目标数组不会被写越界。 */
static void copyName(char target[], int size, const char source[])
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

/* 价格以分保存，显示成“元.分”，避免浮点误差。 */
static void printPrice(int price)
{
    printf("%d.%02d yuan", price / 100, price % 100);
}

static void printList(int list, const struct MenuOption options[], int count,
                      int showInactive)
{
    int i;
    int printed = 0;

    printf("\n%s:\n", listTitle(list));
    for (i = 0; i < count; i++) {
        if (!showInactive && !options[i].active) {
            continue;             /* 顾客只看得到仍在售的选项。 */
        }
        printf("  %2d. %-20s ", options[i].id, options[i].name);
        printPrice(options[i].price);
        if (!options[i].active) {
            printf("  [inactive]");
        }
        printf("\n");
        printed++;
    }
    if (printed == 0) {
        printf("  (nothing available)\n");
    }
}

void showPizzaMenu(const struct ShopData *data)
{
    const struct MenuOption *options;
    int count = 0;

    if (data == NULL) {
        return;
    }
    printf("\n=== Pizza menu ===\n");
    options = getListConst(data, LIST_TYPE, &count);
    if (options != NULL) {
        printList(LIST_TYPE, options, count, 0);
    }
    options = getListConst(data, LIST_SIZE, &count);
    if (options != NULL) {
        printList(LIST_SIZE, options, count, 0);
    }
    options = getListConst(data, LIST_TOPPING, &count);
    if (options != NULL) {
        printList(LIST_TOPPING, options, count, 0);
    }
    printf("\nUnit price = base price + size price + topping price.\n");
    printf("Choose one base, one size and one topping when placing an order.\n");
}

/* 让店长选择类别；返回 0 表示取消，-1 表示输入结束。 */
static int askList(void)
{
    int list;

    list = readInt("List (1 base, 2 size, 3 topping; 0 to cancel): ", 0, 3);
    return list;
}

static int addOption(struct ShopData *data)
{
    struct MenuOption *options;
    struct MenuOption *slot;
    int list, count = 0, i, id = 0, price;
    char name[32];

    list = askList();
    if (list <= 0) {
        return 0;
    }
    options = getList(data, list, &count);
    if (options == NULL) {
        printf("This list is not available.\n");
        return 0;
    }
    if (count >= MAX_OPTIONS) {
        printf("%s is full (%d options). Nothing was added.\n",
               listTitle(list), MAX_OPTIONS);
        return 0;
    }

    if (!readText("New option name (empty to cancel): ", name, sizeof(name))) {
        return 0;
    }
    if (name[0] == '\0') {
        printf("Cancelled.\n");
        return 0;
    }
    price = readInt("Price in cents (0 to 999999): ", 0, MAX_PRICE_CENTS);
    if (price < 0) {
        return 0;
    }

    /* 编号只增不减：已停用的编号也不会再分配，旧订单因此不会指向别的选项。 */
    for (i = 0; i < count; i++) {
        if (options[i].id > id) {
            id = options[i].id;
        }
    }
    if (id > MAX_OPTION_ID) {
        printf("No option number is left. Nothing was added.\n");
        return 0;
    }
    id++;

    slot = &options[count];
    slot->id = id;
    copyName(slot->name, (int)sizeof(slot->name), name);
    slot->price = price;
    slot->active = 1;
    setListCount(data, list, count + 1);

    if (saveData(data) == 1) {
        printf("Option %d was added and saved.\n", id);
        return 1;
    }
    /* 保存失败：撤销本次改动，内存和文件保持一致。 */
    setListCount(data, list, count);
    slot->id = 0;
    slot->name[0] = '\0';
    slot->price = 0;
    slot->active = 0;
    printf("Save failed. The option was not added.\n");
    return -1;
}

static int renameOption(struct ShopData *data)
{
    struct MenuOption *options;
    char name[32];
    char oldName[32];
    int list, count = 0, id, index;

    list = askList();
    if (list <= 0) {
        return 0;
    }
    options = getList(data, list, &count);
    if (options == NULL || count == 0) {
        printf("There is no option in this list.\n");
        return 0;
    }
    printList(list, options, count, 1);

    id = readInt("Option number (0 to cancel): ", 0, MAX_OPTION_ID);
    if (id <= 0) {
        return 0;
    }
    index = findOptionIndex(options, count, id);
    if (index < 0) {
        printf("Option %d does not exist.\n", id);
        return 0;
    }
    if (!readText("New name (empty to cancel): ", name, sizeof(name))) {
        return 0;
    }
    if (name[0] == '\0') {
        printf("Cancelled.\n");
        return 0;
    }
    if (strcmp(name, options[index].name) == 0) {
        printf("The name did not change.\n");
        return 0;
    }

    copyName(oldName, (int)sizeof(oldName), options[index].name);
    copyName(options[index].name, (int)sizeof(options[index].name), name);
    if (saveData(data) == 1) {
        printf("Option %d was renamed and saved.\n", id);
        return 1;
    }
    copyName(options[index].name, (int)sizeof(options[index].name), oldName);
    printf("Save failed. The name was not changed.\n");
    return -1;
}

static int deactivateOption(struct ShopData *data)
{
    struct MenuOption *options;
    int list, count = 0, id, index;

    list = askList();
    if (list <= 0) {
        return 0;
    }
    options = getList(data, list, &count);
    if (options == NULL || count == 0) {
        printf("There is no option in this list.\n");
        return 0;
    }
    printList(list, options, count, 1);

    id = readInt("Option number (0 to cancel): ", 0, MAX_OPTION_ID);
    if (id <= 0) {
        return 0;
    }
    index = findOptionIndex(options, count, id);
    if (index < 0) {
        printf("Option %d does not exist.\n", id);
        return 0;
    }
    if (!options[index].active) {
        printf("Option %d is already inactive.\n", id);
        return 0;
    }

    options[index].active = 0;
    if (saveData(data) == 1) {
        printf("Option %d was deactivated. Its number will not be reused.\n", id);
        return 1;
    }
    options[index].active = 1;
    printf("Save failed. The option is still active.\n");
    return -1;
}

static int changePrice(struct ShopData *data)
{
    struct MenuOption *options;
    int list, count = 0, id, index, price, oldPrice;

    list = askList();
    if (list <= 0) {
        return 0;
    }
    options = getList(data, list, &count);
    if (options == NULL || count == 0) {
        printf("There is no option in this list.\n");
        return 0;
    }
    printList(list, options, count, 1);

    id = readInt("Option number (0 to cancel): ", 0, MAX_OPTION_ID);
    if (id <= 0) {
        return 0;
    }
    index = findOptionIndex(options, count, id);
    if (index < 0) {
        printf("Option %d does not exist.\n", id);
        return 0;
    }
    printf("Current price: ");
    printPrice(options[index].price);
    printf("\n");

    /* 0 是合法价格（例如不加价），取消在输入编号时完成。 */
    price = readInt("New price in cents (0 to 999999): ", 0, MAX_PRICE_CENTS);
    if (price < 0) {
        return 0;
    }
    if (price == options[index].price) {
        printf("The price did not change.\n");
        return 0;
    }

    oldPrice = options[index].price;
    options[index].price = price;
    if (saveData(data) == 1) {
        printf("Option %d now costs ", id);
        printPrice(price);
        printf(" (saved).\n");
        return 1;
    }
    options[index].price = oldPrice;
    printf("Save failed. The price was not changed.\n");
    return -1;
}

int editPizzaMenu(struct ShopData *data)
{
    int choice, result, changed = 0;

    if (data == NULL) {
        return 0;
    }
    while (1) {
        printf("\n--- Edit pizza menu ---\n");
        printf("1. Add an option\n2. Rename an option\n3. Deactivate an option\n0. Back\n");
        choice = readInt("Choice: ", 0, 3);
        if (choice <= 0) {
            break;                /* -1 表示输入结束，0 表示返回。 */
        }
        result = 0;
        if (choice == 1) {
            result = addOption(data);
        } else if (choice == 2) {
            result = renameOption(data);
        } else {
            result = deactivateOption(data);
        }
        if (result == -1) {
            return -1;            /* 保存失败必须马上退出，不能继续写。 */
        }
        if (result == 1) {
            changed = 1;
        }
    }
    return changed;
}

int editPrices(struct ShopData *data)
{
    if (data == NULL) {
        return 0;
    }
    return changePrice(data);
}

int calculatePrice(const struct ShopData *data, int type, int size, int topping)
{
    int typeIndex, sizeIndex, toppingIndex;
    int typePrice, sizePrice, toppingPrice;
    int total;

    if (data == NULL || type <= 0 || size <= 0 || topping <= 0) {
        return -1;
    }
    /* 编号不是下标，必须先查找；停用的选项不能报价。 */
    typeIndex = findOptionIndex(data->types, data->typeCount, type);
    sizeIndex = findOptionIndex(data->sizes, data->sizeCount, size);
    toppingIndex = findOptionIndex(data->toppings, data->toppingCount, topping);
    if (typeIndex < 0 || sizeIndex < 0 || toppingIndex < 0) {
        return -1;
    }
    if (!data->types[typeIndex].active ||
        !data->sizes[sizeIndex].active ||
        !data->toppings[toppingIndex].active) {
        return -1;
    }

    typePrice = data->types[typeIndex].price;
    sizePrice = data->sizes[sizeIndex].price;
    toppingPrice = data->toppings[toppingIndex].price;
    if (typePrice < 0 || typePrice > MAX_PRICE_CENTS ||
        sizePrice < 0 || sizePrice > MAX_PRICE_CENTS ||
        toppingPrice < 0 || toppingPrice > MAX_PRICE_CENTS) {
        return -1;                /* 价格被写坏时拒绝报价，不返回错误金额。 */
    }
    /* 三项都不超过 MAX_PRICE_CENTS，相加不会超出 int 的范围。 */
    total = typePrice + sizePrice + toppingPrice;
    if (total < 0 || total > 3 * MAX_PRICE_CENTS) {
        return -1;
    }
    return total;
}
