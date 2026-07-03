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

### 2026-06-19 - GUI layout and feedback improvements

Improved the first GUI prototype layout and user feedback.

Added:

- clearer activity list rows with activity status
- dynamic completion button text:
  - Mark completed
  - Mark active
- improved result count label
- clearer empty-state messages
- clearer no-results message for searches
- selection preservation after list refresh when possible
- improved button styling
- improved readability for search, filters, list and detail panels

Design notes:

- The GUI remains based on a single `MainWindow`.
- The window content updates according to the selected activity, search query and filters.
- The activity list continues to store activity ids instead of copying activity objects.
- The interface is still a prototype, but it is now easier to test and present.
- Future create/edit/category screens should be implemented inside the same main window, most likely through stacked internal pages instead of separate top-level windows.

Validation:

- Verified successful qmake/make compilation.
- Verified that activities show their status in the list.
- Verified that the completion button changes text according to the selected activity state.
- Verified that toggling completion keeps the selected activity visible when possible.
- Verified that empty search results show a clear message.
- Verified that buttons and filters are readable with the updated QSS.

Estimated time spent: 1h

### 2026-06-19 - Activity JSON serialization

Implemented model-level JSON serialization for activities.

Added:

- `ActivityJsonSerializer`
- serialization of common `Activity` fields
- serialization of activity type
- serialization of priority
- serialization of completion state
- serialization of creation and update timestamps
- serialization of optional recurrence rules
- serialization of `EventActivity` fields
- serialization of `DeadlineActivity` fields
- serialization of `ReminderActivity` fields
- serialization of `ChecklistActivity` fields

Design notes:

- JSON serialization is independent from Qt Widgets and GUI code.
- The serializer converts polymorphic activities into structured JSON objects.
- The activity type is saved explicitly so deserialization can later reconstruct the correct concrete class.
- Recurrence is serialized only when present.
- Checklist date serialization uses the polymorphic `primaryDate()` method, keeping the serializer compatible with the current model interface.

Validation:

- Verified successful qmake/make compilation.
- Verified that all concrete activity types are handled by the serializer.
- Verified that recurrence data can be converted to JSON.
- Verified that serialization code is located in the persistence layer.

Estimated time spent: 1.5h

### 2026-06-19 - Activity JSON deserialization

Implemented JSON deserialization for concrete activity reconstruction.

Added:

- `ActivityJsonSerializer::fromJson(...)`
- JSON reconstruction for `EventActivity`
- JSON reconstruction for `DeadlineActivity`
- JSON reconstruction for `ReminderActivity`
- JSON reconstruction for `ChecklistActivity`
- priority parsing from JSON strings
- activity type parsing from JSON strings
- recurrence parsing from JSON objects
- ISO date/time parsing from JSON strings
- support for restoring activity identity and timestamps

Updated:

- concrete activity constructors now support restoring `createdAt` and `updatedAt`
- `EventActivity`, `DeadlineActivity`, `ReminderActivity` and `ChecklistActivity` can now be reconstructed from persisted data

Design notes:

- Deserialization uses the saved activity type to rebuild the correct concrete subclass.
- Invalid or unknown activity types return `nullptr`.
- Recurrence is restored only when valid recurrence data is present.
- Preserving activity ids is necessary for future save/load consistency and GUI selection logic.
- Preserving timestamps avoids treating loaded activities as newly created ones.
- The persistence layer remains independent from Qt Widgets and GUI code.

Validation:

- Verified successful qmake/make compilation.
- Verified that all concrete activity types can be constructed from JSON data.
- Verified that recurrence data can be parsed and restored.
- Verified that constructor signatures are compatible with persistence requirements.

Estimated time spent: 2h

### 2026-06-20 - JSON file save and load for ActivityManager

Implemented JSON file persistence for the activity manager.

Added:

- `AgendaJsonStorage`
- saving an `ActivityManager` to a JSON file
- loading an `ActivityManager` from a JSON file
- root JSON object with file version
- activities array serialization
- complete reconstruction of activities through `ActivityJsonSerializer`
- error reporting through optional error message output
- validation of JSON structure
- validation of supported file version
- duplicated activity id detection during loading

Design notes:

- File persistence is implemented in the persistence layer.
- The GUI is not involved in the storage logic.
- The JSON root object contains a `version` field to support future format changes.
- Loading uses a temporary vector before replacing the current manager content.
- The current agenda is cleared only after the file has been parsed and all activities have been reconstructed successfully.
- This avoids leaving the application in a partially loaded state if the file is invalid.

Validation:

