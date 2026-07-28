# Mandatory requirements audit

Audit date: 2026-07-28  
Repository: `hipietro/qt_agenda`  
Audited baseline: `main` after PR #70 (`2b6829abf44580385cf51e5e153017d87bf9ff19`)

## Purpose

This document is the formal release-gate audit requested by issue #51. It records concrete evidence for every mandatory project requirement and separates verified code compliance from evidence that must still be produced during documentation and final packaging.

A requirement marked `PASS` is supported by current source code, automated tests, or an already completed validation run. `PENDING` and `BLOCKED` items prevent final resubmission even though they do not invalidate the completed architectural work.

## Executive result

- 12 requirements: `PASS`;
- 1 requirement: `CONDITIONAL` pending an explicit author declaration in the final report;
- 1 requirement: `PENDING` clean-room Docker validation in issue #53;
- 1 requirement: `BLOCKED` until the compliant PDF report is completed in issue #52.

The corrected architecture satisfies the three requirements that previously failed: non-trivial polymorphism, removal of type-code dispatch as a substitute for polymorphism, and in-window creation/editing workflows.

Final packaging remains blocked by issues #52 and #53.

## Requirement-by-requirement audit

### 1. Original individual work

**Status: CONDITIONAL — final author attestation required**

Evidence:

- the repository and issue history show one student-owned project evolving through focused corrective issues and pull requests;
- implementation notes and comments describe project-specific design decisions;
- technical inspection found a coherent architecture rather than unrelated copied fragments.

Limitation:

Originality cannot be proven by static analysis. The final PDF must contain an explicit statement that the submission is original individual work. This evidence is tracked by issue #52.

### 2. Entire implementation written in C++

**Status: PASS**

Evidence:

- `agenda_qt.pro` declares a C++17 Qt application;
- all implementation files under `src/` are `.cpp` or `.h` files;
- Qt resources and the stylesheet are presentation assets, not a second implementation language;
- `scripts/requirements_audit.sh` fails if a non-C++ implementation file appears under `src/`.

### 3. Qt graphical interface

**Status: PASS**

Evidence:

- `agenda_qt.pro` links `core`, `gui`, and `widgets`;
- `MainWindow` derives from `QMainWindow`;
- the application uses Qt Widgets including lists, stacked pages, menus, forms, file dialogs, message boxes, and a monthly overview;
- GUI interaction tests instantiate and exercise the real `MainWindow`.

### 4. Error-free build in the supplied Docker container

**Status: PENDING — issue #53**

Evidence already available:

- the repository contains the supplied course `Dockerfile` based on Ubuntu 24.04 and Qt 6;
- GitHub Actions successfully configures and builds both test targets on Ubuntu 24.04 with Qt 6.4.2;
- the main project has a complete qmake target in `agenda_qt.pro`.

Required final evidence:

- build a clean extracted submission with `qmake6 ../agenda_qt.pro` and `make` inside the supplied Docker image;
- record the exact command and successful output.

The README contains an older Docker-validation claim, but this audit does not treat that statement as sufficient evidence after the architectural changes. Clean-room validation is tracked by issue #53.

### 5. Encapsulation, information hiding, and single-concept classes

**Status: PASS**

Evidence:

- `Activity` keeps identity, common fields, timestamps, completion state, and recurrence data private;
- each concrete activity owns only its type-specific state;
- managers encapsulate activity, category, and template collections;
- storage, filtering, searching, rendering, editing, commands, keyboard navigation, and mouse interaction are separated into focused classes;
- mutation of model state is performed through model methods and command objects rather than direct public fields.

### 6. Clear separation between logical model and GUI

**Status: PASS**

Evidence:

- source code is divided into `model`, `gui`, `persistence`, and `commands`;
- `ActivityVisitor` belongs to the model and forward-declares concrete model types without depending on Qt Widgets;
- GUI visitors depend on the model, while the model does not depend on GUI visitors;
- `scripts/requirements_audit.sh` rejects Qt Widgets includes in model headers.

