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

### 2026-06-17 - ActivityManager

Implemented `ActivityManager`, the model-level class responsible for owning and managing activities.

Added:

- storage based on `std::vector<std::unique_ptr<Activity>>`
- safe activity insertion
- activity removal by id
- activity replacement by id
- lookup by id
- duplicate id prevention
- read-only access to stored activities through non-owning `const Activity*` pointers
- activity cloning by id

Design notes:

- `ActivityManager` owns all activities and prevents unsafe memory handling.
- Activities are stored polymorphically through `std::unique_ptr<Activity>`.
- The class is independent from the GUI and does not use Qt Widgets.
- `removeActivity(...)` returns the removed activity, which will be useful later for undo/redo commands.
- `replaceActivity(...)` requires the replacement object to preserve the same id, preventing accidental identity changes.

Validation:

- Updated temporary `main.cpp` test code to use `ActivityManager`.
- Verified that four different concrete activity types can be stored and displayed polymorphically.
- Verified successful qmake/make compilation.

Estimated time spent: 1h

### 2026-06-17 - Advanced activity search

Implemented `SearchEngine`, a model-level component responsible for searching activities independently from the GUI.

Added:

- case-insensitive search
- partial matching
- whitespace normalization
- accent-insensitive normalization
- search across title, category, description and polymorphic summary
- result ranking through weighted fields
- typo suggestion using Levenshtein distance when no direct result is found

Design notes:

- The search engine returns direct results only when there is an actual normalized partial match.
- Fuzzy matching is not mixed with normal results.
- If no direct result is found, the engine may return a separate suggestion.
- This keeps the user interface clear because real matches and typo suggestions are not confused.
- Search uses the base `Activity` interface and can work with any current or future concrete activity type.
- The use of `summary()` lets the search include type-specific information through polymorphism without checking the concrete class manually.

Validation:

- Added temporary search tests in `main.cpp`.
- Verified normal partial search with query `qt`.
- Verified typo suggestion with query `projet`, which suggests `Submit Qt project`.
- Verified successful qmake/make compilation.

Estimated time spent: 1.5h

### 2026-06-17 - Advanced activity search

Implemented `SearchEngine`, a model-level component responsible for searching activities independently from the GUI.

Added:

- case-insensitive search
- partial matching
- whitespace normalization
- accent-insensitive normalization
- search across title, category, description and polymorphic summary
- result ranking through weighted fields
- typo suggestion using Levenshtein distance when no direct result is found

Design notes:

- The search engine returns direct results only when there is an actual normalized partial match.
- Fuzzy matching is not mixed with normal results.
- If no direct result is found, the engine may return a separate suggestion.
- This keeps the user interface clear because real matches and typo suggestions are not confused.
- Search uses the base `Activity` interface and can work with any current or future concrete activity type.
- The use of `summary()` lets the search include type-specific information through polymorphism without checking the concrete class manually.
- All user-facing text currently displayed by the application has been kept in Italian.

Validation:

- Added temporary search tests in `main.cpp`.
- Verified normal partial search with query `qt`.
- Verified typo suggestion with query `progeto`, which suggests `Consegnare il progetto Qt`.
- Verified successful qmake/make compilation.

Estimated time spent: 1.5h

### 2026-06-17 - User-facing text switched to English

Updated the visible application text from Italian to English.

Reason:

- The professor confirmed that the application can be written in English.
- English makes the project more suitable for a public GitHub portfolio.
- The final report will still be written in Italian.

Changed:

- temporary test output in `main.cpp`
- activity summaries in concrete activity classes
- activity kind display strings

Design notes:

- Source code, class names, commits and development documentation remain in English.
- The final academic report will be written in Italian.
- No Git rollback was performed because the language change was mixed with functional commits. A dedicated refactoring commit is safer and cleaner.

Validation:

- Recompiled successfully with qmake and make.
- Verified that the application opens correctly.
- Verified that visible application text is now in English.

Estimated time spent: 0.5h

### 2026-06-17 - Customizable categories

Implemented model-level support for customizable activity categories.

Added:

- `Category` model class
- `CategoryManager` class
- category creation
- category update
- category deletion by id and by name
- duplicate category name prevention
- stable category ids
- category color validation using hex color strings
- activity category propagation support through `ActivityManager::replaceCategory(...)`
- activity category clearing support through `ActivityManager::clearCategory(...)`

Design notes:

- Categories are managed independently from the GUI.
- Category names are normalized when checking duplicates, so names differing only by case or extra spaces are treated as duplicates.
- Activities currently store their category as a string, which keeps filtering and search simple.
- `CategoryManager` prepares the project for future JSON persistence and GUI category management.
- `ActivityManager` provides explicit methods to propagate category rename/delete operations to existing activities.
- This keeps responsibility separated: `CategoryManager` manages categories, while `ActivityManager` updates activities.

Validation:

- Added temporary test data in `main.cpp`.
- Created initial categories.
- Renamed `University` to `Study`.
- Propagated the category rename to existing activities.
- Removed an unused category.
- Verified that filtering by the renamed category works.
- Verified successful qmake/make compilation.

Estimated time spent: 1.5h

### 2026-06-17 - Recurring activities

Implemented model-level support for recurring activities.

Added:

- `RecurrenceRule` model class
- recurrence frequencies:
  - daily
  - weekly
  - monthly
  - yearly
- recurrence interval support
- recurrence ending modes:
  - never
  - until date
  - after a fixed number of occurrences
