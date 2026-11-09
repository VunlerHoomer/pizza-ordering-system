/* E: L7 structures, L8 files, L3/L5 loops and totals.
 * File format and recovery instructions: docs/e-storage-plan.md.
 */
#include <errno.h>
#include <limits.h>
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
#define FILE_HEADER "PIZZA_SHOP_V1"
#define MAX_RECORD_ID 999999
#define MAX_OPTION_PRICE 999999
#define MAX_UNIT_PRICE (3 * MAX_OPTION_PRICE)

/* 文件内 static 状态，防止调用方在保存失败后意外重试。 */
static int storageReady = 0;
static int storageStopped = 0;

struct SalesTotal {
    int orders;
    int quantity;
    int yuan;
    int cents;
};

static int validText(const char text[], int capacity)
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

static int invalidRecord(const char kind[], int index)
{
    fprintf(stderr, "Invalid %s record %d. Check IDs, fields and amounts.\n",
            kind, index + 1);
    return 0;
}

static int validOptions(const struct MenuOption options[], int count)
{
    int i, j;

    if (count < 0 || count > MAX_OPTIONS) {
        fprintf(stderr, "Invalid menu record count.\n");
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (options[i].id < 1 || options[i].id > MAX_RECORD_ID ||
            !validText(options[i].name, sizeof(options[i].name)) ||
            options[i].price < 0 || options[i].price > MAX_OPTION_PRICE ||
            (options[i].active != 0 && options[i].active != 1)) {
            return invalidRecord("menu", i);
        }
        for (j = 0; j < i; j++) {
            if (options[j].id == options[i].id) {
                return invalidRecord("duplicate menu ID", i);
            }
        }
    }
    return 1;
}

