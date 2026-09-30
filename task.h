#ifndef TASK_H
#define TASK_H

#include <optional>
#include <string>

enum class TaskStatus { Incomplete, InProgress, Complete };
enum class Priority { Low, Medium, High };

struct Task {
    int id;
    std::string title;
    TaskStatus status;
    std::optional<std::string> dueDate;
    Priority priority;

    bool operator==(const Task& other) const;
};

std::string trim(const std::string& value);
std::string toString(TaskStatus status);
std::string toString(Priority priority);
bool parseTaskStatus(const std::string& value, TaskStatus& status);
bool parsePriority(const std::string& value, Priority& priority);
bool isValidDate(const std::string& value);
bool validateTask(const Task& task, std::string& error);

#endif
