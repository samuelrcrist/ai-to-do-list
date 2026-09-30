#include "task_repository.h"

#include <filesystem>
#include <fstream>
#include <system_error>

namespace {

constexpr const char* FILE_HEADER = "TODO_LIST_V1";

std::string escapeField(const std::string& value) {
    std::string escaped;
    for (char character : value) {
        switch (character) {
            case '\\': escaped += "\\\\"; break;
            case '\t': escaped += "\\t"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            default: escaped += character; break;
        }
    }
    return escaped;
}

bool splitEscapedFields(const std::string& line, std::vector<std::string>& fields) {
    std::string current;
    bool escaping = false;
    for (char character : line) {
        if (escaping) {
            switch (character) {
                case '\\': current += '\\'; break;
                case 't': current += '\t'; break;
                case 'n': current += '\n'; break;
                case 'r': current += '\r'; break;
                default: return false;
            }
            escaping = false;
        } else if (character == '\\') {
            escaping = true;
        } else if (character == '\t') {
            fields.push_back(current);
            current.clear();
        } else {
            current += character;
        }
    }
    if (escaping) {
        return false;
    }
    fields.push_back(current);
    return true;
}

bool parsePositiveInt(const std::string& value, int& result) {
    try {
        std::size_t parsed = 0;
        const int number = std::stoi(value, &parsed);
        if (parsed != value.size() || number <= 0) {
            return false;
        }
        result = number;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parseRecord(const std::string& line, Task& task, std::string& error) {
    std::vector<std::string> fields;
    if (!splitEscapedFields(line, fields) || fields.size() != 5) {
        error = "record has an invalid field format";
        return false;
    }

    TaskStatus status;
    Priority priority;
    if (!parsePositiveInt(fields[0], task.id) || !parseTaskStatus(fields[2], status) ||
        !parsePriority(fields[3], priority)) {
        error = "record has an invalid ID, status, or priority";
        return false;
    }

    task.title = fields[1];
    task.status = status;
    task.priority = priority;
    task.dueDate = fields[4].empty() ? std::nullopt : std::optional<std::string>(fields[4]);
    return validateTask(task, error);
}

}  // namespace

TaskRepository::TaskRepository(std::string filePath) : filePath(std::move(filePath)) {}

bool TaskRepository::load(std::vector<Task>& tasks, std::vector<std::string>& warnings,
                          std::string& error) const {
    tasks.clear();
    warnings.clear();
    error.clear();

    std::error_code filesystemError;
    if (!std::filesystem::exists(filePath, filesystemError)) {
        if (filesystemError) {
            error = "Unable to check task file: " + filesystemError.message();
            return false;
        }
        return true;
    }

    std::ifstream file(filePath);
    if (!file) {
        error = "Unable to open task file for reading.";
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) {
        return true;
    }
    if (line != FILE_HEADER) {
        error = "Task file has an unsupported format.";
        return false;
    }

    int lineNumber = 1;
    while (std::getline(file, line)) {
        ++lineNumber;
        Task task;
        std::string recordError;
        if (parseRecord(line, task, recordError)) {
            tasks.push_back(task);
        } else {
            warnings.push_back("Skipped task file line " + std::to_string(lineNumber) +
                               ": " + recordError + ".");
        }
    }
    if (file.bad()) {
        error = "Unable to finish reading task file.";
        return false;
    }
    return true;
}

bool TaskRepository::save(const std::vector<Task>& tasks, std::string& error) const {
    for (const Task& task : tasks) {
        if (!validateTask(task, error)) {
            return false;
        }
    }

    const std::filesystem::path target(filePath);
    const std::filesystem::path temporary = filePath + ".tmp";
    std::ofstream file(temporary, std::ios::trunc);
    if (!file) {
        error = "Unable to open temporary task file for writing.";
        return false;
    }

    file << FILE_HEADER << '\n';
    for (const Task& task : tasks) {
        file << task.id << '\t' << escapeField(task.title) << '\t'
             << toString(task.status) << '\t' << toString(task.priority) << '\t'
             << escapeField(task.dueDate.value_or("")) << '\n';
    }
    file.flush();
    if (!file) {
        error = "Unable to write temporary task file.";
        file.close();
        std::error_code cleanupError;
        std::filesystem::remove(temporary, cleanupError);
        return false;
    }
    file.close();

    std::error_code renameError;
    std::filesystem::rename(temporary, target, renameError);
    if (renameError) {
        error = "Unable to replace task file: " + renameError.message();
        std::error_code cleanupError;
        std::filesystem::remove(temporary, cleanupError);
        return false;
    }
    error.clear();
    return true;
}
