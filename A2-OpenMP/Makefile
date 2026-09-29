CC = gcc
CPPFLAGS = -Iinc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -fopenmp -fno-omit-frame-pointer -g -march=native -ffast-math
LDFLAGS = -fopenmp
LDLIBS = -lm

TARGET = distances
SRC_DIR = src
BUILD_DIR = build
STACK_BUILD_DIR = $(BUILD_DIR)/stack-usage
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDENCIES = $(OBJECTS:.o=.d)
STACK_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(STACK_BUILD_DIR)/%.o,$(SOURCES))
STACK_REPORTS = $(STACK_OBJECTS:.o=.su)
STACK_DEPENDENCIES = $(STACK_OBJECTS:.o=.d)

.PHONY: all clean stack-usage

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

stack-usage: $(STACK_OBJECTS)
	@cat $(STACK_REPORTS)

$(STACK_BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(STACK_BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fstack-usage -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

$(STACK_BUILD_DIR): | $(BUILD_DIR)
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPENDENCIES) $(STACK_DEPENDENCIES)