Qt Core value types such as `QString`, `QDateTime`, `QVector`, and `QJsonObject` do not violate the requirement, which specifically concerns separation from the graphical interface.

### 7. Efficient and robust execution without runtime failures

**Status: PASS for automated validation; final smoke test remains in #53**

Evidence:

- core suite: 9 passed, 0 failed on Ubuntu and macOS;
- GUI suite: 3 passed, 0 failed on Ubuntu/Xvfb and macOS;
- tests cover malformed JSON, exact persistence round trips, command history, repeated undo/redo, filters, search, page navigation, and contextual-menu interactions;
- save/load functions return errors and surface them through the GUI;
- null checks, validation, unsaved-change confirmation, and command failure handling are present in the main workflows;
- GitHub Actions has a job timeout and per-test-function timeout.

A final end-to-end manual smoke test of the extracted package is still required by issue #53.

### 8. Non-trivial polymorphism with clearly different dynamic behavior

**Status: PASS — second review completed**

Primary evidence:

1. **Visitor hierarchy**
   - `Activity::accept(ActivityVisitor&)` performs double dispatch;
   - `ActivityListItemVisitor` creates meaningfully different list cards;
   - `ActivityDetailVisitor` creates different detail structures;
   - `ActivityEditFormVisitor` populates, validates, and rebuilds different edit forms;
   - `ActivityJsonSerializationVisitor` serializes different type-specific fields.

2. **Command hierarchy**
   - `Command::execute()` and `Command::undo()` are abstract dynamic operations;
   - add, update, remove, and completion commands preserve and restore different state;
   - `CommandHistory` works only through the abstract command interface.

3. **Concrete activity behavior**
   - event duration and interval semantics;
   - deadline due-state and hard-deadline data;
   - reminder time and advance-notice data;
   - checklist progress and completion derived from child items.

Automated evidence:

- `AgendaTests::visitorDoubleDispatch`;
- `AgendaTests::visitorRenderers`;
- `AgendaTests::commandHistoryRegression`;
- `AgendaTests::activityJsonRoundTrip`.

This is non-trivial behavioral polymorphism. Compliance does not rely on simple getters, labels, summaries, or cloning alone.

### 9. No `getType`/`kind`-style control flow used in place of polymorphism

**Status: PASS — second review completed**

Reviewed uses of `ActivityKind`:

- filter metadata in `ActivityFilter`;
- user-selectable type filters in `MainWindow`;
- the creation form's user-selected type and concrete object-construction boundary;
- descriptive `kind()` implementations on concrete classes.

These uses are acceptable because they select/filter data or construct a new object before dynamic dispatch is possible.

Type-specific behavior for existing activities does not switch on `kind()`:

- list rendering uses `ActivityListItemVisitor`;
- detail rendering uses `ActivityDetailVisitor`;
- editing uses `ActivityEditFormVisitor`;
- serialization uses `ActivityJsonSerializationVisitor`;
- deserialization uses `ActivityFactoryRegistry`;
- reversible actions use the `Command` hierarchy.

`scripts/requirements_audit.sh` rejects newly introduced `switch` or conditional dispatch based on `Activity::kind()` outside the allowed filtering boundary.

### 10. At least three meaningfully different concrete activity classes

**Status: PASS**

The project contains four concrete classes:

- `EventActivity`: start/end time, location, participants;
- `DeadlineActivity`: due date, context, hard-deadline flag;
- `ReminderActivity`: reminder time, advance minutes, reminder note;
- `ChecklistActivity`: target date, checklist items, progress, derived completion.

Their data, validation, rendering, serialization, and editing behavior differ materially.

### 11. Search, creation, visualisation, editing, and deletion through the GUI, including type-specific attributes

**Status: PASS**

Evidence:

- search is exposed through `MainWindow` and `SearchEngine`;
- creation uses `ActivityCreationPage` with a stacked form for all four types;
- list and detail visualisation use polymorphic visitors;
- editing uses `ActivityEditPage` and `ActivityEditFormVisitor`;
- deletion is exposed through the main buttons and contextual menu and executed through `RemoveActivityCommand`;
- type-specific fields are available for events, deadlines, reminders, and checklists;
- filters, sorting, completion toggling, templates, recurrence, and categories are additional GUI workflows.

