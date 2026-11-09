/* E module regression tests. Run only in a new, disposable empty folder.
 * Include file.c to inject I/O failures without adding test hooks to the app.
 */
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

static const char *fault = "none";
static FILE *faultStream = NULL;
static int transferred = 0;
static int failedTests = 0;
static int assertions = 0;

static int isFault(const char name[])
{
    return strcmp(fault, name) == 0;
}

static FILE *testOpen(const char path[], const char mode[])
{
    FILE *file;

    if ((isFault("new-open") && strcmp(path, "shop.tmp") == 0 &&
         strcmp(mode, "wb") == 0) ||
        (isFault("backup-open") && strcmp(path, "shop_backup.tmp") == 0 &&
         strcmp(mode, "wb") == 0)) {
        errno = EACCES;
        return NULL;
    }
    file = fopen(path, mode);
    if ((strncmp(fault, "new-", 4) == 0 &&
         strcmp(path, "shop.tmp") == 0 && strcmp(mode, "wb") == 0) ||
        (strncmp(fault, "backup-", 7) == 0 &&
         strcmp(path, "shop_backup.tmp") == 0 && strcmp(mode, "wb") == 0) ||
        ((isFault("read") || isFault("read-close")) &&
         strcmp(path, "shop.txt") == 0 && strcmp(mode, "rb") == 0)) {
        faultStream = file;
        transferred = 0;
    }
    return file;
}

static int testPrint(FILE *file, const char format[], ...)
{
    va_list args;
    int result;

    if (isFault("new-write") && file == faultStream && transferred++ > 0) {
        errno = ENOSPC;
        return -1;
    }
    va_start(args, format);
    result = vfprintf(file, format, args);
    va_end(args);
    return result;
}

static int testPut(int ch, FILE *file)
{
    if (isFault("backup-write") && file == faultStream && transferred++ >= 35) {
        errno = ENOSPC;
        return EOF;
    }
    return fputc(ch, file);
}

static int testGet(FILE *file)
{
    if (isFault("read") && file == faultStream && transferred++ >= 70) {
        return EOF;
    }
    return fgetc(file);
}

static int testError(FILE *file)
{
    if (isFault("read") && file == faultStream && transferred > 70) {
        return 1;
    }
    return ferror(file);
}

static int testFlush(FILE *file)
{
    if (isFault("new-flush") && file == faultStream) {
        return EOF;
    }
    return fflush(file);
}

static int testClose(FILE *file)
{
    int fail, result;

    fail = file == faultStream &&
           (isFault("new-close") || isFault("backup-close") ||
            isFault("read-close"));
    result = fclose(file);
    if (fail) {
        return EOF;
    }
    return result;
}

static int failReplacement(const char target[])
{
    return (isFault("backup-replace") &&
            strcmp(target, "shop_backup.txt") == 0) ||
           (isFault("main-replace") && strcmp(target, "shop.txt") == 0);
}

#ifdef _WIN32
static BOOL testMove(const char source[], const char target[], DWORD flags)
{
    if (failReplacement(target)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return 0;
    }
    return MoveFileExA(source, target, flags);
}
#define MoveFileExA testMove
#else
static int testRename(const char source[], const char target[])
{
    if (failReplacement(target)) {
        errno = EACCES;
        return -1;
    }
    return rename(source, target);
}
#define rename testRename
#endif

static int testRemove(const char path[])
{
    if (isFault("noop-remove") && strcmp(path, "shop.tmp") == 0) {
        errno = EACCES;
        return -1;
    }
    return remove(path);
}

#define fopen testOpen
#define fprintf testPrint
#define fputc testPut
#define fgetc testGet
#define ferror testError
#define fflush testFlush
#define fclose testClose
#define remove testRemove
#include "../file.c"
#undef fopen
#undef fprintf
#undef fputc
#undef fgetc
#undef ferror
#undef fflush
#undef fclose
#undef remove
#ifdef _WIN32
#undef MoveFileExA
#else
#undef rename
#endif

/* Independent, hand-written fixture catches field order errors. */
static const char fixture[] =
    "PIZZA_SHOP_V1\nCUSTOMERS\n2\n"
    "101\nAlice Smith\n0012345\ntest-one\n2600\n1\n"
    "202\nFormer Customer\n0098765\ntest-two\n0\n0\n"
    "TYPES\n2\n11\nNew Base\n2500\n1\n22\nRetired Base\n900\n0\n"
    "SIZES\n1\n5\n12 inch\n1200\n1\n"
    "TOPPINGS\n2\n7\nCurrent Topping\n500\n1\n9\nRetired Topping\n300\n0\n"
    "ORDERS\n2\n"
    "1\n202\n22\n5\n9\nOld Base\nOld Size\nOld Topping\n2\n3700\n7400\n"
    "2\n101\n11\n5\n7\nBase At Sale\nSize At Sale\nTopping At Sale\n3\n815\n2445\n"
    "END\n";

