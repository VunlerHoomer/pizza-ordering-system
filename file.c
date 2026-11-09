/* 成员 E：读文件、写文件和统计。
 * 数据格式见 sample.txt，使用和恢复方法见 README.md。
 * 主要使用结构体、数组、循环和文件读写；少量补充函数在使用处说明。
 */
#include <errno.h>  /* 补充：区分文件不存在和没有权限。 */
#include <limits.h> /* 补充：INT_MAX 表示本机 int 的最大值。 */
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "project.h"

#define SHOP_FILE "shop.txt"
#define NEW_FILE "shop.tmp"
#define BACKUP_FILE "shop_backup.txt"
#define NEW_BACKUP "shop_backup.tmp"
#define FILE_TITLE "PIZZA_SHOP_V1"
#define MAX_ID 999999
#define MAX_PRICE 999999
#define MAX_UNIT_PRICE (3 * MAX_PRICE)

/* 1 表示已加载数据；保存出错后必须关闭程序，不能重试。 */
static int fileLoaded = 0;
static int saveFailed = 0;

struct Sales {
    int orders;
    int quantity;
    int yuan;
    int cents;
};

/* 检查字符串非空、没有控制字符，并且能放进数组。 */
static int checkText(const char text[], int capacity)
{
    int i;
    unsigned char ch;

    for (i = 0; i < capacity; i++) {
        ch = (unsigned char)text[i];
        if (ch == '\0') {
            return i > 0;
        }
        if (ch < 32 || ch == 127) {
            return 0;
        }
    }
    return 0;
}

static int dataError(const char kind[], int index)
{
    fprintf(stderr, "Invalid %s record %d. Check IDs, fields and amounts.\n",
            kind, index + 1);
    return 0;
}

static int checkOptions(const struct MenuOption options[], int count)
{
    int i, j;

    if (count < 0 || count > MAX_OPTIONS) {
        fprintf(stderr, "Invalid menu record count.\n");
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (options[i].id < 1 || options[i].id > MAX_ID ||
            !checkText(options[i].name, sizeof(options[i].name)) ||
            options[i].price < 0 || options[i].price > MAX_PRICE ||
            (options[i].active != 0 && options[i].active != 1)) {
            return dataError("menu", i);
        }
        for (j = 0; j < i; j++) {
            if (options[j].id == options[i].id) {
                return dataError("duplicate menu ID", i);
            }
        }
    }
    return 1;
}

/* 检查历史引用时也查找停用记录，旧订单按自己的成交信息校验。 */
static int checkData(const struct ShopData *data)
{
    int i, j;
    const struct Customer *customer;
    const struct Order *order;

    if (data == NULL || data->customerCount < 0 ||
        data->customerCount > MAX_CUSTOMERS || data->orderCount < 0 ||
        data->orderCount > MAX_ORDERS) {
        fprintf(stderr, "Invalid data pointer or record count.\n");
        return 0;
    }
    if (!checkOptions(data->types, data->typeCount) ||
        !checkOptions(data->sizes, data->sizeCount) ||
        !checkOptions(data->toppings, data->toppingCount)) {
        return 0;
    }
    for (i = 0; i < data->customerCount; i++) {
        customer = &data->customers[i];
        if (customer->id < 1 || customer->id > MAX_ID ||
            !checkText(customer->name, sizeof(customer->name)) ||
            !checkText(customer->phone, sizeof(customer->phone)) ||
            !checkText(customer->password, sizeof(customer->password)) ||
            customer->balance < 0 ||
            (customer->active != 0 && customer->active != 1) ||
            (!customer->active && customer->balance != 0)) {
            return dataError("customer", i);
        }
        for (j = 0; j < i; j++) {
            if (data->customers[j].id == customer->id) {
                return dataError("duplicate customer ID", i);
            }
        }
    }
    for (i = 0; i < data->orderCount; i++) {
        order = &data->orders[i];
        if (order->id < 1 ||
            findCustomerIndex(data, order->customerId) < 0 ||
            findOptionIndex(data->types, data->typeCount, order->typeId) < 0 ||
            findOptionIndex(data->sizes, data->sizeCount, order->sizeId) < 0 ||
            findOptionIndex(data->toppings, data->toppingCount,
                            order->toppingId) < 0 ||
            !checkText(order->typeName, sizeof(order->typeName)) ||
            !checkText(order->sizeName, sizeof(order->sizeName)) ||
            !checkText(order->toppingName, sizeof(order->toppingName)) ||
            order->quantity < 1 || order->quantity > 10 ||
            order->unitPrice < 0 || order->unitPrice > MAX_UNIT_PRICE) {
            return dataError("order", i);
        }
        /* 单价和数量的上限保证乘法在目标平台的 int 范围内。 */
        if (order->total != order->unitPrice * order->quantity) {
            return dataError("order total", i);
        }
        for (j = 0; j < i; j++) {
            if (data->orders[j].id == order->id) {
                return dataError("duplicate order ID", i);
            }
        }
    }
    return 1;
}