- recurrence validation
- next occurrence calculation
- optional recurrence support in `Activity`
- recurrence removal through `Activity::clearRecurrenceRule()`
- recurring activity filtering through `ActivityFilter`

Design notes:

- Recurrence is stored as an optional rule inside `Activity`.
- Infinite recurrence is supported only through the explicit `EndMode::Never` option.
- The default recurrence behavior avoids accidental infinite recurrence by using `AfterOccurrences`.
- Recurring activities do not generate infinite copies in the model.
- The next occurrence is calculated dynamically from the activity primary date and the recurrence rule.
- Recurrence logic is independent from Qt Widgets and GUI code.
- `Activity::nextOccurrenceAfter(...)` uses the polymorphic `primaryDate()` method, so each concrete activity type determines the starting date according to its own behavior.

Validation:

- Added temporary recurring test data in `main.cpp`.
- Assigned a weekly recurrence to a reminder.
- Assigned a repeated daily recurrence to a checklist with an end date.
- Verified next occurrence calculation.
- Verified filtering recurring activities only.
- Verified successful qmake/make compilation.

Estimated time spent: 2h

### 2026-06-17 - Activity templates

Implemented model-level support for activity templates.

Added:

- `ActivityTemplate` model class
- `ActivityTemplateManager` class
- template creation
- template update
- template deletion
- duplicate template name prevention
- creation of new activities from templates
- deep-copy support for templates
- `Activity::cloneWithNewId()` for creating independent activities from prototypes

Design notes:

- Templates store a prototype activity.
- Creating an activity from a template uses polymorphic cloning.
- `clone()` preserves the original activity identity and remains useful for undo/redo.
- `cloneWithNewId()` creates a separate activity with a new id and updated timestamps, which is appropriate for templates and future duplicate actions.
- This implementation applies the Prototype Pattern in a concrete and useful way.
- The template system is independent from Qt Widgets and GUI code.
- Templates are prepared for future JSON persistence and GUI integration.

Validation:

- Added temporary template data in `main.cpp`.
- Created an exam deadline template.
- Created a study checklist template.
- Generated a new activity from the exam deadline template.
- Verified that the generated activity is inserted into `ActivityManager`.
- Verified successful qmake/make compilation.

Estimated time spent: 1.5h

### 2026-06-17 - Activity templates

Implemented model-level support for activity templates.

Added:

- `ActivityTemplate` model class
- `ActivityTemplateManager` class
- template creation
- template update
- template deletion
- duplicate template name prevention
- creation of new activities from templates
- deep-copy support for templates
- `Activity::cloneWithNewId()` for creating independent activities from prototypes

Design notes:

- Templates store a prototype activity.
- Creating an activity from a template uses polymorphic cloning.
- `clone()` preserves the original activity identity and remains useful for undo/redo.
- `cloneWithNewId()` creates a separate activity with a new id and updated timestamps, which is appropriate for templates and future duplicate actions.
- This implementation applies the Prototype Pattern in a concrete and useful way.
- The template system is independent from Qt Widgets and GUI code.
- Templates are prepared for future JSON persistence and GUI integration.

Validation:

- Added temporary template data in `main.cpp`.
- Created an exam deadline template.
- Created a study checklist template.
- Generated a new activity from the exam deadline template.
- Verified that the generated activity is inserted into `ActivityManager`.
- Verified successful qmake/make compilation.

Estimated time spent: 1.5h

### 2026-06-19 - First GUI prototype

Implemented the first real GUI prototype.

Added:

- `MainWindow` class
- activity list view
- activity detail view
- search input
- activity type filter
- automatic list refresh when search or filter changes
- selection-based detail panel
- demo data moved from static text output to structured GUI population

Design notes:

- The GUI uses the existing model classes instead of duplicating activity logic.
- Activity details are still read-only in this first prototype.
- The model remains separated from Qt Widgets.
- Search uses `SearchEngine`.
- Type filtering uses `ActivityFilter`.
- The activity list stores activity ids instead of copying full objects.
- Demo data remains temporary and will later be replaced by JSON persistence.

Validation:

- Verified successful qmake/make compilation.
- Verified that the main window opens correctly.
- Verified that activities are displayed in the list.
- Verified that selecting an activity updates the detail panel.
- Verified that search updates the visible activities.
- Verified that the activity type filter works.

Estimated time spent: 2h

### 2026-06-19 - Basic GUI actions

Implemented basic interactive GUI actions.

Added:

- toggle completion action for the selected activity
- delete action for the selected activity
- confirmation dialog before deletion
- automatic detail refresh after completion changes
- automatic list refresh after deletion
- disabled action buttons when no activity is selected
- improved QSS styling for buttons, combo boxes, inputs and lists

Design notes:

- The GUI modifies activities through `ActivityManager`.
- The activity list keeps using activity ids instead of copying objects.
- The completion toggle uses the existing `Activity::setCompleted(...)` method.
- Deletion uses `ActivityManager::removeActivity(...)`.
- The confirmation dialog prevents accidental deletion.
- Styling was made explicit to avoid unreadable white-on-white widgets on macOS.

Validation:

- Verified successful qmake/make compilation.
- Verified that the toggle completed button updates the selected activity status.
- Verified that the detail panel refreshes after toggling completion.
- Verified that the delete button asks for confirmation.
- Verified that confirmed deletion removes the activity from the list.
- Verified that buttons and filters are visible with the updated QSS.

Estimated time spent: 1.5h
