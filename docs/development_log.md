# Development Log - Agenda Qt

This document tracks the design and implementation progress of the Agenda Qt project.  
It is used as internal documentation and as support for the final project report.

## Project goal

Agenda Qt is a desktop application written in C++ and Qt Widgets for managing personal activities such as events, deadlines, reminders and checklists.

The project focuses on object-oriented design, non-trivial polymorphism, separation between logical model and graphical interface, local persistence and a usable Qt GUI.

## Main technical choices

- Language: C++
- GUI framework: Qt Widgets
- Build system: qmake with `.pro` file
- Main persistence format: JSON
- Additional import/export format: CSV
- Architecture: separated model, persistence, commands and GUI layers
- Planned design patterns:
  - Command Pattern for undo/redo
  - Factory Pattern for activity creation from JSON/templates
  - Prototype Pattern through polymorphic cloning

## Planned concrete activity classes

The model will use an abstract base class `Activity` and four concrete subclasses:

- `EventActivity`
- `DeadlineActivity`
- `ReminderActivity`
- `ChecklistActivity`

These classes must differ in attributes and behavior, not only in name.

## Planned non-trivial polymorphism

The following polymorphic methods are planned:

- `primaryDate()`
- `isOverdue(...)`
- `summary()`
- `clone()`

Each concrete activity type will implement these methods differently according to its meaning.

Examples:

- An event is overdue/past when its end date is before the current date.
- A deadline is overdue when the deadline has passed and the activity is not completed.
- A reminder is overdue when the reminder time has passed.
- A checklist is overdue when its due date has passed and not all subtasks are completed.

## Development entries

### 2026-06-16 - Initial Qt project skeleton

Created the initial repository structure for the Qt/qmake project.

Implemented:

- `agenda_qt.pro`
- `src/main.cpp`
- `resources/resources.qrc`
- `resources/style.qss`
- basic Qt window
- basic QSS loading through Qt resources

Verified:

- qmake works on macOS using Qt 6.10.1
- project compiles successfully
- application opens correctly
- build folder is ignored by Git

Notes:

- The project must use qmake and a `.pro` file.
- CMake is intentionally not used.
- Docker is not currently installed locally, but Docker compatibility must be checked before delivery.

Estimated time spent: 1h
### 2026-06-17 - Core activity hierarchy

Implemented the first version of the logical model.

Added:

- abstract base class `Activity`
- concrete class `EventActivity`
- concrete class `DeadlineActivity`
- concrete class `ReminderActivity`
- concrete class `ChecklistActivity`
- `Priority` enum
- polymorphic methods:
  - `primaryDate()`
  - `isOverdue(...)`
  - `summary()`
  - `clone()`

Design notes:

- The model does not depend on Qt Widgets or GUI classes.
- The base class contains shared attributes such as id, title, description, category, priority, completion state, creation date and update date.
- Each concrete activity type defines specific attributes and behavior.
- `clone()` will be useful later for templates, undo/redo and safe editing.
- `isOverdue(...)` is implemented differently by each concrete class, providing an initial example of non-trivial polymorphism.

Validation:

- Created temporary test data in `main.cpp`.
- Stored different activity types inside a `std::vector<std::unique_ptr<Activity>>`.
- Displayed polymorphic summaries through the base `Activity` interface.
- Verified successful compilation with qmake and make.
- Verified that the application opens correctly on macOS.

Estimated time spent: 2h