static void expect(int condition, const char label[])
{
    assertions++;
    if (!condition) {
        failedTests++;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void writeText(const char path[], const char text[])
{
    FILE *file = fopen(path, "wb");

    if (file == NULL || fputs(text, file) == EOF || fclose(file) == EOF) {
        fprintf(stderr, "Cannot prepare test fixture.\n");
        exit(2);
    }
}

static int textEquals(const char path[], const char expected[])
{
    FILE *file;
    int ch, same = 1;
    size_t i = 0;

    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }
    while ((ch = fgetc(file)) != EOF) {
        if (expected[i] == '\0' || ch != (unsigned char)expected[i]) {
            same = 0;
            break;
        }
        i++;
    }
    if (same && expected[i] != '\0') {
        same = 0;
    }
    if (ferror(file) || fclose(file) == EOF) {
        same = 0;
    }
    return same;
}

/* Test-only resets model a fresh program process between independent cases. */
static void resetFolder(void)
{
    fault = "none";
    faultStream = NULL;
    storageReady = 0;
    storageStopped = 0;
    remove("shop.txt");
    remove("shop.tmp");
    remove("shop_backup.txt");
    remove("shop_backup.tmp");
    writeText("shop.txt", fixture);
}

static void testRoundTrip(void)
{
    struct ShopData data = {0};
    struct ShopData loaded = {0};
    struct ShopData before;

    resetFolder();
    expect(loadData(&data) == 1, "load hand-written fixture");
    expect(data.customerCount == 2 && data.orderCount == 2, "record counts");
    expect(strcmp(data.customers[0].name, "Alice Smith") == 0 &&
           strcmp(data.customers[0].phone, "0012345") == 0, "spaces and leading zero");
    expect(data.orders[0].unitPrice == 3700 &&
           strcmp(data.orders[0].toppingName, "Old Topping") == 0,
           "historical name and price with inactive references");
    data.customers[0].balance = 4600;
    strcpy(data.customers[0].name, "A | B \"quoted\"");
    expect(saveData(&data) == 1, "save changed balance and text");
    expect(textEquals("shop_backup.txt", fixture), "backup is previous data");
    expect(loadData(&loaded) == 1, "reload changed data");
    expect(loaded.customers[0].balance == 4600 &&
           strcmp(loaded.customers[0].name, data.customers[0].name) == 0,
           "changed fields round trip");
    expect(saveData(&loaded) == 1, "unchanged exit save succeeds");
    expect(textEquals("shop_backup.txt", fixture), "unchanged save preserves useful backup");
    memcpy(&before, &loaded, sizeof(loaded));
    showStatistics(&loaded);
    expect(memcmp(&before, &loaded, sizeof(loaded)) == 0,
           "statistics preserve all business records");
}

static void testLineEndings(void)
{
    struct ShopData data = {0};
    FILE *file;
    size_t i;

    resetFolder();
    file = fopen("shop.txt", "wb");
    expect(file != NULL, "open CRLF fixture");
    if (file == NULL) {
        return;
    }
    fputs("\357\273\277", file);
    for (i = 0; fixture[i] != '\0'; i++) {
        if (fixture[i] == '\n') {
            fputc('\r', file);
        }
        fputc(fixture[i], file);
    }
    expect(fclose(file) == 0, "close CRLF fixture");
    expect(loadData(&data) == 1, "UTF-8 BOM and CRLF load");
    expect(saveData(&data) == 1, "normalize CRLF on save");
    resetFolder();
    file = fopen("shop.txt", "wb");
    expect(file != NULL, "open no-final-LF fixture");
    if (file == NULL) {
        return;
    }
    fwrite(fixture, 1, strlen(fixture) - 1, file);
    fclose(file);
    expect(loadData(&data) == 1, "final END without newline");
}

static void badText(const char text[], const char label[])
{
    struct ShopData data = {0};
    struct ShopData before;

    resetFolder();
    data.customerCount = 73;
    strcpy(data.customers[0].name, "KEEP ME");
    memcpy(&before, &data, sizeof(data));
    writeText("shop.txt", text);
    expect(loadData(&data) == 0, label);
    expect(memcmp(&data, &before, sizeof(data)) == 0,
           "failed load leaves caller byte-for-byte unchanged");
    expect(textEquals("shop.txt", text), "failed load preserves original file");
}

static void replaceFixture(const char old[], const char replacement[],
                           const char label[])
{
    char text[4096];
    const char *at = strstr(fixture, old);
    size_t prefix;

    expect(at != NULL, "fixture replacement target exists");
    if (at == NULL) {
        return;
    }
    prefix = (size_t)(at - fixture);
    memcpy(text, fixture, prefix);
    text[prefix] = '\0';
    strcat(text, replacement);
    strcat(text, at + strlen(old));
    badText(text, label);
}

static void testBadFiles(void)
{
    struct ShopData data = {0};
    FILE *file;
    char text[4096];

    badText("", "empty file");
    badText("PIZZA_SHOP_V1\nCUSTOMERS\n2\n101\n", "truncated customer");
    replaceFixture("PIZZA_SHOP_V1", "PIZZA_SHOP_V2", "unknown version");
    replaceFixture("CUSTOMERS\n2\n", "CUSTOMERS\n101\n", "customer count overflow");
    replaceFixture("TYPES\n2\n", "TYPES\n11\n", "menu count overflow");
    replaceFixture("ORDERS\n2\n", "ORDERS\n501\n", "order count overflow");
    replaceFixture("2600\n1\n", "-1\n1\n", "negative balance");
    replaceFixture("2600\n1\n", "2147483648\n1\n", "integer overflow");
    replaceFixture("2600\n1\n", "2600abc\n1\n", "numeric trailing garbage");
    replaceFixture("2600\n1\n", "2600\n2\n", "invalid active flag");
    replaceFixture("2600\n1\n", "2600\n0\n", "inactive nonzero balance");
    replaceFixture("202\nFormer Customer", "101\nFormer Customer", "duplicate customer ID");
    replaceFixture("22\nRetired Base", "11\nRetired Base", "duplicate menu ID");
    replaceFixture("2\n101\n11\n", "1\n101\n11\n", "duplicate order ID");
    replaceFixture("1\n202\n22\n", "1\n303\n22\n", "missing historical customer");
    replaceFixture("1\n202\n22\n", "1\n202\n33\n", "missing historical menu option");
    replaceFixture("3700\n7400\n", "3700\n7401\n", "inconsistent order total");
    replaceFixture("Old Topping\n2\n", "Old Topping\n11\n", "quantity outside bounds");
    replaceFixture("2500\n1\n", "1000000\n1\n", "menu price outside bounds");
    replaceFixture("Alice Smith", "", "empty customer name");
    replaceFixture("Alice Smith",
                   "12345678901234567890123456789012345678901234567890",
                   "name exceeds byte capacity");
    strcpy(text, fixture);
    strcat(text, "EXTRA\n");
    badText(text, "content after END");
    resetFolder();
    file = fopen("shop.txt", "wb");
    expect(file != NULL, "open embedded NUL fixture");
    if (file != NULL) {
        fputs("PIZZA_SHOP_V1", file);
        fputc(0, file);
        fputs(fixture + strlen("PIZZA_SHOP_V1"), file);
        fclose(file);
        expect(loadData(&data) == 0, "embedded NUL rejected");
    }
    resetFolder();
    remove("shop.txt");
    expect(loadData(&data) == 0, "missing file is not a new store");
    resetFolder();
    writeText("shop.tmp", "unfinished");
    expect(loadData(&data) == 0, "pending new data blocks startup");
    resetFolder();
    writeText("shop_backup.tmp", "unfinished");
    expect(loadData(&data) == 0, "pending backup blocks startup");
}

static void testFailure(const char point[])
{
    struct ShopData data = {0};

    resetFolder();
    writeText("shop_backup.txt", "PREVIOUS BACKUP\n");
    expect(loadData(&data) == 1, "load before injected failure");
    if (!isFault("none")) {
        return;
    }
    data.customers[0].balance = 4600;
    fault = point;
    faultStream = NULL;
    expect(saveData(&data) == 0, point);
    fault = "none";
    faultStream = NULL;
    expect(textEquals("shop.txt", fixture), "failure preserves old main file");
    if (strcmp(point, "main-replace") == 0) {
        expect(textEquals("shop_backup.txt", fixture), "new valid backup retained");
    } else {
        expect(textEquals("shop_backup.txt", "PREVIOUS BACKUP\n"),
               "failed preparation preserves previous backup");
    }
    expect(saveData(&data) == 0, "save failure cannot be retried this session");
    expect(loadData(&data) == 0, "reload cannot bypass failed-session guard");
    expect(textEquals("shop.txt", fixture), "retry does not modify main");
}

static void testFailures(void)
{
    const char *points[] = {"new-open", "new-write", "new-flush", "new-close",
                            "backup-open", "backup-write", "backup-close",
                            "backup-replace", "main-replace"};
    struct ShopData data = {0};
    unsigned int i;

    for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
        testFailure(points[i]);
    }
    resetFolder();
    expect(loadData(&data) == 1, "load before noop cleanup error");
    fault = "noop-remove";
    expect(saveData(&data) == 0, "failed noop cleanup reported");
    fault = "none";
    expect(textEquals("shop.txt", fixture), "noop cleanup leaves main intact");
    resetFolder();
    fault = "read";
    expect(loadData(&data) == 0, "read error detected");
    resetFolder();
    fault = "read-close";
    expect(loadData(&data) == 0, "read close error detected");
    resetFolder();
    expect(loadData(&data) == 1, "load before disk corruption");
    writeText("shop.txt", "CORRUPTED\n");
    writeText("shop_backup.txt", fixture);
    data.customers[0].balance++;
    expect(saveData(&data) == 0, "corrupt main is never backed up over good backup");
    expect(textEquals("shop_backup.txt", fixture), "good backup survives disk corruption");
}

