/* C：显示披萨菜单、改菜单、算价格。改完要保存，失败就改回去。 */
#include <stdio.h>
#include <string.h>
#include "project.h"

/* 价格都用分，比如 1700 就是 17 元。单个选项不能超过这个上限。 */
#define MAX_PRICE_CENTS 999999
/* 编号最多六位。停用的编号也留着，不分给新的选项。 */
#define MAX_OPTION_ID 999999

/* 1 饼底，2 尺寸，3 配料。 */
#define BASE_MENU 1
#define SIZE_MENU 2
#define TOPPING_MENU 3

int findOptionIndex(const struct MenuOption options[], int count, int id)
{
    int i;

    if (options == NULL || count < 0 || count > MAX_OPTIONS) {
        return -1;
    }
    for (i = 0; i < count; i++) {
        if (options[i].id == id) {
            return i;             /* 找到了位置，还要看 active 是不是 1。 */
        }
    }
    return -1;
}

static const char *menuName(int kind)
{
    if (kind == BASE_MENU) { return "Pizza bases"; }
    if (kind == SIZE_MENU) { return "Sizes"; }
    if (kind == TOPPING_MENU) { return "Toppings"; }
    return "";
}

/* 显示菜单用：根据 kind 找数组，顺便把数量带回去。const 表示不修改。 */
static const struct MenuOption *getMenu(const struct ShopData *data,
                                            int kind, int *count)
{
    const struct MenuOption *options;
    int n;

    if (data == NULL || count == NULL) {
        return NULL;
    }
    if (kind == BASE_MENU) {
        options = data->types;
        n = data->typeCount;
    } else if (kind == SIZE_MENU) {
        options = data->sizes;
        n = data->sizeCount;
    } else if (kind == TOPPING_MENU) {
        options = data->toppings;
        n = data->toppingCount;
    } else {
        return NULL;
    }
    if (n < 0 || n > MAX_OPTIONS) {
        return NULL;              /* 数量不对就停下，不能读到数组外面。 */
    }
    *count = n;
    return options;
}

/* 店长编辑用：和上面类似，不过拿到的数组可以修改。 */
static struct MenuOption *getMenuForEdit(struct ShopData *data, int kind, int *count)
{
    struct MenuOption *options;
    int n;

