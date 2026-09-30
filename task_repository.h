#ifndef TASK_REPOSITORY_H
#define TASK_REPOSITORY_H

#include "task.h"

#include <string>
#include <vector>

class TaskRepository {
public:
    explicit TaskRepository(std::string filePath);

    bool load(std::vector<Task>& tasks, std::vector<std::string>& warnings,
              std::string& error) const;
    bool save(const std::vector<Task>& tasks, std::string& error) const;

private:
    std::string filePath;
};

#endif
