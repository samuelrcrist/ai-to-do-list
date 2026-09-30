CXX ?= g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -MMD -MP

APP := todo
TEST_BIN := tests/todo_list_tests
APP_SOURCES := main.cpp task.cpp task_repository.cpp
TEST_SOURCES := tests/todo_list_tests.cpp task.cpp task_repository.cpp
DEPENDENCIES := $(sort $(APP_SOURCES:.cpp=.d) $(TEST_SOURCES:.cpp=.d))

.PHONY: all run test clean

all: $(APP)

$(APP): $(APP_SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@

run: $(APP)
	./$(APP)

$(TEST_BIN): $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	$(RM) $(APP) $(TEST_BIN) $(DEPENDENCIES)

-include $(DEPENDENCIES)