### 12. Local persistence in at least one structured format

**Status: PASS**

Evidence:

- `AgendaJsonStorage` saves and loads JSON files;
- `ActivityJsonSerializationVisitor` writes type-specific data;
- `ActivityFactoryRegistry` reconstructs the correct concrete activity class;
- activities, categories, templates, recurrence, completion state, participants, and checklist entries are covered by round-trip tests;
- `examples/sample_agenda.json` provides structured demonstration data.

### 13. Save and load available at runtime through graphical file dialogs with no hard-coded paths

**Status: PASS**

Evidence in `MainWindow.cpp`:

- `QFileDialog::getSaveFileName` implements Save As;
- `QFileDialog::getOpenFileName` implements Load;
- Save reuses only the path previously chosen by the user;
- cancellation is handled safely;
- errors display the selected file and failure reason;
- tests use `QTemporaryDir`, not user-specific paths.

### 14. Main workflows navigated inside the same main window, especially creation and editing

**Status: PASS — second review completed**

Evidence:

- `MainWindow` owns `m_workspaceStack`;
- `ActivityCreationPage` and `ActivityEditPage` derive from `QWidget`, not `QDialog`;
- both pages are added directly to the main window's workspace stack;
- save/cancel handlers navigate back to the agenda detail page;
- responsive layout logic keeps the workflow inside the main window on both wide and compact windows;
- no `ActivityCreationDialog` or `ActivityEditDialog` remains in the project;
- `GuiInteractionTests::internalPagesAndContextMenu` verifies the `activityCreationPage` and `activityEditPage` navigation without an active modal widget.

### 15. A compliant PDF report containing every required section

**Status: BLOCKED — issue #52**

The current repository documentation is not sufficient final evidence. In particular, the README still contains outdated wording that presents `kind()` as a principal polymorphic mechanism and refers to creation/editing dialogs.

Issue #52 must produce and validate the final PDF with:

- name, surname, and student number;
- introduction;
- model description;
- explicit non-trivial polymorphism analysis;
- persistence description;
- additional features;
- hour accounting;
- `Modifiche rispetto alla consegna precedente`;
- updated UML diagrams;
- maximum 8 pages using readable 10 pt text.

The PDF must explicitly explain the corrections for requirements 8, 9, and 14.

## Static invariants enforced by CI

`scripts/requirements_audit.sh` checks that:

- required project, Docker, test, and example files exist;
- implementation files under `src/` are C++ headers or sources;
- model headers do not include Qt Widgets classes;
- all four concrete activity classes exist;
- Visitor-based rendering, editing, and serialization contracts exist;
- Command execute/undo behavior remains abstract and dynamic;
- obsolete creation/edit dialog names do not reappear;
- existing-activity behavior is not dispatched through `Activity::kind()`;
- creation and editing pages remain inside `MainWindow`;
- runtime save/load continues to use graphical file dialogs and JSON storage.

These checks prevent architectural regressions but do not replace the manual authorship declaration, PDF review, Docker build, or final smoke test.

## Validation evidence reviewed

- GitHub Actions run `30317215711`: configure, build, core tests, GUI tests, and artifact upload passed on Ubuntu 24.04 / Qt 6.4.2;
- local macOS / Apple Silicon / Qt 6.10.1:
  - core target: 9 passed, 0 failed;
  - GUI target: 3 passed, 0 failed;
- PR #70 merged as commit `2b6829abf44580385cf51e5e153017d87bf9ff19`;
- issue #50 closed after real CI and local validation.

## Blocking follow-up work

### Issue #52 — documentation and report

Must resolve requirements 1 and 15 and remove obsolete architectural descriptions from README/UML/report.

### Issue #53 — Docker and final package

Must provide final evidence for requirement 4 and repeat the complete smoke test against the exact extracted submission archive.

## Release decision

The implementation may proceed to documentation work, but it is **not ready for resubmission** until issues #52 and #53 are completed. No additional architectural blocker was found during this audit.
