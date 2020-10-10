CXX = g++

CFLAGS = -c -pedantic-errors -Wall -Wextra -Werror
CFLAGS += -I $(INC_DIR) -I $(BME280_LIB)

CXXFLAGS = $(CFLAGS) -std=c++11 

LDFLAGS = -lwiringPi #-lbcm2835
BUILD_DIR = .

INC_DIR = $(BUILD_DIR)/inc
SRC_DIR = $(BUILD_DIR)/src
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
LIBS_DIR = $(BUILD_DIR)/libs

SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SRCS))

BME280_LIB = $(LIBS_DIR)/BME280_driver

BIN = $(BIN_DIR)/bin

.PHONY: all
all: $(BIN) 

print-%  : ; @echo $* = $($*)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(OBJ_DIR)/*.o -o $@ 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc $(OBJ_DIR)/bme280.o
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $< -o $@

$(OBJ_DIR)/bme280.o: $(BME280_LIB)/bme280.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

run:
	$(BIN)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