- Verified successful qmake/make compilation.
- Verified that `AgendaJsonStorage` can access `ActivityManager`.
- Verified that saving uses `ActivityJsonSerializer`.
- Verified that loading reconstructs concrete activities through `ActivityJsonSerializer::fromJson(...)`.
- Verified that the persistence layer remains independent from Qt Widgets.

Estimated time spent: 1.5h

### 2026-06-20 - GUI save and load actions

Added file save and load actions to the GUI.

Added:

- File menu
- Load action
- Save action
- Save As action
- keyboard shortcuts for file actions
- JSON save through `AgendaJsonStorage`
- JSON load through `AgendaJsonStorage`
- file dialogs for choosing save/load paths
- status bar feedback after save/load operations
- window title update based on the current file path

Design notes:

- File paths are selected through `QFileDialog`.
- No hardcoded persistence path is used.
- `Save` reuses the current file path when available.
- `Save As` lets the user choose a new file path.
- `Load` replaces the current agenda content only if the selected JSON file is valid.
- The GUI remains based on a single `MainWindow`.
- Deleting an activity does not automatically save the file; the user must explicitly save the current state.

Validation:

- Verified successful qmake/make compilation.
- Verified that Save As creates a JSON file.
- Verified that Load restores previously saved activities.
- Verified that deleted activities return when loading an older saved file.
- Verified that Cmd+S saves the current agenda state.
- Verified that file actions work without hardcoded paths.
- Verified that shortcuts work correctly on macOS.

Estimated time spent: 1.5h

### 2026-06-20 - Unsaved changes tracking

Implemented unsaved changes tracking in the GUI.

Added:

- dirty-state tracking for unsaved changes
- `*` marker in the window title when the agenda has unsaved changes
- confirmation dialog before loading another file with unsaved changes
- confirmation dialog before closing the app with unsaved changes
- dirty-state reset after successful save
- dirty-state reset after successful load
- dirty-state update after deleting an activity
- dirty-state update after toggling activity completion

Design notes:

- The GUI tracks whether the current in-memory agenda differs from the last saved or loaded file.
- The `*` marker gives immediate visual feedback to the user.
- Loading a file is blocked unless the user confirms discarding unsaved changes.
- Closing the application is blocked unless the user confirms discarding unsaved changes.
- This feature does not implement undo/redo yet; it only protects against accidental loss of unsaved work.
- Save and Load still rely on `AgendaJsonStorage`, keeping persistence logic separated from the GUI.

Validation:

- Verified successful qmake/make compilation.
- Verified that deleting an activity marks the agenda as unsaved.
- Verified that toggling completion marks the agenda as unsaved.
- Verified that saving removes the unsaved marker.
- Verified that loading a file with unsaved changes asks for confirmation.
- Verified that closing the app with unsaved changes asks for confirmation.
- Verified that loading a valid JSON file still refreshes the GUI correctly.

Estimated time spent: 1h

### 2026-06-21 - Activity creation dialog

Implemented the activity creation dialog.

Added:

- `ActivityCreationDialog`
- creation form for `EventActivity`
- creation form for `DeadlineActivity`
- creation form for `ReminderActivity`
- creation form for `ChecklistActivity`
- common fields shared by all activity types:
  - title
  - description
  - category
  - priority
- type-specific fields shown through an internal `QStackedWidget`
- validation for empty titles
- validation for event end time after start time
- add activity button in `MainWindow`
- insertion of created activities into `ActivityManager`
- automatic selection of the newly created activity
- unsaved changes tracking after activity creation

Improved:

- dialog layout and spacing
- readability of date/time widgets
- readability of combo boxes and spin boxes
- dropdown arrow visibility for combo boxes and date/time fields
- hover/selection feedback for combo box popup items
- checklist item visibility in the activity detail panel
- QSS styling for the creation workflow

Design notes:

- The creation dialog is separated from `MainWindow` to keep the main window focused on coordination and navigation.
- The dialog creates concrete activity subclasses and returns them as `std::unique_ptr<Activity>`.
- This keeps the rest of the GUI based on the abstract `Activity` interface.
- I used a `QStackedWidget` because each concrete activity type needs different input fields.
- I kept the dialog modal because it is a temporary data-entry step, not a separate persistent application window.
- The created activity is inserted through `ActivityManager`, so ownership remains centralized.
- The GUI remains based on a single main workflow, while necessary temporary dialogs are used only when appropriate.

Validation:

- Verified successful qmake/make compilation.
- Verified that Event activities can be created.
- Verified that Deadline activities can be created.
- Verified that Reminder activities can be created.
- Verified that Checklist activities can be created.
- Verified that checklist items are shown in the detail panel.
- Verified that created activities appear in the main list.
- Verified that the newly created activity is selected after creation.
- Verified that creating an activity marks the agenda as unsaved.
- Verified that saved and reloaded JSON files preserve created activities.
- Verified that date/time fields are readable and no longer show black-on-black text.

