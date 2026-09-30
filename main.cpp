#include "task.h"
#include "task_repository.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

bool readLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, value));
}

bool parsePositiveId(const std::string& value, int& id) {
    const std::string cleaned = trim(value);
    try {
        std::size_t parsed = 0;
        const int number = std::stoi(cleaned, &parsed);
        if (parsed != cleaned.size() || number <= 0) {
            return false;
        }
        id = number;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

Task* findTask(std::vector<Task>& tasks, int id) {
    const auto task = std::find_if(tasks.begin(), tasks.end(),
                                   [id](const Task& candidate) { return candidate.id == id; });
    return task == tasks.end() ? nullptr : &*task;
}

int nextTaskId(const std::vector<Task>& tasks) {
    int largestId = 0;
    for (const Task& task : tasks) {
        largestId = std::max(largestId, task.id);
    }
    return largestId == std::numeric_limits<int>::max() ? 0 : largestId + 1;
}

bool saveChanges(TaskRepository& repository, std::vector<Task>& tasks,
                 const std::vector<Task>& previousTasks) {
    std::string error;
    if (repository.save(tasks, error)) {
        return true;
    }
    tasks = previousTasks;
    std::cout << "Could not save changes: " << error << '\n';
    return false;
}

bool promptId(int& id) {
    std::string input;
    if (!readLine("Task ID: ", input)) {
        return false;
    }
    if (!parsePositiveId(input, id)) {
        std::cout << "Please enter a positive numeric task ID.\n";
        return false;
    }
    return true;
}

bool promptPriority(Priority& priority, const std::string& prompt) {
    std::string input;
    if (!readLine(prompt, input)) {
        return false;
    }
    if (!parsePriority(trim(input), priority)) {
        std::cout << "Priority must be low, medium, or high.\n";
        return false;
    }
    return true;
}

bool promptStatus(TaskStatus& status) {
    std::string input;
    if (!readLine("Status (incomplete, in progress, complete): ", input)) {
        return false;
    }
    if (!parseTaskStatus(trim(input), status)) {
        std::cout << "Status must be incomplete, in progress, or complete.\n";
        return false;
    }
    return true;
}

bool promptDueDate(std::optional<std::string>& dueDate, const std::string& prompt,
                   bool allowUnchanged) {
    std::string input;
    if (!readLine(prompt, input)) {
        return false;
    }
    input = trim(input);
    if (allowUnchanged && input.empty()) {
        return true;
    }
    if (input == "-") {
        dueDate.reset();
        return true;
    }
    if (!isValidDate(input)) {
        std::cout << "Due date must be YYYY-MM-DD, a real date, or - to clear it.\n";
        return false;
    }
    dueDate = input;
    return true;
}

void listTasks(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        std::cout << "No tasks yet.\n";
        return;
    }
    for (const Task& task : tasks) {
        std::cout << '[' << task.id << "] " << task.title << "\n"
                  << "  Status: " << toString(task.status) << "\n"
                  << "  Due date: " << task.dueDate.value_or("No due date") << "\n"
                  << "  Priority: " << toString(task.priority) << "\n";
    }
}

void addTask(TaskRepository& repository, std::vector<Task>& tasks) {
    const int id = nextTaskId(tasks);
    if (id == 0) {
        std::cout << "Cannot create another task: task ID limit reached.\n";
        return;
    }
    std::string title;
    if (!readLine("Title: ", title)) {
        return;
    }
    title = trim(title);
    std::optional<std::string> dueDate;
    if (!promptDueDate(dueDate, "Due date (YYYY-MM-DD, or - for none): ", false)) {
        return;
    }
    Priority priority;
    if (!promptPriority(priority, "Priority (low, medium, high): ")) {
        return;
    }
    Task task{id, title, TaskStatus::Incomplete, dueDate, priority};
    std::string error;
    if (!validateTask(task, error)) {
        std::cout << error << '\n';
        return;
    }
    const std::vector<Task> previousTasks = tasks;
    tasks.push_back(task);
    if (saveChanges(repository, tasks, previousTasks)) {
        std::cout << "Task added.\n";
    }
}

void editTask(TaskRepository& repository, std::vector<Task>& tasks) {
    int id;
    if (!promptId(id)) {
        return;
    }
    Task* task = findTask(tasks, id);
    if (task == nullptr) {
        std::cout << "No task with that ID.\n";
        return;
    }
    std::string title;
    if (!readLine("New title (blank keeps current): ", title)) {
        return;
    }
    Priority priority;
    if (!promptPriority(priority, "New priority (low, medium, high): ")) {
        return;
    }
    std::optional<std::string> dueDate = task->dueDate;
    if (!promptDueDate(dueDate, "New due date (blank keeps current, - clears): ", true)) {
        return;
    }
    const std::vector<Task> previousTasks = tasks;
    if (!trim(title).empty()) {
        task->title = trim(title);
    }
    task->priority = priority;
    task->dueDate = dueDate;
    std::string error;
    if (!validateTask(*task, error)) {
        tasks = previousTasks;
        std::cout << error << '\n';
        return;
    }
    if (saveChanges(repository, tasks, previousTasks)) {
        std::cout << "Task updated.\n";
    }
}

void changeStatus(TaskRepository& repository, std::vector<Task>& tasks) {
    int id;
    if (!promptId(id)) {
        return;
    }
    Task* task = findTask(tasks, id);
    if (task == nullptr) {
        std::cout << "No task with that ID.\n";
        return;
    }
    TaskStatus status;
    if (!promptStatus(status)) {
        return;
    }
    const std::vector<Task> previousTasks = tasks;
    task->status = status;
    if (saveChanges(repository, tasks, previousTasks)) {
        std::cout << "Task status updated.\n";
    }
}

void deleteTask(TaskRepository& repository, std::vector<Task>& tasks) {
    int id;
    if (!promptId(id)) {
        return;
    }
    Task* task = findTask(tasks, id);
    if (task == nullptr) {
        std::cout << "No task with that ID.\n";
        return;
    }
    std::string confirmation;
    if (!readLine("Delete \"" + task->title + "\"? (y/n): ", confirmation)) {
        return;
    }
    if (trim(confirmation) != "y") {
        std::cout << "Deletion cancelled.\n";
        return;
    }
    const std::vector<Task> previousTasks = tasks;
    tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
                               [id](const Task& candidate) { return candidate.id == id; }),
                tasks.end());
    if (saveChanges(repository, tasks, previousTasks)) {
        std::cout << "Task deleted.\n";
    }
}