/* 逐字符读取可识别嵌入的 NUL，避免坏文件被当成较短字符串。
 * 接受 LF、CRLF 和末行无换行；超长字段或控制字符直接报错。
 */
static int readLine(FILE *file, char text[], int capacity)
{
    int ch, length = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            break;
        }
        if (ch == '\r') {
            if (fgetc(file) != '\n') {
                return 0;
            }
            break;
        }
        if (ch < 32 || ch == 127 || length >= capacity - 1) {
            return 0;
        }
        text[length++] = (char)ch;
    }
    text[length] = '\0';
    return !ferror(file) && (ch != EOF || length > 0);
}

static int readNumber(FILE *file, int min, int max, int *result)
{
    char text[32];

    if (!readLine(file, text, sizeof(text))) {
        return 0;
    }
    return parseNumber(text, min, max, result);
}

static int readTitle(FILE *file, const char expected[])
{
    char text[64];

    if (!readLine(file, text, sizeof(text))) {
        return 0;
    }
    /* 有的编辑器会在文件开头加这三个字节（UTF-8 BOM），读标题时跳过。 */
    if (strcmp(expected, FILE_TITLE) == 0 &&
        (unsigned char)text[0] == 239 &&
        (unsigned char)text[1] == 187 && (unsigned char)text[2] == 191) {
        return strcmp(text + 3, expected) == 0;
    }
    return strcmp(text, expected) == 0;
}

static int readOptions(FILE *file, const char label[],
                       struct MenuOption options[], int *count)
{
    int i;

    if (!readTitle(file, label) ||
        !readNumber(file, 0, MAX_OPTIONS, count)) {
        return 0;
    }
    for (i = 0; i < *count; i++) {
        if (!readNumber(file, 1, MAX_ID, &options[i].id) ||
            !readLine(file, options[i].name, sizeof(options[i].name)) ||
            !readNumber(file, 0, MAX_PRICE, &options[i].price) ||
            !readNumber(file, 0, 1, &options[i].active)) {
            return 0;
        }
    }
    return 1;
}