Estimated time spent: 2.5h

### 2026-06-21 - Activity edit dialog

Implemented the activity edit dialog.

Added:

- `ActivityEditDialog`
- editing support for `EventActivity`
- editing support for `DeadlineActivity`
- editing support for `ReminderActivity`
- editing support for `ChecklistActivity`
- edit activity button in `MainWindow`
- replacement of edited activities through `ActivityManager::replaceActivity(...)`
- preservation of activity id
- preservation of original creation timestamp
- update of modification timestamp after editing
- unsaved changes tracking after editing
- automatic reselection of the edited activity after refresh

Improved:

- checklist editing through selectable checkbox items
- checklist item addition from the edit dialog
- checklist item removal from the edit dialog
- checklist item completion state editing
- checklist details display in the main detail panel
- checkbox visibility in QSS

Design notes:

- Activity type cannot be changed while editing.
- I chose this because changing a concrete type would require converting type-specific fields and could cause data loss.
- The edit dialog creates an updated concrete activity and gives it back to `MainWindow`.
- `MainWindow` replaces the old object inside `ActivityManager`, keeping ownership centralized.
- Checklist items are edited through a `QListWidget` with checkboxes instead of a textual `[x]` format.
- I chose this because it avoids user input errors and keeps the GUI closer to the actual boolean state stored in `ChecklistItem`.

Validation:

- Verified successful qmake/make compilation.
- Verified that existing activities can be edited.
- Verified that Event fields can be updated.
- Verified that Deadline fields can be updated.
- Verified that Reminder fields can be updated.
- Verified that Checklist items can be added, removed and marked as completed.
- Verified that checklist progress updates correctly after editing.
- Verified that edited activities remain selected after refresh.
- Verified that editing marks the agenda as unsaved.
- Verified that checkbox indicators are visible in the edit dialog.

Estimated time spent: 2.5h

### 2026-06-21 - Recurrence editing controls

Implemented recurrence controls in the activity creation and edit dialogs.

Added:

- recurrence section in `ActivityCreationDialog`
- recurrence section in `ActivityEditDialog`
- repeat enable/disable checkbox
- frequency selection:
  - daily
  - weekly
  - monthly
  - yearly
- interval selection through a "Repeat every" control
- end condition selection:
  - never
  - until date
  - after occurrences
- conditional recurrence fields depending on the selected end condition
- recurrence preservation when editing an existing recurring activity
- recurrence removal when disabling repeats in the edit dialog
- integration with the existing `RecurrenceRule` model
- integration with existing JSON persistence

Design notes:

- Recurrence is treated as a common property of every activity type, not as a type-specific field.
- I chose to show recurrence controls inside both creation and edit dialogs because this makes the existing recurrence model visible and usable from the GUI.
- The GUI does not generate separate future activity instances yet; it stores and displays the recurrence rule associated with the activity.
- The current recurrence layout is functional but will be visually improved during the final UI polish phase.

Validation:

- Verified successful qmake/make compilation.
- Verified that a recurring activity can be created.
- Verified that recurrence appears in the activity details.
- Verified that recurring activities can be saved to JSON.
- Verified that recurring activities can be loaded from JSON.
- Verified that recurrence values are preserved when editing an activity.
- Verified that disabling repeats removes the recurrence rule from the activity.

Estimated time spent: 2h

### 2026-06-21 - Template-based activity creation

Implemented template-based activity creation in the GUI.

Added:

- `ActivityTemplateManager` lifetime moved from demo-data local scope to application scope
- `MainWindow` integration with `ActivityTemplateManager`
- `From template` button in the main window
- `Templates` menu
- creation of new activities from existing templates
- saving the selected activity as a new template
- template selection through a simple dialog
- automatic insertion of the generated activity into `ActivityManager`
- automatic selection of the newly created activity
- unsaved changes tracking after creating an activity from a template

Design notes:

- Templates are stored in memory for the current session.
- JSON persistence for templates is intentionally left to a separate issue.
- Template creation uses the existing polymorphic clone mechanism.
- The GUI does not manually duplicate type-specific fields; it asks `ActivityTemplateManager` to create a new activity from the selected template.
- This part was less immediate than expected because `ActivityTemplateManager` was originally created only inside demo-data setup, so it was destroyed before the GUI could use it. The solution was to move its lifetime to `main()` and pass it to `MainWindow`.

Validation:

