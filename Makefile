
# Simple Makefile for JSON Expression Evaluator

CXX = g++-14
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC = src/main.cpp src/evaluator.cpp src/json.cpp
TEST_SRC = src/test.cpp src/evaluator.cpp src/json.cpp
EXEC = json_eval
TEST_EXEC = test_executable

# Default target to compile the main executable
all: compile

compile:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXEC)

test:
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_EXEC)
	@echo "Run ./$(TEST_EXEC) to execute tests."

clean:
	rm -f $(EXEC) $(TEST_EXEC)

# Phony targets
.PHONY: all compile test clean
