# AI To-Do List

This repository is the starter codebase for an AI to-do-list project. At present, the application is a minimal C++ program: it builds and exits successfully, but it does not yet provide task management, AI features, user input, or persistent storage.

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

The project structure is in place, but the to-do-list application itself has not been implemented yet. Future work can add the task model, command-line or other interface, AI-assisted behavior, tests, and specifications.