- Verified successful qmake/make compilation.
- Verified that demo templates are available from the GUI.
- Verified that a new activity can be created from a template.
- Verified that the new activity is inserted into the agenda.
- Verified that the newly created activity is selected automatically.
- Verified that the selected activity can be saved as a new template.
- Verified that newly saved templates can be reused during the same session.

Estimated time spent: 2h

### 2026-06-21 - Input validation and user feedback

Improved input validation and user feedback across the activity workflow.

Added:

- validation for empty activity titles
- validation for event end time before or equal to start time
- validation for empty checklist creation
- validation for empty checklist editing
- validation for recurrence end date before or equal to the activity date
- user feedback when trying to remove a checklist item without selecting one
- protection against removing the last checklist item
- updated empty-agenda message in the detail panel
- clearer warning messages for invalid activity data

Improved:

- checklist editing now prevents the dialog from reaching an invalid empty state
- recurrence validation now checks that "Until date" is after the activity primary date
- feedback messages are more specific and easier to understand
- the main empty-state message now references the implemented creation actions

Difficulties encountered:

- The edit dialog initially used `selectedActivityKind()`, which only exists in the creation dialog. This caused a compilation error because the edit dialog stores the fixed activity type in `m_activityKind`.
- A missing closing brace in `ActivityEditDialog::validateForm()` caused multiple cascading compiler errors.
- The first checklist validation only blocked saving, but the user could still remove all checklist items inside the dialog. This made the interface confusing even if the final save was rejected.
- The recurrence validation had to be duplicated carefully in both creation and editing workflows because the two dialogs obtain the activity type differently.

Resolutions:

- Replaced `selectedActivityKind()` with `m_activityKind` inside the edit dialog.
- Rewrote the full `ActivityEditDialog::validateForm()` function to restore the correct block structure.
- Added a guard in the "Remove selected" checklist action to prevent deleting the final remaining item.
- Kept validation in `validateForm()` as a second safety layer.
- Added specific warning messages so the user understands why an action is rejected.

Validation:

- Verified successful qmake/make compilation.
- Verified that empty titles are rejected.
- Verified that invalid event times are rejected.
- Verified that checklist activities cannot be saved without items.
- Verified that the last checklist item cannot be removed from the edit dialog.
- Verified that recurrence "Until date" must be after the activity primary date.
- Verified that valid forms still save correctly.

Estimated time spent: 1.5h

### 2026-06-21 - Base Command interface

Implemented the base command interface for the future undo/redo system.

Added:

- `Command` abstract interface
- `execute()` method
- `undo()` method
- `description()` method
- project integration through `agenda_qt.pro`

Design notes:

- The command system is based on the Command pattern.
- Each future modifying action will be represented as an executable and undoable object.
- This keeps undo/redo logic separated from `MainWindow`.
- The base interface is intentionally small because concrete commands will handle the specific details of adding, removing, updating and toggling activities.

Validation:

- Verified successful qmake/make compilation.
- Verified that the new interface is included in the qmake project.

Estimated time spent: 0.5h

### 2026-06-21 - AddActivityCommand

Implemented the command used to add activities through the future undo/redo system.

Added:

- `AddActivityCommand`
- command execution for adding an activity to `ActivityManager`
- command undo logic for removing the added activity
- activity id tracking
- activity title tracking for command descriptions
- protection against executing the command twice while the activity is already present
- project integration through `agenda_qt.pro`

Design notes:

- The command stores a polymorphic prototype of the activity.
- When executed, the command adds a cloned activity to the manager.
- This design makes future redo possible because the command can recreate the activity after undo.
- The GUI is not connected to this command yet; integration will happen after the undo/redo stack is implemented.

Difficulties encountered:

- Ownership had to be handled carefully because `ActivityManager` owns activities through `std::unique_ptr`.
- Moving the original activity directly into the manager would make redo difficult.
- The solution was to keep a cloneable prototype inside the command and add cloned instances during execution.

Validation:

- Verified successful qmake/make compilation.
- Verified that the command is included in the qmake project.
- Verified that the implementation does not require GUI changes yet.

Estimated time spent: 0.75h

### 2026-06-21 - RemoveActivityCommand

Implemented the command used to remove activities through the future undo/redo system.

Added:

- `RemoveActivityCommand`
- command execution for removing an activity from `ActivityManager`
- command undo logic for restoring the removed activity
- activity id tracking
- activity title tracking for command descriptions
- removed activity backup through polymorphic cloning
- duplicate protection when undoing a removal
- project integration through `agenda_qt.pro`

Design notes:

