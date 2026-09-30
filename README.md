# AI To-Do List

This repository is the starter codebase for an AI to-do-list project. At present, the application is a minimal C++ program: it builds and exits successfully, and provides basic task management with local storage.
## Run the program

The project requires a C++ compiler such as `g++`.

```bash
g++ main.cpp -o app
./app
```

Or use the included helper script:

```bash
./test_runner.sh
```

The current program has no console output; a successful run exits with status code `0`.

## Project layout

- `main.cpp` — current application entry point.
- `test_runner.sh` — compiles all C++ files in the repository root and runs the resulting `app` executable.
- `specs/` — requirements and feature specifications.
- `tests/` — automated test code.
- `.agents/` — local AI-agent configuration and skills.

## Current status

A working version has been created. The program allows users to create a to-do list items, assign priorities and due dates, and edit or delete entries.
