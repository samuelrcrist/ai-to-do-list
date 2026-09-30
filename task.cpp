#include "task.h"

#include <algorithm>
#include <cctype>

namespace {

bool isLeapYear(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

bool isDigits(const std::string& value, std::size_t start, std::size_t length) {
    return std::all_of(value.begin() + static_cast<std::ptrdiff_t>(start),
                       value.begin() + static_cast<std::ptrdiff_t>(start + length),
                       [](unsigned char character) { return std::isdigit(character) != 0; });
}

}  // namespace

bool Task::operator==(const Task& other) const {
    return id == other.id && title == other.title && status == other.status &&
           dueDate == other.dueDate && priority == other.priority;
}

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(),
                                        [](unsigned char character) { return std::isspace(character) != 0; });
    const auto last = std::find_if_not(value.rbegin(), value.rend(),
                                       [](unsigned char character) { return std::isspace(character) != 0; }).base();
    return first < last ? std::string(first, last) : std::string();
}

std::string toString(TaskStatus status) {
    switch (status) {
        case TaskStatus::Incomplete: return "incomplete";
        case TaskStatus::InProgress: return "in progress";
        case TaskStatus::Complete: return "complete";
    }
    return "";
}

std::string toString(Priority priority) {
    switch (priority) {
        case Priority::Low: return "low";
        case Priority::Medium: return "medium";
        case Priority::High: return "high";
    }
    return "";
}

bool parseTaskStatus(const std::string& value, TaskStatus& status) {
    if (value == "incomplete") {
        status = TaskStatus::Incomplete;
    } else if (value == "in progress") {
        status = TaskStatus::InProgress;
    } else if (value == "complete") {
        status = TaskStatus::Complete;
    } else {
        return false;
    }
    return true;
}

bool parsePriority(const std::string& value, Priority& priority) {
    if (value == "low") {
        priority = Priority::Low;
    } else if (value == "medium") {
        priority = Priority::Medium;
    } else if (value == "high") {
        priority = Priority::High;
    } else {
        return false;
    }
    return true;
}

bool isValidDate(const std::string& value) {
    if (value.size() != 10 || value[4] != '-' || value[7] != '-' ||
        !isDigits(value, 0, 4) || !isDigits(value, 5, 2) || !isDigits(value, 8, 2)) {
        return false;
    }

    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));
    const int daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) {
        return false;
    }
    const int maximumDay = month == 2 && isLeapYear(year) ? 29 : daysPerMonth[month];
    return day >= 1 && day <= maximumDay;
}

bool validateTask(const Task& task, std::string& error) {
    if (task.id <= 0) {
        error = "Task ID must be positive.";
        return false;
    }
    if (trim(task.title).empty()) {
        error = "Task title is required.";
        return false;
    }
    if (task.title.size() > 200) {
        error = "Task title must not exceed 200 characters.";
        return false;
    }
    if (task.dueDate.has_value() && !isValidDate(*task.dueDate)) {
        error = "Due date must be a real date in YYYY-MM-DD format.";
        return false;
    }
    error.clear();
    return true;
}
