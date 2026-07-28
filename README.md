# Agenda Qt

[![Qt tests](https://github.com/hipietro/qt_agenda/actions/workflows/tests.yml/badge.svg)](https://github.com/hipietro/qt_agenda/actions/workflows/tests.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52)
![Build system](https://img.shields.io/badge/build-qmake-F39C12)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey)

**Agenda Qt** is a desktop agenda application written in C++17 with Qt Widgets. It manages events, deadlines, reminders, and checklists through one polymorphic model while giving every concrete activity its own data, validation rules, list card, detail view, edit workflow, and JSON representation.

The project was developed for the Object-Oriented Programming course at the University of Padua. Its architecture focuses on non-trivial polymorphism, model/GUI separation, in-window workflows, local JSON persistence, reversible commands, and automated regression testing.

> **Source release:** prebuilt application bundles are not currently provided. Qt 6 and a C++17 development environment are required to build the application. Follow [INSTALL.md](INSTALL.md) from a clean machine.

## Highlights

- four concrete activity types with dedicated fields and visual presentation;
- create, view, edit, delete, complete, save, and load entirely from the GUI;
- creation and editing pages hosted inside `MainWindow`;
- weighted, case-insensitive, accent-insensitive, typo-tolerant search;
- combined filters and deterministic sorting;
- categories, recurrence rules, reusable templates, and monthly overview;
- undo/redo for add, remove, update, and completion operations;
- contextual mouse/trackpad actions and keyboard navigation;
- resizable workspace splitters, consistent SVG icons, compact controls, and type-specific cards;
- automated core, architecture, persistence, and GUI interaction tests.

## Install and run

The complete dependency, `PATH`, build, and first-run instructions are in **[INSTALL.md](INSTALL.md)**.

After Qt and the compiler are available, the portable quick start is:

```bash
git clone https://github.com/hipietro/qt_agenda.git
cd qt_agenda

rm -rf build
mkdir build
cd build

QMAKE_BIN="$(command -v qmake6 || command -v qmake)"
"$QMAKE_BIN" ../agenda_qt.pro
make -j2
```

Run the generated application:

```bash
# macOS
open agenda_qt.app

# Linux
./agenda_qt
```

The installation guide also covers:

- Xcode Command Line Tools and the Qt Online Installer on macOS;
- persistent Qt `PATH` configuration for zsh;
- Ubuntu/Debian packages and `qmake6`;
- the supplied course Docker image;
- verification commands and common setup errors.

## Activity types

| Type | Specific data | Dedicated presentation |
| --- | --- | --- |
| `EventActivity` | start/end time, location, participants | interval, duration, location, participants, calendar icon |
| `DeadlineActivity` | due date, context, hard-deadline flag | due state, overdue badge, rigidity, deadline icon |
| `ReminderActivity` | reminder time, advance notice, note | notification time, advance calculation, bell icon |
| `ChecklistActivity` | target date and checklist items | completion counter, progress bar, item state, checklist icon |

The logical objects are owned through `std::unique_ptr`. Managers expose controlled operations instead of public mutable collections.

## User interface

The main window is divided by resizable Qt splitters:

- the left panel contains search, filters, sorting, activity cards, and actions;
- the right workspace contains the selected activity details and monthly overview;
- creation and editing replace the right workspace through a `QStackedWidget`, without opening separate activity windows.

The list and detail widgets are built through Visitor double dispatch. Each concrete type therefore renders different widgets, icons, badges, sections, and progress information rather than relying on one generic text summary.

### Mouse and trackpad

| Interaction | Result |
| --- | --- |
| Single left click | selects the activity and shows its details |
| Double left click | opens the internal edit page |
| Right click / secondary click | opens actions for the item under the pointer |
| Drag main divider | changes the width of list and workspace |
| Drag detail divider | changes the balance between details and monthly overview |
| Click a calendar day | filters activities by date |

### Keyboard shortcuts

Qt exposes the platform-native equivalent where applicable.

| Shortcut | Action |
| --- | --- |
| `Ctrl/Cmd+N` | create a new activity |
| `Ctrl+E` | edit the selected activity |
| `Ctrl/Cmd+F` | focus and select the search field |
| `Ctrl/Cmd+O` | load an agenda |
| `Ctrl/Cmd+S` | save the current agenda |
| `Ctrl/Cmd+Shift+S` | save to a new file |
| `Ctrl+T` | create from a template |
| `Ctrl+Shift+T` | save the selected activity as a template |
| `Ctrl+Shift+C` | manage categories |
| native Undo / Redo | undo or redo the last command |
| `Enter` in the list | edit the selected activity |
| `Space` in the list | toggle completion |
| `Delete` / `Backspace` in the list | delete the selected activity |
| `Enter` in a form | submit the creation or edit form |
| `Ctrl/Cmd+Enter` in multiline fields | submit without losing multiline editing |
| `Esc` in a form | cancel and return to the agenda |

## Search, filters, and sorting

`SearchEngine` normalizes text through Unicode decomposition, lowercase conversion, whitespace simplification, and removal of diacritical marks. Matching is therefore case-insensitive and accent-insensitive.

Direct matches are weighted by field and match quality. When no direct result exists, the engine uses Levenshtein distance on complete fields and words to tolerate small typing errors.

Filters can be combined by activity type, priority, category, completion state, recurrence state, overdue state, and selected calendar date. Sorting supports date, title, priority, completion, creation time, and update time with deterministic tie-breaking.

## Object-oriented architecture

### Visitor double dispatch

`Activity::accept(ActivityVisitor&)` selects the correct overload for the dynamic activity type.

| Visitor | Responsibility |
| --- | --- |
| `ActivityListItemVisitor` | builds type-specific list cards with icons, badges, dates, state, and progress |
| `ActivityDetailVisitor` | builds different detail sections for all concrete types |
| `ActivityEditFormVisitor` | populates, validates, and rebuilds the correct edit form |
| `ActivityJsonSerializationVisitor` | writes common and type-specific JSON fields |

Existing activities are not rendered, edited, or serialized through a `kind()` switch.

### Command hierarchy

`CommandHistory` depends only on the abstract `Command` interface. `AddActivityCommand`, `RemoveActivityCommand`, `UpdateActivityCommand`, and `ToggleCompletionCommand` implement different forward and inverse operations.

Undo and redo transfer command ownership between two `std::unique_ptr` stacks and expose descriptive labels in the GUI.

### Persistence factory

Deserialization uses `ActivityFactoryRegistry`, which maps persisted type identifiers to concrete factory functions. This avoids a central switch over activity types during loading.

## Persistence

Agenda data is stored in local JSON files selected through `QFileDialog`. The document contains common and type-specific activity fields, categories, templates, recurrence rules, checklist items, completion state, identifiers, and millisecond-precision timestamps.

Loading validates the JSON structure and required values before replacing the current managers. I/O errors are shown to the user without silently corrupting the current agenda.

A ready-to-use example containing all activity types is available at [`examples/sample_agenda.json`](examples/sample_agenda.json).

## Project structure

```text
agenda_qt/
├── agenda_qt.pro
├── Dockerfile
├── INSTALL.md
├── LICENSE
├── CHANGELOG.md
├── examples/
│   └── sample_agenda.json
├── resources/
│   ├── icons/
│   ├── resources.qrc
│   └── style.qss
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

## Automated tests

The project has separate core and native GUI Qt Test targets. Full commands are documented in [`tests/README.md`](tests/README.md).

The suite covers Visitor double dispatch, type-specific rendering, JSON reconstruction and round trips, malformed input, filtering, deterministic sorting, normalized and fuzzy search, command history, internal creation/editing pages, and mouse interactions.

GitHub Actions runs the architecture audit, configures and builds both test targets, executes core and GUI tests on Ubuntu under Xvfb, and uploads the results.

## Release status

`v1.0.0` is intended to be the first stable **source release**. It does not include a signed `.dmg`, `.app`, AppImage, or Windows installer. See [CHANGELOG.md](CHANGELOG.md) for the release summary.

## Documentation

- [Installation and first run](INSTALL.md)
- [Release history](CHANGELOG.md)
- [Mandatory requirements audit](docs/requirements_audit.md)
- [Final report source and validation](docs/report/README.md)
- [UML and architecture diagrams](docs/uml/)
- [Test commands and coverage](tests/README.md)

## License

The repository is published for portfolio and academic review. See [LICENSE](LICENSE) for the permitted uses and restrictions against redistribution or submission as academic work.