- The command stores the id of the activity to remove.
- Before removing the activity, the command saves a polymorphic clone of it.
- Undo restores a cloned copy of the removed activity.
- This design preserves the original concrete type without adding type-specific logic to the command.
- The GUI is not connected to this command yet; integration will happen after the undo/redo stack is implemented.

Difficulties encountered:

- Removing an activity is more delicate than adding one because undo needs the full original object.
- Since activities are owned by `ActivityManager` through `std::unique_ptr`, the command cannot simply keep a raw pointer to the removed object.
- The solution was to clone the activity before removing it and use that clone as the restoration prototype.

Validation:

- Verified successful qmake/make compilation.
- Verified that the command is included in the qmake project.
- Verified that the implementation remains independent from Qt Widgets.
- Verified that the command relies on the existing polymorphic `clone()` behavior.

Estimated time spent: 0.75h

### 2026-06-21 - UpdateActivityCommand

Implemented the command used to update activities through the future undo/redo system.

Added:

- `UpdateActivityCommand`
- command execution for replacing an existing activity with an updated version
- command undo logic for restoring the previous activity state
- activity id tracking
- activity title tracking for command descriptions
- previous activity backup through polymorphic cloning
- updated activity prototype through polymorphic cloning
- project integration through `agenda_qt.pro`

Design notes:

- The command stores the updated activity as a prototype.
- On first execution, the command clones and stores the previous state of the activity.
- Undo restores the previous cloned state.
- A later redo can execute the command again without losing the original previous state.
- The command uses `ActivityManager::replaceActivity(...)`, keeping ownership centralized in the model layer.
- The GUI is not connected to this command yet; integration will happen after the undo/redo stack is implemented.

Difficulties encountered:

- Update is more complex than add/remove because it needs two states: before and after.
- The previous state must not be overwritten during redo.
- Since activities are polymorphic and owned through `std::unique_ptr`, both the previous and updated states must be stored through `clone()`.
- The solution was to store the previous state only on the first execution and reuse it for undo.

Validation:

- Verified successful qmake/make compilation.
- Verified that the command is included in the qmake project.
- Verified that the implementation remains independent from Qt Widgets.
- Verified that the command relies on existing polymorphic cloning and manager replacement logic.

Estimated time spent: 0.75h

### 2026-06-21 - ToggleCompletionCommand

Implemented the command used to toggle activity completion through the future undo/redo system.

Added:

- `ToggleCompletionCommand`
- command execution for changing an activity from active to completed, or from completed to active
- command undo logic for restoring the previous completion state
- activity id tracking
- activity title tracking for command descriptions
- project integration through `agenda_qt.pro`

Design notes:

- The command stores both the previous completion state and the new completion state.
- Undo restores the exact previous state instead of simply toggling again.
- This is safer because it avoids inconsistent behavior if the activity state changes unexpectedly before undo.
- The GUI is not connected to this command yet; integration will happen after the undo/redo stack is implemented.

Difficulties encountered:

- A simple toggle in undo would be fragile because it assumes no other operation changed the activity state.
- The safer solution was to store explicit before/after boolean values.

Validation:

- Verified successful qmake/make compilation.
- Verified that the command is included in the qmake project.
- Verified that the implementation remains independent from Qt Widgets.

Estimated time spent: 0.5h

### 2026-06-21 - CommandHistory undo/redo stacks

Implemented the undo/redo stack manager.

Added:

- `CommandHistory`
- undo stack
- redo stack
- command execution through `executeCommand(...)`
- undo operation
- redo operation
- undo availability check
- redo availability check
- undo description
- redo description
- stack size helpers
- clear operation
- project integration through `agenda_qt.pro`

Design notes:

- `CommandHistory` owns commands through `std::unique_ptr`.
- Executing a new command clears the redo stack.
- Undo moves a command from the undo stack to the redo stack.
- Redo moves a command from the redo stack back to the undo stack.
- If undo or redo fails, the command is restored to its original stack to keep the history consistent.
- The class is independent from Qt Widgets and does not depend on MainWindow.
- GUI integration will be handled in the next issue.

Difficulties encountered:

- The main design concern was keeping ownership clear while moving commands between stacks.
- Since commands are owned through `std::unique_ptr`, stack operations must use move semantics carefully.
- Another important detail was failure handling: if undo or redo fails, the command must not be lost.
- The solution was to temporarily move the command out of the stack, execute the operation, and restore it if the operation fails.

Validation:

- Verified successful qmake/make compilation.
- Verified that the undo/redo stack manager is included in the qmake project.
- Verified that the implementation remains independent from Qt Widgets.
- Verified that commands are moved safely between undo and redo stacks.

Estimated time spent: 0.75h
