# Feature: Persistent Interactive CLI To-Do List

## Feature Description

Build a C++17, single-user command-line to-do application. On launch from its
executable directory, it loads `tasks.txt` from that directory and displays a
numbered main menu. Guided prompts
allow the user to create, list, edit, change the status of, and delete tasks.
Each task has a title, a status (`incomplete`, `in progress`, or `complete`),
an optional due date, and a priority. Saving occurs after every successful
mutating operation so data survives normal application exits and relaunches.

## User Story

As a single local user
I want to manage tasks through a guided numbered terminal menu
So that I can keep an organized, persistent to-do list without a graphical
application or online account.

## Problem Statement

The starter program has no task-management behavior and does not preserve
information across executions. A user therefore has no simple way to record,
review, update, or complete personal tasks in this project.

## Solution Statement

Use a small domain model separated from menu/input and file-storage concerns:

```text
main menu / guided prompts
            |
            v
task service operations (validate, identify, sort/display)
            |
            v
Task objects <----> local text-file repository
```

The application will use only the C++ standard library. The repository stores
one versioned, escaped record per line in `tasks.txt`, preventing a task title
from corrupting adjacent records. A malformed record is reported and skipped;
valid records remain available.

## Objective

- Provide a usable interactive task manager for one local user.
- Preserve data between executions in a local text file.
- Keep the first version deliberately small, portable, and dependency-free.

Success means a user can complete the core task lifecycle from the terminal,
restart the program, and observe the same saved tasks.

## Tech Stack

- C++17
- C++ standard library (`iostream`, `fstream`, `sstream`, `vector`, `algorithm`)
- `g++` compiler, invoked by the existing Bash test script
- No third-party dependency or UI/E2E browser test: this is a terminal UI.

## Commands

Commands expected after implementation:

```bash
# Build the application with warnings treated as errors.
g++ -std=c++17 -Wall -Wextra -Werror main.cpp task.cpp task_repository.cpp input.cpp -o todo

# Run it from the executable's directory; `tasks.txt` is read/written there.
./todo

# Build and run the automated unit/integration tests.
./test_runner.sh
```

## Project Structure

```text
main.cpp                    # Program entry point and main-menu loop
task.h / task.cpp           # Task data model, enums, validation, formatting
task_repository.h /.cpp     # Load/save the versioned local text-file format
input.h / input.cpp         # Reusable prompt and strict input-parsing helpers
tests/todo_list_tests.cpp   # Automated model, repository, and flow tests
test_runner.sh              # Builds production sources plus test executable
tasks.txt                   # Runtime data beside `./todo`; ignored by Git, created on first save
specs/                      # Feature specifications and plans
```

## Data and Interaction Rules

- Every task receives a positive, stable numeric ID when created. Prompts use
  that ID for edit, status change, and deletion instead of a fragile list index.
- `title` is required after trimming whitespace; its maximum length is 200
  characters to keep storage and terminal output manageable.
- `status` is one of `incomplete`, `in progress`, or `complete`; new tasks
  start as `incomplete`.
- `priority` is `low`, `medium`, or `high`; new tasks default to `medium`.
- `dueDate` is optional. When supplied it must use ISO `YYYY-MM-DD` format and
  be a real calendar date. Listing displays `No due date` when absent.
- The main menu offers: add task, list tasks, edit task, change status, delete
  task, and exit. `list` displays ID, title, status, due date, and priority.
- Invalid menu selections or field values show a clear message and re-prompt;
  they never terminate the program or write invalid data.
- A delete action requires an explicit `y/n` confirmation.
- The documented launch command is `./todo` from the executable's directory;
  that directory is the fixed location of its `tasks.txt` file.
- A save failure is shown to the user and rolls the attempted mutation back, so
  the in-memory list continues to match the last successfully persisted state.

## Code Style

- Use `PascalCase` for types/enums, `camelCase` for functions and variables,
  and `UPPER_SNAKE_CASE` only for constants.
- Keep input, persistence, and task-domain logic out of `main.cpp` so they can
  be tested without interactive `std::cin`/`std::cout`.
- Return explicit success/failure results and error text rather than calling
  `exit()` within a helper.

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

### Unit Tests

- Validate accepted and rejected task titles, status values, priority values,
  and ISO dates (including leap years).
- Test ID allocation and editing only the requested fields.
- Test serializer/parser round trips for all fields, absent due dates, and
  titles containing delimiter/escape characters.
- Test missing storage files, empty files, malformed lines, and failed writes
  through an injectable file path or repository error path.
- Test status changes and deletion by existing and nonexistent IDs, including
  rollback when persistence fails.

### Terminal Flow Tests

