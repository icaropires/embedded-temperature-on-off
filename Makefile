CC = gcc
CXX = g++

CFLAGS = -c -pedantic-errors -Wall -Wextra -Werror
CFLAGS += -I $(INC_DIR) -I $(BME280_LIB) -I $(DISPLAY_LIB) -I $(BCM2835_LIB)

CXXFLAGS = $(CFLAGS) -std=c++14

LDFLAGS = -lwiringPi -lbcm2835 -lpthread -lncurses
BUILD_DIR = .

INC_DIR = $(BUILD_DIR)/inc
SRC_DIR = $(BUILD_DIR)/src
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
LIBS_DIR = $(BUILD_DIR)/libs

SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRCS))

BME280_LIB = $(LIBS_DIR)/BME280_driver
DISPLAY_LIB = $(LIBS_DIR)/display
BCM2835_LIB = $(LIBS_DIR)/bcm2835

BIN = $(BIN_DIR)/bin

.PHONY: all
all: $(BIN) 

print-%  : ; @echo $* = $($*)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(OBJ_DIR)/*.o -o $@ $(LDFLAGS) 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc $(OBJ_DIR)/bme280.o $(OBJ_DIR)/display.o
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $< -o $@

$(OBJ_DIR)/bme280.o: $(BME280_LIB)/bme280.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_DIR)/display.o: $(DISPLAY_LIB)/display.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

run:
	$(BIN)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
