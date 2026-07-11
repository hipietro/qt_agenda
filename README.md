# Agenda Qt

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt 6](https://img.shields.io/badge/Qt-6-green)
![Build system](https://img.shields.io/badge/build-qmake-orange)
![Platform](https://img.shields.io/badge/platform-desktop-lightgrey)

Agenda Qt is a desktop agenda application written in **C++17** with **Qt Widgets**.
It was developed as an Object-Oriented Programming project and focuses on a clear separation between the logical model, GUI, persistence layer and undo/redo commands.

The application manages different kinds of personal activities: events, deadlines, reminders and checklists. Each activity type is represented by a concrete subclass of a common abstract base class.

---
⚠️Project status: 
Paused waiting for academic evaluation.
---

## Main features

- Create, edit, delete and complete activities
- Four concrete activity types:
  - events
  - deadlines
  - reminders
  - checklists
- Search by title, description, category and summary
- Advanced filters by type, priority, category, completion state, recurrence and overdue state
- Sorting by date, title, priority, completion state, creation time and update time
- Visual feedback for completed, overdue and high-priority activities
- Category management dialog with add, rename, remove and clear actions
- Editable category selectors in activity creation and editing dialogs
- Recurring activities with configurable frequency and end condition
- Activity templates with JSON persistence
- Undo/redo for the main activity actions
- Save/load using local JSON files
- Unsaved changes tracking and confirmation before destructive actions
- Sample agenda file for testing and demonstration

---

## Object-oriented design

The core model is based on an abstract class, `Activity`, and four concrete subclasses:

| Class | Purpose |
| --- | --- |
| `EventActivity` | Activity with start/end time, location and participants |
| `DeadlineActivity` | Activity with due date, context and hard-deadline flag |
| `ReminderActivity` | Activity with reminder time, advance notice and note |
| `ChecklistActivity` | Activity with target date and multiple checklist items |

The application uses polymorphism for behavior such as:

- `kind()`
- `primaryDate()`
- `isOverdue(...)`
- `summary()`
- `clone()`

This means the GUI and managers can work with generic `Activity` pointers while the specific behavior remains inside the concrete activity classes.

---

## Project structure

```text
agenda_qt/
├── agenda_qt.pro                 # qmake project file
├── Dockerfile                    # course Docker environment
├── examples/
│   └── sample_agenda.json        # demo agenda file
├── resources/
│   ├── resources.qrc             # Qt resource collection
│   └── style.qss                 # application stylesheet
├── src/
│   ├── commands/                 # undo/redo command system
│   ├── gui/                      # Qt Widgets interface
│   ├── model/                    # domain model and business logic
│   ├── persistence/              # JSON save/load layer
│   └── main.cpp
└── docs/
    ├── development_log.md        # implementation log
    └── extra_features.md         # design highlights and optional features
```

---

## Build requirements

Required tools:

- C++17 compiler
- Qt 6
- qmake
- make

The project intentionally uses **qmake** through `agenda_qt.pro`.
CMake is not used.

---

## Build locally

Create a separate build folder from the project root:

```bash
mkdir -p build
cd build
qmake ../agenda_qt.pro
make
```

On some Linux systems the command may be `qmake6` instead of `qmake`:

```bash
mkdir -p build
cd build
qmake6 ../agenda_qt.pro
make
```

On macOS, if Qt was installed manually, use the qmake executable from the Qt installation, for example:

```bash
mkdir -p build
cd build
~/Qt/6.10.1/macos/bin/qmake ../agenda_qt.pro
make
open agenda_qt.app
```

---

## Build with the course Docker environment

The repository includes the Dockerfile used to validate the project against the course environment.

From the project root:

```bash
docker build -t unipd-oop/qt-env:2025 .
docker run --rm -it -v "$PWD":/workspace -w /workspace unipd-oop/qt-env:2025 bash
```

Inside the container:

```bash
mkdir -p build_docker
cd build_docker
qmake6 ../agenda_qt.pro
make -j$(nproc)
```

The GUI does not need to be launched inside Docker for build validation, because graphical forwarding depends on the host system.
The important part is that `qmake6` and `make` complete without errors.

---

## Demo data

A ready-to-use sample agenda is available at:

```text
examples/sample_agenda.json
```

To load it:

1. Open the application
2. Select **File → Load...**
3. Choose `examples/sample_agenda.json`

The sample agenda contains:

- custom categories
- all four activity types
- completed activities
- overdue activities
- recurring activities
- high and critical priority activities
- checklist items
- saved templates

This makes it useful for testing and for quickly demonstrating the main features.

---

## Persistence format

Agenda data is stored in JSON files.

The saved state can include:

- activities
- categories
- templates
- recurrence rules
- type-specific activity fields

The persistence layer reconstructs the correct concrete activity type when loading the file.
Older JSON files without templates or categories are handled with optional sections where possible.

---

## Main design choices

### Separation of responsibilities

The project is split into four main areas:

- `model`: activity hierarchy, managers, filtering, search, recurrence and categories
- `gui`: Qt Widgets windows and dialogs
- `persistence`: JSON serialization and deserialization
- `commands`: undo/redo actions

The model does not depend on Qt Widgets.
This keeps the business logic more independent from the interface.

### Command-based undo/redo

Undo and redo are implemented using command objects.
The command history manages reversible actions such as adding, removing, editing and completing activities.

### Polymorphic cloning

Activities can be cloned through the common `Activity` interface.
This is used for templates and safe object duplication without forcing the GUI to manually duplicate every concrete activity type.

### Dynamic overdue calculation

Overdue states are calculated using the current system date and time.
They are not based on a hardcoded day.

---

## Useful documentation

Additional project notes are available in:

- `docs/development_log.md`
- `docs/extra_features.md`

These files document the development process, design decisions, implemented features and validation steps.

---

## Current status

The project has been validated with:

- local Qt build on macOS
- qmake build system
- official course Docker environment
- sample agenda loading
- manual GUI checks for creation, editing, categories, filters, templates, save/load and undo/redo