Create a shell-driven terminal integration test that copies `./todo` into a
temporary directory, then pipes a minimal sequence of menu choices into it.
It must prove that add,
list, update-status, delete confirmation, exit, and reload are wired to the
real menu and `tasks.txt`. Capture output and assert key prompts/data with
`grep`; remove only the explicitly created temporary directory afterward.

### Edge Cases

- Empty or whitespace-only title; title longer than 200 characters.
- Non-numeric/out-of-range menu selections and task IDs.
- Unexpected end-of-input (for example, Ctrl-D).
- Invalid dates: bad format, month/day bounds, and February 29 in a non-leap year.
- Optional due date skipped during creation and cleared during editing.
- Missing, empty, corrupt, or partially corrupt `tasks.txt`.
- Duplicate-looking titles; operations must still select by unique ID.
- Declined delete confirmation; task must remain unchanged.

## Boundaries

- **Always:** validate all interactive/file input, save after successful
  mutations, keep the program warning-clean, and run the specified tests.
- **Ask first:** change the storage format after it is introduced, add an
  external library, alter build tooling, or add features such as sorting/filtering.
- **Never:** commit a user's `tasks.txt`, silently discard valid saved tasks,
  add accounts/network calls, or expand this plan into a GUI/reminder system.

## Relevant Files

- `README.md` — documents the current C++ container workflow; update only if
  the implementation changes the run/test instructions.
- `main.cpp` — currently the empty entry point; becomes the menu-loop host.
- `test_runner.sh` — current compile/run test entry point; update to build
  separated source and test files without name collisions.
- `tests/README.md` — test-folder convention; retain and add test sources there.
- `specs/README.md` — specification-folder convention.

### New Files

- `task.h`, `task.cpp`
- `task_repository.h`, `task_repository.cpp`
- `input.h`, `input.cpp`
- `tests/todo_list_tests.cpp`
- `tests/cli_todo_list_e2e.sh`

## Implementation Plan

### Phase 1: Foundation

Define a testable task model, validation rules, and a versioned text-record
format before any interactive flow. This gives the menu stable contracts and
makes storage behavior independently verifiable.

### Phase 2: Core Implementation

Implement load/save, then the menu actions one complete path at a time: add,
list, edit/status change, and confirmed deletion. Persist every mutation only
after it has passed validation.

### Phase 3: Integration

Wire startup loading and orderly exit into `main.cpp`; update scripts and
documentation; run automated unit and terminal-flow validation in a clean
temporary working directory.

## Step by Step Tasks

### 1. Establish the task-domain contract and its tests

- Add `Task`, `TaskStatus`, and `Priority`, plus conversion/validation helpers.
- Implement ISO-date validation and the required/optional field rules above.
- Add focused tests before or alongside implementation.
- **Acceptance:** Invalid domain values cannot produce a valid `Task`.
- **Verify:** The new domain tests compile and pass through `./test_runner.sh`.
- **Dependencies:** None.
- **Estimated scope:** Medium (3 files).

### 2. Implement the local text-file repository

- Specify a simple versioned escaped record format and implement symmetric
  serialization/deserialization in `task_repository`.
- Load a missing file as an empty list; retain valid data when corrupt lines
  occur and return a warning for skipped records.
- Save atomically when practical (write a sibling temporary file, then rename)
  so an interrupted write does not normally truncate existing tasks.
- Add repository round-trip, missing-file, and malformed-record tests.
- **Acceptance:** A task list saved to a temp path reloads with identical IDs
  and fields; a missing file does not fail startup.
- **Verify:** `./test_runner.sh` passes repository tests.
- **Dependencies:** Task 1.
- **Estimated scope:** Medium (3 files).

### Checkpoint: Persistent foundation

- [ ] Domain and repository tests pass.
- [ ] Project compiles with `-std=c++17 -Wall -Wextra -Werror`.
- [ ] The storage format and recovery behavior are documented in source.

### 3. Add reusable guided-input helpers

- Implement robust line-based prompts so mixed numeric/text input cannot leave
  unread characters in `std::cin`.
- Re-prompt for invalid menu choices, IDs, dates, statuses, priorities, and
  confirmations; handle end-of-input cleanly.
- Add tests for parsing pure functions; keep stream-loop checks in terminal
  flow tests.
- **Acceptance:** Invalid input never crashes the program or mutates tasks.
- **Verify:** Unit tests and warning-clean build pass.
- **Dependencies:** Task 1.
- **Estimated scope:** Small (2–3 files).

### 4. Build the add and list vertical slice

- Replace the empty `main.cpp` with startup load, numbered main menu, add-task
  prompts, list formatting, and exit.
- Assign monotonic IDs and save only after a successful add.
- Write the early terminal integration test to add a task, list it, exit, and
  relaunch to confirm persistence.