/* 按文件中的固定顺序读取：顾客、三类菜单、订单。 */
static int readData(FILE *file, struct ShopData *data)
{
    int i;
    struct Customer *customer;
    struct Order *order;

    if (!readTitle(file, FILE_TITLE) || !readTitle(file, "CUSTOMERS") ||
        !readNumber(file, 0, MAX_CUSTOMERS, &data->customerCount)) {
        return 0;
    }
    for (i = 0; i < data->customerCount; i++) {
        customer = &data->customers[i];
        if (!readNumber(file, 1, MAX_ID, &customer->id) ||
            !readLine(file, customer->name, sizeof(customer->name)) ||
            !readLine(file, customer->phone, sizeof(customer->phone)) ||
            !readLine(file, customer->password, sizeof(customer->password)) ||
            !readNumber(file, 0, INT_MAX, &customer->balance) ||
            !readNumber(file, 0, 1, &customer->active)) {
            return 0;
        }
    }
    if (!readOptions(file, "TYPES", data->types, &data->typeCount) ||
        !readOptions(file, "SIZES", data->sizes, &data->sizeCount) ||
        !readOptions(file, "TOPPINGS", data->toppings, &data->toppingCount) ||
        !readTitle(file, "ORDERS") ||
        !readNumber(file, 0, MAX_ORDERS, &data->orderCount)) {
        return 0;
    }
    for (i = 0; i < data->orderCount; i++) {
        order = &data->orders[i];
        if (!readNumber(file, 1, INT_MAX, &order->id) ||
            !readNumber(file, 1, MAX_ID, &order->customerId) ||
            !readNumber(file, 1, MAX_ID, &order->typeId) ||
            !readNumber(file, 1, MAX_ID, &order->sizeId) ||
            !readNumber(file, 1, MAX_ID, &order->toppingId) ||
            !readLine(file, order->typeName, sizeof(order->typeName)) ||
            !readLine(file, order->sizeName, sizeof(order->sizeName)) ||
            !readLine(file, order->toppingName, sizeof(order->toppingName)) ||
            !readNumber(file, 1, 10, &order->quantity) ||
            !readNumber(file, 0, MAX_UNIT_PRICE, &order->unitPrice) ||
            !readNumber(file, 0, INT_MAX, &order->total)) {
            return 0;
        }
    }
    return readTitle(file, "END") && fgetc(file) == EOF && !ferror(file);
}

/* 先读到临时变量，全部正确后才交给调用者。 */
static int readFile(const char path[], struct ShopData *data)
{
    FILE *file;
    struct ShopData loaded = {0};
    int success;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Cannot open %s for reading.\n", path);
        return 0;
    }
    success = readData(file, &loaded);
    if (success) {
        success = checkData(&loaded);
    }
    if (!success) {
        fprintf(stderr, "%s is incomplete or invalid near byte %ld.\n",
                path, ftell(file));
    }
    if (fclose(file) == EOF) {
        fprintf(stderr, "Cannot close %s after reading.\n", path);
        success = 0;
    }
    if (success) {
        *data = loaded;
    }
    return success;
}

static int writeOptions(FILE *file, const char label[],
                        const struct MenuOption options[], int count)
{
    int i;

    if (fprintf(file, "%s\n%d\n", label, count) < 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (fprintf(file, "%d\n%s\n%d\n%d\n", options[i].id,
                    options[i].name, options[i].price, options[i].active) < 0) {
            return 0;
        }
    }
    return 1;
}

/* 与读取顺序相同，每个字段写一行，名称可以包含空格。 */
static int writeData(FILE *file, const struct ShopData *data)
{
    int i;
    const struct Customer *customer;
    const struct Order *order;

    if (fprintf(file, "%s\nCUSTOMERS\n%d\n", FILE_TITLE,
                data->customerCount) < 0) {
        return 0;
    }
    for (i = 0; i < data->customerCount; i++) {
        customer = &data->customers[i];
        if (fprintf(file, "%d\n%s\n%s\n%s\n%d\n%d\n", customer->id,
                    customer->name, customer->phone, customer->password,
                    customer->balance, customer->active) < 0) {
            return 0;
        }
    }
    if (!writeOptions(file, "TYPES", data->types, data->typeCount) ||
        !writeOptions(file, "SIZES", data->sizes, data->sizeCount) ||
        !writeOptions(file, "TOPPINGS", data->toppings, data->toppingCount) ||
        fprintf(file, "ORDERS\n%d\n", data->orderCount) < 0) {
        return 0;
    }
    for (i = 0; i < data->orderCount; i++) {
        order = &data->orders[i];
        if (fprintf(file, "%d\n%d\n%d\n%d\n%d\n%s\n%s\n%s\n%d\n%d\n%d\n",
                    order->id, order->customerId, order->typeId, order->sizeId,
                    order->toppingId, order->typeName, order->sizeName,
                    order->toppingName, order->quantity, order->unitPrice,
                    order->total) < 0) {
            return 0;
        }
    }
    return fprintf(file, "END\n") >= 0;
}

static int writeFile(const char path[], const struct ShopData *data)
{
    FILE *file;
    int success;

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Cannot create %s.\n", path);
        return 0;
    }
    success = writeData(file, data);
    if (fflush(file) == EOF) {
        success = 0;
    }
    if (fclose(file) == EOF) {
        success = 0;
    }
    if (!success) {
        fprintf(stderr, "Writing or closing %s failed.\n", path);
    }
    return success;
}

