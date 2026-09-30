# Spec: Persistent Interactive CLI To-Do List

## Objective

Build a C++17 terminal application for one local user to manage a personal
to-do list. The application must provide a guided numbered menu and retain
tasks after it closes.

The user can add, list, edit, change the status of, and confirmed-delete
tasks. Success is a task created or changed in one run appearing with the same
saved fields after the user exits and starts `./todo` again from the same
directory.

### Confirmed Requirements

- A task has a required title, optional due date, priority, status, and a
  stable positive numeric ID.
- Status is exactly one of `incomplete`, `in progress`, or `complete`; new
  tasks start as `incomplete`.
- Priority is exactly `low`, `medium`, or `high`; new tasks default to
  `medium`.
- A supplied due date is a real Gregorian date in `YYYY-MM-DD` form. A task
  without one displays `No due date`.
- The program uses guided prompts and a numbered main menu, not command-line
  flags or a graphical interface.
- `tasks.txt` is local runtime state. Users launch `./todo` from its
  executable directory; that working directory is the fixed location for
  `tasks.txt`.
- Every successful mutation is saved. If saving fails, the attempted mutation
  is rolled back and the program reports the error.
- Input errors and bad saved records must be handled without crashing or
  overwriting valid task data.

## Tech Stack

- C++17
- C++ standard library only
- `g++` and Bash in the repository's documented Linux/container environment
- No accounts, networking, external database, browser UI, or third-party
  package

## Commands

Run these commands from the repository root after implementation:

```bash
g++ -std=c++17 -Wall -Wextra -Werror main.cpp task.cpp task_repository.cpp input.cpp -o todo
./todo
./test_runner.sh
bash tests/cli_todo_list_e2e.sh
```

`./todo` must be launched from the directory containing the `todo` executable
so that `tasks.txt` is read and written in that same directory.

## Project Structure

```text
main.cpp                    # Entry point and menu loop
task.h / task.cpp           # Task model, enums, validation, formatting
task_repository.h /.cpp     # Versioned text-file load and save behavior
input.h / input.cpp         # Line-based guided-input parsing
tests/todo_list_tests.cpp   # Unit and persistence tests
tests/cli_todo_list_e2e.sh  # Terminal integration test
test_runner.sh              # Test build/run entry point
tasks.txt                   # Local runtime data; Git-ignored
specs/                      # Product specs and implementation plans
```

## Code Style

- Use `PascalCase` for types and enums, `camelCase` for functions and local
  variables, and `UPPER_SNAKE_CASE` only for constants.
- Keep domain, storage, and interactive-input logic separate from `main.cpp`.
- Use line-based user input and return explicit result/error values from
  helpers; do not call `exit()` inside non-entry-point code.

```cpp
enum class TaskStatus { Incomplete, InProgress, Complete };

struct Task {
    int id;
    std::string title;
    TaskStatus status;
    std::optional<std::string> dueDate;
    Priority priority;
};
```

## Testing Strategy

- Unit-test task validation, enum conversions, date validation (including leap
  years), ID behavior, and targeted updates.
- Round-trip test the text-file serializer/parser, including absent due dates
  and escaped delimiters in titles.
- Test missing, empty, malformed, and partially malformed storage files.
- Test repository/save errors and confirm an attempted mutation is rolled back.
- Run a terminal integration test in a temporary directory. It must exercise
  add, list, status change, declined deletion, confirmed deletion, exit, and
  reload against the actual executable and `tasks.txt`.

## Boundaries

- **Always:** validate interactive and persisted input; save only valid
  changes; roll back a change if saving fails; compile warning-clean; run the
  unit and terminal integration suites.
- **Ask first:** add a dependency, change the on-disk file format once shipped,
  change build tooling, or add sorting/filtering/recurrence/reminders.
- **Never:** commit `tasks.txt`, silently discard valid saved records, add
  networking/accounts, or build a GUI as part of this feature.

## Success Criteria

- The startup menu offers add, list, edit, change status, delete, and exit.
- Listing shows ID, title, status, due date (or `No due date`), and priority.
- Invalid menu entries, task IDs, fields, dates, and confirmations re-prompt
  without crashing or changing data.
- Delete requires explicit confirmation.
- Valid data persists between launches; missing data starts an empty list;
  malformed records do not destroy valid records.
- A failed save leaves the visible in-memory list identical to the last saved
  state and reports an error.
- The commands in this document complete successfully after implementation.

## Open Questions

None. The product scope and the implementation-facing storage, rollback,
priority, and launch-directory decisions are confirmed.

## Related Plan

The ordered implementation tasks, checkpoints, risks, and detailed validation
steps are in [the CLI to-do implementation plan](cli-todo-list-20260930-plan.md).