- **Acceptance:** A user can add all fields through prompts, see the task, and
  see it again after relaunch.
- **Verify:** `./test_runner.sh` and `bash tests/cli_todo_list_e2e.sh` pass.
- **Dependencies:** Tasks 1–3.
- **Estimated scope:** Medium (3–4 files).

### 5. Add edit and status-change flows

- Prompt for a task ID; permit editing title, priority, and due date, including
  explicitly clearing an existing due date.
- Add a dedicated status action constrained to the three agreed statuses.
- Save changes after validation and report a clear message for unknown IDs.
- Extend unit and terminal-flow tests.
- **Acceptance:** The selected task alone changes and keeps its ID; invalid IDs
  and values leave all saved data unchanged.
- **Verify:** Full test and terminal flow pass.
- **Dependencies:** Task 4.
- **Estimated scope:** Medium (2–3 files).

### 6. Add confirmed deletion and failure-safe persistence messaging

- Prompt by ID, display the task being removed, and require explicit `y` before
  deleting and saving.
- Ensure `n`, invalid confirmation input, an unknown ID, or a save failure does
  not discard the in-memory task.
- Extend tests for confirmation behavior and repository errors.
- **Acceptance:** Only explicitly confirmed deletions persist after restart.
- **Verify:** Full test and terminal flow pass.
- **Dependencies:** Tasks 2–5.
- **Estimated scope:** Small (2–3 files).

### Checkpoint: Complete feature

- [ ] Every main-menu action works in a clean working directory.
- [ ] Tasks persist across a separate program invocation.
- [ ] Invalid input and malformed storage are handled without crashes or data loss.

### 7. Finalize the project test entry point and documentation

- Update `test_runner.sh` to build all production sources and test sources with
  strict C++17 warnings, then execute both automated suites.
- Update `README.md` with build/run instructions and the local-data-file note.
- Add `tasks.txt` to `.gitignore` if not already covered.
- **Acceptance:** A new developer can build, test, and run the feature using
  the documented commands.
- **Verify:** Run every command below from a clean checkout/work directory.
- **Dependencies:** Tasks 1–6.
- **Estimated scope:** Small (3 files).

### 8. Run final validation

- Execute the exact validation commands below and resolve failures before
  requesting review.
- Review the diff to ensure no runtime task data or unrelated changes are
  included.
- **Acceptance:** All commands exit zero and the manual scenario succeeds.
- **Verify:** Validation Commands section.
- **Dependencies:** Tasks 1–7.
- **Estimated scope:** Small (no planned source changes).

## Acceptance Criteria

- [ ] The app presents a numbered main menu at startup and continues until the
  user selects exit or input ends.
- [ ] Users can add, list, edit, change status, and confirmed-delete tasks.
- [ ] Each task has a required title, an optional due date, a low/medium/high
  priority, and one of the three agreed statuses.
- [ ] A task added or changed before exit is present with the same fields after
  a later launch from the same directory.
- [ ] Invalid user input, unknown IDs, missing data files, and malformed
  storage records do not crash the application or overwrite valid tasks.
- [ ] Automated unit/repository tests and the terminal integration test pass.

## Validation Commands

Run from the repository root after implementation:

```bash
g++ -std=c++17 -Wall -Wextra -Werror main.cpp task.cpp task_repository.cpp input.cpp -o todo
./test_runner.sh
bash tests/cli_todo_list_e2e.sh
```

Manual smoke test in a disposable directory:

```bash
mkdir -p /tmp/cli-todo-smoke
cp ./todo /tmp/cli-todo-smoke/
cd /tmp/cli-todo-smoke && ./todo
```

In that session: add one high-priority task with no due date; add another with
a valid date; list tasks; edit the first; set the second to `in progress`;
decline then confirm deletion; exit and relaunch. Confirm the retained task
and its changed values remain.

## Risks and Mitigations

| Risk | Impact | Mitigation |
| --- | --- | --- |
| Delimiter/newline characters corrupt a naive text format | High | Define and test escaping plus a format version before using storage. |
| Interactive input mixes formatted extraction and lines | Medium | Use line-based input helpers exclusively. |
| Failed write creates an unsaved UI state | High | Write a temporary sibling file then rename; restore the pre-mutation list and report the error if saving fails. |
| Date rules differ by platform | Low | Implement and unit-test a simple Gregorian date validator. |

## Notes

- Priorities are `low`, `medium`, and `high`; new tasks default to `medium`.
- This scope deliberately does not define sort/filter commands. Listing can use
  creation/ID order initially; adding alternative views later is a separate
  feature.
- `tasks.txt` is local runtime state and must not be committed.