static int copyFile(const char source[], const char target[])
{
    FILE *input, *output;
    int ch, success = 1;

    input = fopen(source, "rb");
    if (input == NULL) {
        return 0;
    }
    output = fopen(target, "wb");
    if (output == NULL) {
        if (fclose(input) == EOF) {
            fprintf(stderr, "Cannot close backup source.\n");
        }
        return 0;
    }
    while ((ch = fgetc(input)) != EOF) {
        if (fputc(ch, output) == EOF) {
            success = 0;
            break;
        }
    }
    if (ferror(input) || ferror(output)) {
        success = 0;
    }
    if (fclose(input) == EOF) {
        success = 0;
    }
    if (fclose(output) == EOF) {
        success = 0;
    }
    return success;
}

/* 返回 1 表示相同，0 表示不同，-1 表示读取或关闭失败。 */
static int sameFile(const char leftPath[], const char rightPath[])
{
    FILE *left, *right;
    int a, b, result = 1;

    left = fopen(leftPath, "rb");
    if (left == NULL) {
        return -1;
    }
    right = fopen(rightPath, "rb");
    if (right == NULL) {
        if (fclose(left) == EOF) {
            fprintf(stderr, "Cannot close comparison source.\n");
        }
        return -1;
    }
    do {
        a = fgetc(left);
        b = fgetc(right);
        if (a != b) {
            result = 0;
            break;
        }
    } while (a != EOF);
    if (ferror(left) || ferror(right)) {
        result = -1;
    }
    if (fclose(left) == EOF) {
        result = -1;
    }
    if (fclose(right) == EOF) {
        result = -1;
    }
    return result;
}

/* 发现上次中断遗留的文件时，先交由用户按恢复说明核查。 */
static int noTempFile(const char path[])
{
    FILE *file;

    errno = 0;
    file = fopen(path, "rb");
    if (file != NULL) {
        if (fclose(file) == EOF) {
            fprintf(stderr, "Cannot close pending file %s.\n", path);
        }
        fprintf(stderr, "Pending file %s found. Follow the recovery guide.\n",
                path);
        return 0;
    }
    if (errno != ENOENT) {
        fprintf(stderr, "Cannot check pending file %s.\n", path);
        return 0;
    }
    return 1;
}

/* 补充知识仅集中在这里：Windows 用 MoveFileExA，其他系统用 rename。
 * 两者都在新文件准备好后替换旧文件，失败时不会先删掉旧文件。
 * 不能简化成 remove(target) 再 rename，否则失败时可能丢失正式文件。
 */