static void testCapacityAndTotals(void)
{
    struct ShopData data = {0};
    struct ShopData loaded = {0};
    struct SalesTotal total = {0};
    struct SalesTotal carried = {0};
    struct Order fractional = {0};
    int i;

    resetFolder();
    expect(loadData(&data) == 1, "load before capacity test");
    data.customerCount = MAX_CUSTOMERS;
    for (i = 0; i < MAX_CUSTOMERS; i++) {
        data.customers[i] = data.customers[0];
        data.customers[i].id = i + 1;
        data.customers[i].balance = INT_MAX;
        data.customers[i].active = 1;
    }
    data.typeCount = MAX_OPTIONS;
    data.sizeCount = MAX_OPTIONS;
    data.toppingCount = MAX_OPTIONS;
    for (i = 0; i < MAX_OPTIONS; i++) {
        data.types[i] = data.types[0];
        data.types[i].id = i + 1;
        data.types[i].price = MAX_OPTION_PRICE;
        data.types[i].active = 1;
        data.sizes[i] = data.types[i];
        data.toppings[i] = data.types[i];
    }
    data.orderCount = MAX_ORDERS;
    for (i = 0; i < MAX_ORDERS; i++) {
        data.orders[i] = data.orders[0];
        data.orders[i].id = i + 1;
        data.orders[i].customerId = 1;
        data.orders[i].typeId = 1;
        data.orders[i].sizeId = 1;
        data.orders[i].toppingId = 1;
        data.orders[i].quantity = 10;
        data.orders[i].unitPrice = MAX_UNIT_PRICE;
        data.orders[i].total = MAX_UNIT_PRICE * 10;
        addSale(&total, &data.orders[i]);
    }
    expect(total.orders == 500 && total.quantity == 5000 &&
           total.yuan == 149999850 && total.cents == 0,
           "maximum sales total avoids 32-bit cents overflow");
    fractional.quantity = 1;
    fractional.unitPrice = 99;
    fractional.total = 99;
    addSale(&carried, &fractional);
    addSale(&carried, &fractional);
    expect(carried.yuan == 1 && carried.cents == 98,
           "fractional cents carry into yuan");
    expect(saveData(&data) == 1, "all arrays at capacity save");
    expect(loadData(&loaded) == 1, "all arrays at capacity reload");
    expect(loaded.customerCount == 100 && loaded.orderCount == 500 &&
           loaded.toppingCount == 10 && loaded.customers[99].balance == INT_MAX,
           "capacity and INT_MAX balance preserved");
    showStatistics(&loaded);
    resetFolder();
    expect(loadData(&data) == 1, "load before invalid memory test");
    data.orderCount = MAX_ORDERS + 1;
    expect(saveData(&data) == 0, "invalid memory count cannot write");
    expect(textEquals("shop.txt", fixture), "invalid memory leaves disk intact");
    resetFolder();
    expect(loadData(&data) == 1, "load before zero orders");
    data.orderCount = 0;
    showStatistics(&data);
}

int main(int argc, char *argv[])
{
    FILE *existing;
    const char *protectedPaths[] = {"project.h", "shop.txt", "shop_backup.txt",
                                    "shop.tmp", "shop_backup.tmp"};
    unsigned int i;

    if (argc != 2 || strcmp(argv[1], "--empty-test-folder") != 0) {
        fprintf(stderr, "Run in a NEW EMPTY folder with --empty-test-folder.\n");
        return 2;
    }
    for (i = 0; i < sizeof(protectedPaths) / sizeof(protectedPaths[0]); i++) {
        existing = fopen(protectedPaths[i], "rb");
        if (existing != NULL) {
            fclose(existing);
            fprintf(stderr, "Refusing to overwrite an existing project or data file.\n");
            return 2;
        }
    }
    testRoundTrip();
    testLineEndings();
    testBadFiles();
    testFailures();
    testCapacityAndTotals();
    fault = "none";
    remove("shop.txt");
    remove("shop.tmp");
    remove("shop_backup.txt");
    remove("shop_backup.tmp");
    printf("\nE TEST RESULTS: %d assertions, %d failures\n", assertions, failedTests);
    return failedTests != 0;
}
