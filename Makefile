CXX = g++
CXXFLAGS = -std=c++23 -Wall -Werror -g -fsanitize=address -MMD -MP
LDFLAGS = -fsanitize=address -rdynamic -lstdc++exp

SRC_DIR = src
BIN_DIR = bin

SRC = $(shell find $(SRC_DIR) -name '*.cpp')
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

TARGET = $(BIN_DIR)/main

ARGS = programm out

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEP)

run: all
	./$(TARGET) $(ARGS)

wrun: 
	> dist/logs.log
	$(MAKE) run

clean:
	rm -rf $(BIN_DIR)