/* 检查历史引用时也查找停用记录，旧订单按自己的成交信息校验。 */
static int validData(const struct ShopData *data)
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
    if (!validOptions(data->types, data->typeCount) ||
        !validOptions(data->sizes, data->sizeCount) ||
        !validOptions(data->toppings, data->toppingCount)) {
        return 0;
    }
    for (i = 0; i < data->customerCount; i++) {
        customer = &data->customers[i];
        if (customer->id < 1 || customer->id > MAX_RECORD_ID ||
            !validText(customer->name, sizeof(customer->name)) ||
            !validText(customer->phone, sizeof(customer->phone)) ||
            !validText(customer->password, sizeof(customer->password)) ||
            customer->balance < 0 ||
            (customer->active != 0 && customer->active != 1) ||
            (!customer->active && customer->balance != 0)) {
            return invalidRecord("customer", i);
        }
        for (j = 0; j < i; j++) {
            if (data->customers[j].id == customer->id) {
                return invalidRecord("duplicate customer ID", i);
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
            !validText(order->typeName, sizeof(order->typeName)) ||
            !validText(order->sizeName, sizeof(order->sizeName)) ||
            !validText(order->toppingName, sizeof(order->toppingName)) ||
            order->quantity < 1 || order->quantity > 10 ||
            order->unitPrice < 0 || order->unitPrice > MAX_UNIT_PRICE) {
            return invalidRecord("order", i);
        }
        /* 单价和数量的上限保证乘法在目标平台的 int 范围内。 */
        if (order->total != order->unitPrice * order->quantity) {
            return invalidRecord("order total", i);
        }
        for (j = 0; j < i; j++) {
            if (data->orders[j].id == order->id) {
                return invalidRecord("duplicate order ID", i);
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

static int readLabel(FILE *file, const char expected[])
{
    char text[64];

    if (!readLine(file, text, sizeof(text))) {
        return 0;
    }
    /* 兼容 Windows 文本编辑器可能写出的 UTF-8 BOM。 */
    if (strcmp(expected, FILE_HEADER) == 0 &&
        strncmp(text, "\357\273\277", 3) == 0) {
        return strcmp(text + 3, expected) == 0;
    }
    return strcmp(text, expected) == 0;
}

static int readOptions(FILE *file, const char label[],
                       struct MenuOption options[], int *count)
{
    int i;

    if (!readLabel(file, label) ||
        !readNumber(file, 0, MAX_OPTIONS, count)) {
        return 0;
    }
    for (i = 0; i < *count; i++) {
        if (!readNumber(file, 1, MAX_RECORD_ID, &options[i].id) ||
            !readLine(file, options[i].name, sizeof(options[i].name)) ||
            !readNumber(file, 0, MAX_OPTION_PRICE, &options[i].price) ||
            !readNumber(file, 0, 1, &options[i].active)) {
            return 0;
        }
    }
    return 1;
}

static int readRecords(FILE *file, struct ShopData *data)
{
    int i;
    struct Customer *customer;
    struct Order *order;

    if (!readLabel(file, FILE_HEADER) || !readLabel(file, "CUSTOMERS") ||
        !readNumber(file, 0, MAX_CUSTOMERS, &data->customerCount)) {
        return 0;
    }
    for (i = 0; i < data->customerCount; i++) {
        customer = &data->customers[i];
        if (!readNumber(file, 1, MAX_RECORD_ID, &customer->id) ||
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
        !readLabel(file, "ORDERS") ||
        !readNumber(file, 0, MAX_ORDERS, &data->orderCount)) {
        return 0;
    }
    for (i = 0; i < data->orderCount; i++) {
        order = &data->orders[i];
        if (!readNumber(file, 1, INT_MAX, &order->id) ||
            !readNumber(file, 1, MAX_RECORD_ID, &order->customerId) ||
            !readNumber(file, 1, MAX_RECORD_ID, &order->typeId) ||
            !readNumber(file, 1, MAX_RECORD_ID, &order->sizeId) ||
            !readNumber(file, 1, MAX_RECORD_ID, &order->toppingId) ||
            !readLine(file, order->typeName, sizeof(order->typeName)) ||
            !readLine(file, order->sizeName, sizeof(order->sizeName)) ||
            !readLine(file, order->toppingName, sizeof(order->toppingName)) ||
            !readNumber(file, 1, 10, &order->quantity) ||
            !readNumber(file, 0, MAX_UNIT_PRICE, &order->unitPrice) ||
            !readNumber(file, 0, INT_MAX, &order->total)) {
            return 0;
        }
    }
    return readLabel(file, "END") && fgetc(file) == EOF && !ferror(file);
}

static int readShopFile(const char path[], struct ShopData *data)
{
    FILE *file;
    struct ShopData loaded = {0};
    int success;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Cannot open %s for reading.\n", path);
        return 0;
    }
    success = readRecords(file, &loaded);
    if (success) {
        success = validData(&loaded);
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

static int writeRecords(FILE *file, const struct ShopData *data)
{
    int i;
    const struct Customer *customer;
    const struct Order *order;

    if (fprintf(file, "%s\nCUSTOMERS\n%d\n", FILE_HEADER,
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

static int writeShopFile(const char path[], const struct ShopData *data)
{
    FILE *file;
    int success;

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Cannot create %s.\n", path);
        return 0;
    }
    success = writeRecords(file, data);
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
static int temporaryAbsent(const char path[])
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

/* Windows 的 C 运行库 rename 不能覆盖已有目标，用小段平台适配处理。 */
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
    storageReady = 0;
    if (data == NULL || storageStopped) {
        return 0;
    }
    if (!temporaryAbsent(NEW_FILE) || !temporaryAbsent(NEW_BACKUP) ||
        !readShopFile(SHOP_FILE, data)) {
        fprintf(stderr, "Loading stopped. Check shop.txt and shop_backup.txt.\n"
                "For a new test folder, copy shop.example.txt to shop.txt.\n");
        return 0;
    }
    storageReady = 1;
    return 1;
}

int saveData(const struct ShopData *data)
{
    struct ShopData checked = {0};
    int comparison;

    if (storageStopped) {
        fprintf(stderr, "Storage already failed. Close the program.\n");
        return 0;
    }
    /* 先标记失败状态，每个失败出口都会阻止本进程继续保存。 */
    storageStopped = 1;
    if (!storageReady || !validData(data) ||
        !temporaryAbsent(NEW_FILE) || !temporaryAbsent(NEW_BACKUP) ||
        !readShopFile(SHOP_FILE, &checked) ||
        !writeShopFile(NEW_FILE, data) || !readShopFile(NEW_FILE, &checked)) {
        return 0;
    }
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
        storageStopped = 0;
        return 1;                 /* 正常退出时保留上一份有用备份。 */
    }
    if (!copyFile(SHOP_FILE, NEW_BACKUP) ||
        sameFile(SHOP_FILE, NEW_BACKUP) != 1 ||
        !readShopFile(NEW_BACKUP, &checked)) {
        fprintf(stderr, "Backup preparation failed. shop.txt was preserved.\n");
        return 0;
    }
    if (!replaceFile(NEW_BACKUP, BACKUP_FILE) ||
        !replaceFile(NEW_FILE, SHOP_FILE)) {
        fprintf(stderr, "Save stopped. Inspect the main, backup and temporary files.\n");
        return 0;
    }
    storageStopped = 0;
    return 1;
}

/* 每单先拆成元与分，再累加并进位，避免分的总和超出 32 位 int。
 * 当前 500 单、每单最多 10 个的容量下，元的总和仍在范围内。
 */
static void addSale(struct SalesTotal *total, const struct Order *order)
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
    struct SalesTotal overall = {0};
    struct SalesTotal toppings[MAX_OPTIONS] = {{0}};
    int i, j, index, largest = 0, barLength;

    if (!validData(data)) {
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
    printf("\n+-----------------------------------------------------------------------+\n");
    printf("|                         STORE SALES REPORT                            |\n");
    printf("+-----------------------------------------------------------------------+\n");
    printf("  Completed orders : %d\n", overall.orders);
    printf("  Pizzas sold      : %d\n", overall.quantity);
    printf("  Sales revenue    : %d.%02d yuan\n", overall.yuan, overall.cents);
    printf("  Scope            : all recorded sales\n\n");
    printf("  %-31s %6s %6s %15s\n", "Topping (current name)",
           "Orders", "Pizzas", "Revenue / yuan");
    printf("  ---------------------------------------------------------------------\n");
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
    printf("  ---------------------------------------------------------------------\n");
    printf("  Deposits are tracked in account balances.\n");
}
