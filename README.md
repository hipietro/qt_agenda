# Agenda Qt

[![Qt tests](https://github.com/hipietro/qt_agenda/actions/workflows/tests.yml/badge.svg)](https://github.com/hipietro/qt_agenda/actions/workflows/tests.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt 6](https://img.shields.io/badge/Qt-6-green)
![Build system](https://img.shields.io/badge/build-qmake-orange)

Agenda Qt is a desktop agenda application written in **C++17** with **Qt Widgets**. It manages events, deadlines, reminders, and checklists through a common object-oriented model while preserving genuinely different type-specific data and behavior.

The project was developed for the Object-Oriented Programming course at the University of Padua. Its architecture emphasizes behavioral polymorphism, model/GUI separation, local JSON persistence, reversible commands, and automated regression testing.

## Main features

- create, view, edit, delete, and complete activities;
- four concrete activity types with dedicated fields and validation;
- full-text search, combined filters, and multiple sort orders;
- categories, reusable templates, recurrence rules, and monthly overview;
- undo/redo for add, remove, update, and completion operations;
- local JSON save/load through graphical file dialogs;
- unsaved-change tracking and user-facing error handling;
- single-click selection, double-click editing, and contextual list actions.

## Object-oriented architecture

### Activity hierarchy

`Activity` is the abstract base class for:

| Concrete type | Type-specific state and behavior |
| --- | --- |
| `EventActivity` | start/end time, location, participants, interval semantics |
| `DeadlineActivity` | due date, context, hard-deadline flag |
| `ReminderActivity` | reminder time, advance notice, reminder note |
| `ChecklistActivity` | target date, child items, progress, derived completion |

Objects are owned through `std::unique_ptr`, and managers expose controlled operations rather than public collections.

### Non-trivial polymorphism

The main dynamic mechanism is **Visitor double dispatch** through `Activity::accept(ActivityVisitor&)`:

- `ActivityListItemVisitor` builds different list cards;
- `ActivityDetailVisitor` builds different detail structures;
- `ActivityEditFormVisitor` populates, validates, and rebuilds the correct edit form;
- `ActivityJsonSerializationVisitor` writes type-specific JSON fields.

The **Command hierarchy** provides a second behavioral abstraction. `CommandHistory` works only through the abstract `Command` interface, while concrete add, remove, update, and completion commands implement different `execute()` and `undo()` behavior.

`ActivityKind` is retained only as classifier/filter data and at the user-selected object-construction boundary. Existing-activity rendering, editing, and serialization do not dispatch through `kind()`.

## In-window workflows

Creation and editing are not modal activity dialogs. `ActivityCreationPage` and `ActivityEditPage` are `QWidget` pages owned by `MainWindow` and displayed inside its `QStackedWidget` workspace.

The category-management dialog remains a secondary utility; it is not used for the mandatory creation or editing workflows.

## Persistence

Agenda state is stored in local JSON files selected through `QFileDialog`. The saved document includes:

- activities and all type-specific fields;
- categories;
- templates;
- recurrence rules;
- completion state and checklist entries;
- identifiers and timestamps with millisecond precision.

Serialization uses `ActivityJsonSerializationVisitor`. Deserialization uses `ActivityFactoryRegistry`, which maps the persisted type identifier to the correct concrete factory without a central type switch.

A ready-to-use example is available at [`examples/sample_agenda.json`](examples/sample_agenda.json).

## Project structure

```text
agenda_qt/
├── agenda_qt.pro
├── Dockerfile
├── examples/
├── resources/
├── scripts/
│   └── requirements_audit.sh
├── src/
│   ├── commands/
│   ├── gui/
│   ├── model/
│   ├── persistence/
│   └── main.cpp
├── tests/
└── docs/
    ├── requirements_audit.md
    ├── report/
    └── uml/
```

## Build locally

Requirements: a C++17 compiler, Qt 6, qmake, and make.

```bash
mkdir -p build
cd build
qmake6 ../agenda_qt.pro
make -j2
```

On macOS with the project Qt installation:

```bash
mkdir -p build
cd build
~/Qt/6.10.1/macos/bin/qmake ../agenda_qt.pro
make -j"$(sysctl -n hw.ncpu)"
open agenda_qt.app
```

## Course Docker environment

```bash
docker build -t unipd-oop/qt-env:2025 .
docker run --rm -it \
  -v "$PWD":/workspace \
  -w /workspace \
  unipd-oop/qt-env:2025 bash
```

Inside the container:

```bash
mkdir -p build_docker
cd build_docker
qmake6 ../agenda_qt.pro
make -j$(nproc)
```

## Automated tests

The project has separate core and GUI Qt Test targets. Full commands are documented in [`tests/README.md`](tests/README.md).

The test suite covers:

- Visitor double dispatch and type-specific rendering;
- exact JSON and full-agenda round trips;
- malformed input handling and factory reconstruction;
- command execution and repeated undo/redo;
- filtering, sorting, and searching;
- internal creation/editing pages and list mouse interactions.

GitHub Actions runs the architecture audit, builds both targets, executes the core and GUI suites on Ubuntu/Xvfb, and uploads the test results.

## Documentation

- [Mandatory requirements audit](docs/requirements_audit.md)
- [Final project report](docs/report/Relazione_Agenda_Qt_Acampora.pdf)
- [UML and architecture diagrams](docs/uml/)
- [Test commands and coverage](tests/README.md)
