CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
LDFLAGS =
PYTHON = python3
BUILD_DIR = build
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard include/*.h)
APP_CORE := src/app.c src/input.c
APP_UI := src/main.c src/ui.c
TEST_DOUBLE := tests/demo_modules.c
TARGET := $(BUILD_DIR)/pizza-ordering-system
DEMO := $(BUILD_DIR)/pizza-demo
CORE_TEST := $(BUILD_DIR)/test-core

.PHONY: all run demo test clean windows
all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) $(HEADERS) Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@

$(DEMO): $(APP_CORE) $(APP_UI) $(TEST_DOUBLE) $(HEADERS) tests/demo_modules.h Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -DPIZZA_DEMO $(APP_CORE) $(APP_UI) $(TEST_DOUBLE) $(LDFLAGS) -o $@

$(CORE_TEST): tests/test_core.c $(APP_CORE) $(TEST_DOUBLE) $(HEADERS) tests/demo_modules.h Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_core.c $(APP_CORE) $(TEST_DOUBLE) $(LDFLAGS) -o $@

run: $(TARGET)
	./$(TARGET)

demo: $(DEMO)
	./$(DEMO)

test: $(TARGET) $(DEMO) $(CORE_TEST)
	./$(CORE_TEST)
	$(PYTHON) tests/test_app.py --build-dir $(BUILD_DIR)

# Optional cross-compiler; the resulting executable still needs Windows runtime QA.
windows: | $(BUILD_DIR)
	x86_64-w64-mingw32-gcc $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(BUILD_DIR)/pizza-ordering-system.exe

clean:
	rm -rf $(BUILD_DIR)