static int replaceFile(const char source[], const char target[])
{
#ifdef _WIN32
    if (!MoveFileExA(source, target,
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        fprintf(stderr, "Cannot replace %s (Windows error %lu).\n",
                target, (unsigned long)GetLastError());
        return 0;
    }
#else
    if (rename(source, target) != 0) {
        fprintf(stderr, "Cannot replace %s.\n", target);
        return 0;
    }
#endif
    return 1;
}

int loadData(struct ShopData *data)
{
    fileLoaded = 0;
    if (data == NULL || saveFailed) {
        return 0;
    }
    if (!noTempFile(NEW_FILE) || !noTempFile(NEW_BACKUP) ||
        !readFile(SHOP_FILE, data)) {
        fprintf(stderr, "Loading stopped. Check shop.txt and shop_backup.txt.\n"
                "For a new test folder, copy sample.txt to shop.txt.\n");
        return 0;
    }
    fileLoaded = 1;
    return 1;
}

int saveData(const struct ShopData *data)
{
    struct ShopData checked = {0};
    int comparison;

    if (saveFailed) {
        fprintf(stderr, "Storage already failed. Close the program.\n");
        return 0;
    }
    /* 1. 检查加载状态和数据。后面任一步出错，都不能再保存。 */
    saveFailed = 1;
    if (!fileLoaded || !checkData(data)) {
        return 0;
    }
    if (!noTempFile(NEW_FILE) || !noTempFile(NEW_BACKUP)) {
        return 0;
    }
    if (!readFile(SHOP_FILE, &checked)) {
        return 0;
    }

    /* 2. 先把新数据完整写到临时文件，再读一遍确认。 */
    if (!writeFile(NEW_FILE, data) || !readFile(NEW_FILE, &checked)) {
        return 0;
    }

    /* 3. 内容没有变化时，不替换已有备份。 */
    comparison = sameFile(NEW_FILE, SHOP_FILE);
    if (comparison < 0) {
        fprintf(stderr, "Cannot compare the pending save with shop.txt.\n");
        return 0;
    }
    if (comparison == 1) {
        if (remove(NEW_FILE) != 0) {
            fprintf(stderr, "Cannot remove the unchanged pending save.\n");
            return 0;
        }
        saveFailed = 0;
        return 1;                 /* 正常退出时保留上一份有用备份。 */
    }
    /* 4. 先准备好旧数据的备份。 */
    if (!copyFile(SHOP_FILE, NEW_BACKUP) ||
        sameFile(SHOP_FILE, NEW_BACKUP) != 1 ||
        !readFile(NEW_BACKUP, &checked)) {
        fprintf(stderr, "Backup preparation failed. shop.txt was preserved.\n");
        return 0;
    }
    /* 5. 都检查成功后，才替换备份和正式文件。 */
    if (!replaceFile(NEW_BACKUP, BACKUP_FILE) ||
        !replaceFile(NEW_FILE, SHOP_FILE)) {
        fprintf(stderr, "Save stopped. Inspect the main, backup and temporary files.\n");
        return 0;
    }
    saveFailed = 0;
    return 1;
}

/* 每单先拆成元与分，再累加并进位，避免分的总和超出 32 位 int。
 * 当前 500 单、每单最多 10 个的容量下，元的总和仍在范围内。
 */
static void addSale(struct Sales *total, const struct Order *order)
{
    total->orders++;
    total->quantity += order->quantity;
    total->yuan += order->total / 100;
    total->cents += order->total % 100;
    total->yuan += total->cents / 100;
    total->cents %= 100;
}

void showStatistics(const struct ShopData *data)
{
    struct Sales overall = {0};
    struct Sales toppings[MAX_OPTIONS] = {{0}};
    int i, j, index, largest = 0, barLength;

    if (!checkData(data)) {
        fprintf(stderr, "Statistics unavailable because records are invalid.\n");
        return;
    }
    for (i = 0; i < data->orderCount; i++) {
        index = findOptionIndex(data->toppings, data->toppingCount,
                                data->orders[i].toppingId);
        addSale(&overall, &data->orders[i]);
        addSale(&toppings[index], &data->orders[i]);
        if (toppings[index].quantity > largest) {
            largest = toppings[index].quantity;
        }
    }
    printf("\n--- Sales statistics ---\n");
    printf("  Completed orders : %d\n", overall.orders);
    printf("  Pizzas sold      : %d\n", overall.quantity);
    printf("  Sales revenue    : %d.%02d yuan\n", overall.yuan, overall.cents);
    printf("  Scope            : all recorded sales\n\n");
    printf("  %-31s %6s %6s %15s\n", "Topping (current name)",
           "Orders", "Pizzas", "Revenue / yuan");
    printf("\n");
    for (i = 0; i < data->toppingCount; i++) {
        printf("  %-31s %6d %6d %12d.%02d\n", data->toppings[i].name,
               toppings[i].orders, toppings[i].quantity,
               toppings[i].yuan, toppings[i].cents);
        printf("    ");
        barLength = 0;
        if (largest > 0) {
            barLength = toppings[i].quantity * 20 / largest;
        }
        for (j = 0; j < barLength; j++) {
            putchar('#');
        }
        if (!data->toppings[i].active) {
            printf(" [inactive, historical sales included]");
        }
        putchar('\n');
    }
    if (overall.orders == 0) {
        printf("  No completed orders yet.\n");
    }
    printf("\n");
    printf("  Deposits are tracked in account balances.\n");
}
