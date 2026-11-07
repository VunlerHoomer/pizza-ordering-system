CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard include/*.h)
TARGET := build/pizza-ordering-system

.PHONY: all run clean
all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) Makefile
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $@

run: all
	./$(TARGET)

clean:
	rm -rf build