    if (data == NULL || count == NULL) {
        return NULL;
    }
    if (kind == BASE_MENU) {
        options = data->types;
        n = data->typeCount;
    } else if (kind == SIZE_MENU) {
        options = data->sizes;
        n = data->sizeCount;
    } else if (kind == TOPPING_MENU) {
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

static void setMenuCount(struct ShopData *data, int kind, int count)
{
    if (data == NULL) {
        return;
    }
    if (kind == BASE_MENU) {
        data->typeCount = count;
    } else if (kind == SIZE_MENU) {
        data->sizeCount = count;
    } else if (kind == TOPPING_MENU) {
        data->toppingCount = count;
    }
}

/* 逐个复制名字，留一个位置放 '\0'。 */
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

/* 除以 100 是元，余数是分，显示时分要有两位。 */
static void printPrice(int price)
{
    printf("%d.%02d yuan", price / 100, price % 100);
}

static void showMenu(int kind, const struct MenuOption options[], int count,
                      int showAll)
{
    int i;
    int shown = 0;

    printf("\n%s:\n", menuName(kind));
    for (i = 0; i < count; i++) {
        if (!showAll && !options[i].active) {
            continue;             /* 停用的选项不显示给顾客。 */
        }
        printf("  %2d. %-20s ", options[i].id, options[i].name);
        printPrice(options[i].price);
        if (!options[i].active) {
            printf("  [inactive]");
        }
        printf("\n");
        shown++;
    }
    if (shown == 0) {
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
    options = getMenu(data, BASE_MENU, &count);
    if (options != NULL) {
        showMenu(BASE_MENU, options, count, 0);
    }
    options = getMenu(data, SIZE_MENU, &count);
    if (options != NULL) {
        showMenu(SIZE_MENU, options, count, 0);
    }
    options = getMenu(data, TOPPING_MENU, &count);
    if (options != NULL) {
        showMenu(TOPPING_MENU, options, count, 0);
    }
    printf("\nUnit price = base price + size price + topping price.\n");
    printf("Choose one base, one size and one topping when placing an order.\n");
}

/* 先问要改哪一类。0 取消，-1 是输入结束。 */
static int chooseMenu(void)
{
    int kind;

    kind = readInt("List (1 base, 2 size, 3 topping; 0 to cancel): ", 0, 3);
    return kind;
}

static int addOption(struct ShopData *data)
{
    struct MenuOption *options;
    struct MenuOption *newOption;
    int kind, count = 0, i, id = 0, price;
    char name[32];

    kind = chooseMenu();
    if (kind <= 0) {
        return 0;
    }
    options = getMenuForEdit(data, kind, &count);
    if (options == NULL) {
        printf("This list is not available.\n");
        return 0;
    }
    if (count >= MAX_OPTIONS) {
        printf("%s is full (%d options). Nothing was added.\n",
               menuName(kind), MAX_OPTIONS);
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

    /* 找出已经用过的最大编号，停用的也算，免得旧订单认错选项。 */
    for (i = 0; i < count; i++) {
        if (options[i].id > id) {
            id = options[i].id;
        }
    }
    /* 已经到 999999 就不能再加 1；999998 还能加一次。 */
    if (id >= MAX_OPTION_ID) {
        printf("No option number is left. Nothing was added.\n");
        return 0;
    }
    id++;

    newOption = &options[count];
    newOption->id = id;
    copyName(newOption->name, (int)sizeof(newOption->name), name);
    newOption->price = price;
    newOption->active = 1;
    setMenuCount(data, kind, count + 1);

    if (saveData(data) == 1) {
        printf("Option %d was added and saved.\n", id);
        return 1;
    }
    /* 没存成功，把刚加的这一项撤掉，数量也改回去。 */
    setMenuCount(data, kind, count);
    newOption->id = 0;
    newOption->name[0] = '\0';
    newOption->price = 0;
    newOption->active = 0;
    printf("Save failed. The option was not added.\n");
    return -1;
}

static int changeName(struct ShopData *data)
{
    struct MenuOption *options;
    char name[32];
    char oldName[32];
    int kind, count = 0, id, pos;

    kind = chooseMenu();
    if (kind <= 0) {
        return 0;
    }
    options = getMenuForEdit(data, kind, &count);
    if (options == NULL || count == 0) {
        printf("There is no option in this list.\n");
        return 0;
    }
    showMenu(kind, options, count, 1);

    id = readInt("Option number (0 to cancel): ", 0, MAX_OPTION_ID);
    if (id <= 0) {
        return 0;
    }
    pos = findOptionIndex(options, count, id);
    if (pos < 0) {
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
    if (strcmp(name, options[pos].name) == 0) {
        printf("The name did not change.\n");
        return 0;
    }

    /* 先留住原来的名字，保存失败时要用。 */
    copyName(oldName, (int)sizeof(oldName), options[pos].name);
    copyName(options[pos].name, (int)sizeof(options[pos].name), name);
    if (saveData(data) == 1) {
        printf("Option %d was renamed and saved.\n", id);
        return 1;
    }
    copyName(options[pos].name, (int)sizeof(options[pos].name), oldName);
    printf("Save failed. The name was not changed.\n");
    return -1;
}

static int disableOption(struct ShopData *data)
{
    struct MenuOption *options;
    int kind, count = 0, id, pos;

    kind = chooseMenu();
    if (kind <= 0) {
        return 0;
    }
    options = getMenuForEdit(data, kind, &count);
    if (options == NULL || count == 0) {
        printf("There is no option in this list.\n");
        return 0;
    }
    showMenu(kind, options, count, 1);

    id = readInt("Option number (0 to cancel): ", 0, MAX_OPTION_ID);
    if (id <= 0) {
        return 0;
    }
    pos = findOptionIndex(options, count, id);
    if (pos < 0) {
        printf("Option %d does not exist.\n", id);
        return 0;
    }
    if (!options[pos].active) {
        printf("Option %d is already inactive.\n", id);
        return 0;
    }

    options[pos].active = 0;
    if (saveData(data) == 1) {
        printf("Option %d was deactivated. Its number will not be reused.\n", id);
        return 1;
    }
    options[pos].active = 1;
    printf("Save failed. The option is still active.\n");
    return -1;
}

static int changePrice(struct ShopData *data)
{
    struct MenuOption *options;
    int kind, count = 0, id, pos, price, oldPrice;

    kind = chooseMenu();
    if (kind <= 0) {
        return 0;
    }
    options = getMenuForEdit(data, kind, &count);
    if (options == NULL || count == 0) {
        printf("There is no option in this list.\n");
        return 0;
    }
    showMenu(kind, options, count, 1);

    id = readInt("Option number (0 to cancel): ", 0, MAX_OPTION_ID);
    if (id <= 0) {
        return 0;
    }
    pos = findOptionIndex(options, count, id);
    if (pos < 0) {
        printf("Option %d does not exist.\n", id);
        return 0;
    }
    printf("Current price: ");
    printPrice(options[pos].price);
    printf("\n");

    /* 价格可以是 0，比如尺寸不加价。要取消就在前面输入编号时选 0。 */
    price = readInt("New price in cents (0 to 999999): ", 0, MAX_PRICE_CENTS);
    if (price < 0) {
        return 0;
    }
    if (price == options[pos].price) {
        printf("The price did not change.\n");
        return 0;
    }

    /* 旧价格先留着，后面存不下来就还原。 */
    oldPrice = options[pos].price;
    options[pos].price = price;
    if (saveData(data) == 1) {
        printf("Option %d now costs ", id);
        printPrice(price);
        printf(" (saved).\n");
        return 1;
    }
    options[pos].price = oldPrice;
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
            result = changeName(data);
        } else {
            result = disableOption(data);
        }
        if (result == -1) {
            return -1;            /* 这次保存失败了，直接回到主菜单函数去结束程序。 */
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
    /* 编号不一定等于数组下标，先找位置，再看能不能买。 */
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
        return -1;                /* 价格不对就返回 -1，不能拿错误价格下单。 */
    }
    /* 三个价格都检查过了，在本项目的 32 位 int 范围内可以相加。 */
    total = typePrice + sizePrice + toppingPrice;
    if (total < 0 || total > 3 * MAX_PRICE_CENTS) {
        return -1;
    }
    return total;
}
