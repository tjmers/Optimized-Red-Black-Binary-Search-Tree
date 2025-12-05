CXX := g++
CXX_FLAGS := -std=c++23 -Wextra -Wall -I"./include"

TEST_DIR := test

# Since all test cases are single files with no uncompiled dependencies, do not create a build/bin dir with object files
all: red_black_test

%: $(TEST_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) $< -o $@

.PHONY: all