void printMenu() {
    std::cout << "\nTo-Do List\n"
              << "1. Add task\n"
              << "2. List tasks\n"
              << "3. Edit task\n"
              << "4. Change task status\n"
              << "5. Delete task\n"
              << "6. Exit\n";
}

}  // namespace

int main() {
    TaskRepository repository("tasks.txt");
    std::vector<Task> tasks;
    std::vector<std::string> warnings;
    std::string error;
    if (!repository.load(tasks, warnings, error)) {
        std::cerr << "Unable to start: " << error << '\n';
        return 1;
    }
    for (const std::string& warning : warnings) {
        std::cerr << "Warning: " << warning << '\n';
    }
    while (true) {
        printMenu();
        std::string choice;
        if (!readLine("Choose an option: ", choice)) {
            std::cout << "\nGoodbye.\n";
            return 0;
        }
        const std::string cleanedChoice = trim(choice);
        const char option = cleanedChoice.size() == 1 ? cleanedChoice[0] : '0';
        switch (option) {
            case '1': addTask(repository, tasks); break;
            case '2': listTasks(tasks); break;
            case '3': editTask(repository, tasks); break;
            case '4': changeStatus(repository, tasks); break;
            case '5': deleteTask(repository, tasks); break;
            case '6': std::cout << "Goodbye.\n"; return 0;
            default: std::cout << "Please choose a number from 1 to 6.\n"; break;
        }
    }
}
