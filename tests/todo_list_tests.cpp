#include "../task.h"
#include "../task_repository.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::filesystem::path makeTemporaryDirectory() {
    const auto uniquePart = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() /
                      ("ai_todo_list_tests_" + uniquePart);
    std::filesystem::create_directories(path);
    return path;
}

Task makeTask(int id, const std::string& title) {
    return Task{id, title, TaskStatus::Incomplete, std::nullopt,
                Priority::Medium};
}

void testTaskValidation() {
    std::string error;
    Task task = makeTask(1, "  Buy groceries  ");
    require(validateTask(task, error), "A normal task should be valid");

    task.title = "   ";
    require(!validateTask(task, error), "Whitespace-only titles must be rejected");

    task = makeTask(1, "Pay rent");
    task.dueDate = "2028-02-29";
    require(validateTask(task, error), "Leap-day due date should be valid");

    task.dueDate = "2027-02-29";
    require(!validateTask(task, error), "Invalid leap-day due date must be rejected");
}

void testRepositoryRoundTrip() {
    const auto directory = makeTemporaryDirectory();
    const auto dataPath = directory / "tasks.txt";
    TaskRepository repository(dataPath.string());
    std::string error;

    Task first = makeTask(1, "Plan\twrite\\test");
    first.dueDate = "2026-10-15";
    first.priority = Priority::High;
    Task second = makeTask(2, "Read book");
    second.status = TaskStatus::InProgress;

    require(repository.save({first, second}, error), "Tasks should save successfully");

    std::vector<Task> loaded;
    std::vector<std::string> warnings;
    require(repository.load(loaded, warnings, error), "Saved tasks should load successfully");
    require(warnings.empty(), "A valid task file should not produce warnings");
    require(loaded.size() == 2, "Both saved tasks should reload");
    require(loaded[0] == first && loaded[1] == second,
            "Reloaded tasks should preserve every field");

    std::filesystem::remove_all(directory);
}

void testMissingAndMalformedFiles() {
    const auto directory = makeTemporaryDirectory();
    const auto dataPath = directory / "tasks.txt";
    TaskRepository repository(dataPath.string());
    std::vector<Task> loaded;
    std::vector<std::string> warnings;
    std::string error;

    require(repository.load(loaded, warnings, error), "A missing task file should load as empty");
    require(loaded.empty(), "A missing task file should contain no tasks");

    std::ofstream file(dataPath);
    file << "TODO_LIST_V1\n";
    file << "1\tValid task\tincomplete\tmedium\t\n";
    file << "broken record\n";
    file.close();

    require(repository.load(loaded, warnings, error),
            "A file with a malformed record should still load valid records");
    require(loaded.size() == 1, "Valid records should survive malformed records");
    require(!warnings.empty(), "Malformed records should produce a warning");

    std::filesystem::remove_all(directory);
}

void testSaveFailureIsReported() {
    const auto directory = makeTemporaryDirectory();
    TaskRepository repository((directory / "missing" / "tasks.txt").string());
    std::string error;

    require(!repository.save({makeTask(1, "Cannot save")}, error),
            "Saving to a missing parent directory must fail");
    require(!error.empty(), "A save failure must provide an error message");

    std::filesystem::remove_all(directory);
}

}  // namespace

int main() {
    try {
        testTaskValidation();
        testRepositoryRoundTrip();
        testMissingAndMalformedFiles();
        testSaveFailureIsReported();
        std::cout << "All todo-list tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
}
