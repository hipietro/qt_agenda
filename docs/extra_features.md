# Extra features and design highlights

This document tracks the optional or enhanced features implemented in the project, beyond the minimum functional requirements.
Its purpose is to make these choices explicit for the final report and for project evaluation.

## Search improvements

### Case-insensitive search

The search system is case-insensitive.
This means that searching for `meeting`, `Meeting` or `MEETING` produces consistent results.

This improves usability because users do not need to remember the exact capitalization used when creating an activity.

### Accent-insensitive search

The search system normalizes text before comparing it.
This makes searches more tolerant when working with accented characters.

For example, a search can still match relevant text even when accents are typed differently.

### Partial search

The search system supports partial matches.
The user does not need to type the complete title or description of an activity.

This makes the search more practical in everyday use, especially when the agenda contains many activities.

### Ranked search results

Search results are not simply returned in arbitrary order.
The `SearchEngine` assigns relevance based on where the match occurs, for example giving more importance to matches in the title than matches in secondary text.

This makes the search more useful because the most relevant activities tend to appear first.

### Fuzzy suggestion when no direct result is found

If the search produces no exact or partial result, the system can suggest close matches using a similarity-based approach.

This is useful when the user makes small typing mistakes or remembers only an approximate title.

## Filtering and sorting

### Advanced activity filtering

The project includes a dedicated filtering component, `ActivityFilter`, which allows activities to be filtered by multiple criteria.

Supported filters include:

* activity type
* category
* priority
* completion state
* overdue state
* recurrence state
* date range

This goes beyond a basic agenda list because the user can narrow the visible activities according to practical needs.

### Multiple sorting criteria

Activities can be sorted using different criteria, such as:

* title
* primary date
* priority
* completion state
* creation date
* update date

This improves the flexibility of the application and keeps sorting logic separated from the GUI.

## Checklist improvements

### Checklist activities as a concrete activity type

Checklist activities are implemented as one of the concrete subclasses of `Activity`.

They are not just plain text notes: each checklist stores individual items and each item has its own completion state.

### Checklist progress calculation

Checklist activities can show their completion progress based on the number of completed items.

This gives the user more information than a simple completed/not completed state.

### Checklist item editing through GUI checkboxes

Checklist items can be edited using selectable checkbox items in the edit dialog.

This avoids requiring the user to manually write textual markers such as `[x]` or `[ ]`, reducing input errors and making the GUI more intuitive.

## Activity hierarchy and polymorphism

### Abstract base class with concrete activity types

The project uses an abstract `Activity` base class and multiple concrete derived classes:

* `EventActivity`
* `DeadlineActivity`
* `ReminderActivity`
* `ChecklistActivity`

Each concrete activity type defines its own behavior for methods such as:

* `kind()`
* `primaryDate()`
* `isOverdue()`
* `summary()`
* `clone()`

### Non-trivial polymorphism

The application relies on polymorphism to treat different activity types through the common `Activity` interface.

For example, the GUI and manager can work with `Activity` pointers while each concrete subclass provides its own summary, date logic and overdue behavior.

This avoids placing all behavior in a single monolithic class.

## Activity manager features

### Centralized ownership

Activities are stored and managed by `ActivityManager`.

The GUI does not own activities directly.
This keeps ownership centralized and makes memory management clearer.

### Safe replacement of edited activities

When an activity is edited, the edit dialog creates an updated concrete instance.
The `ActivityManager` then replaces the old activity while preserving the same id.

This keeps editing logic clean and avoids exposing too many type-specific setters in the model.

### Activity cloning

Activities support cloning through polymorphic `clone()` methods.

This is useful for features such as templates and safe object duplication.

## Templates

### Activity templates

The model includes support for reusable activity templates through `ActivityTemplate` and `ActivityTemplateManager`.

Templates allow the application to create new activities starting from a predefined prototype.

This is an optional feature that improves extensibility and can be useful for repeated tasks.

## Categories

### Custom category model

The project includes a `Category` model and a `CategoryManager`.

Categories are not only plain strings inside activities: they can be managed separately and can include additional information such as color.

### Category propagation

The activity manager includes logic to update or clear categories across existing activities.

This allows category changes to be reflected consistently in the agenda.

## Recurrence

### Recurrence model

The project includes a `RecurrenceRule` model.

Supported recurrence frequencies include:

* daily
* weekly
* monthly
* yearly

Supported end modes include:

* never
* until a specific date
* after a number of occurrences

Although the GUI controls for recurrence are still being developed, the model and persistence layers already support recurring activities.

## Persistence

### JSON serialization and deserialization

The application supports saving and loading activities from local JSON files.

The JSON persistence layer reconstructs the correct concrete activity subclass when loading data.

### Safe loading strategy

The JSON loading process reconstructs activities before replacing the current agenda.

This reduces the risk of losing the current state if the selected JSON file is invalid.

### File dialog integration

Save and load operations are integrated into the GUI through file dialogs.

The application does not rely on hardcoded file paths.

## GUI and user experience

### Single main window workflow

The application is based on a single main window.

Temporary modal dialogs are used only for focused actions such as creating or editing an activity.

This keeps navigation simple and avoids opening many unrelated top-level windows.

### Unsaved changes tracking

The GUI tracks unsaved changes.

When the agenda has been modified, the window title shows an unsaved marker and the user is warned before closing or loading another file.

This reduces the risk of accidental data loss.

### Automatic selection after creation and editing

After creating or editing an activity, the application automatically selects the affected activity in the list.

This gives immediate visual feedback and makes the workflow smoother.

### Improved widget styling

The project includes custom QSS styling to improve readability and consistency.

Specific attention was given to:

* date/time fields
* combo boxes
* spin boxes
* checklist checkboxes
* button states
* selected list items

This also helps the application look less like a default Qt prototype.

## Planned enhancements to mention if completed later

The following features are planned or partially prepared, but should only be listed as completed if they are actually implemented before submission:

* recurrence controls in the GUI
* template creation and usage from the GUI
* category management from the GUI
* undo/redo commands
* CSV import/export
* final modern GUI polish with sidebar, icons and dashboard-like layout
* Docker validation with the professor's environment

## Type-aware creation and editing dialogs

### Type-specific activity fields in the GUI

The GUI does not use a single generic form for every activity.

Creation and editing dialogs show fields that depend on the concrete activity type:

- events expose start time, end time, location and participants
- deadlines expose due date, context and hard deadline flag
- reminders expose reminder time, advance notice and note
- checklists expose target date and editable checklist items

This makes the GUI consistent with the object-oriented model and makes the differences between concrete subclasses visible to the user.

### Checklist editing with selectable items

Checklist items can be edited through GUI list items with checkboxes.

This is more user-friendly than requiring textual markers such as `[x]` or `[ ]`, and it maps directly to the boolean completion state stored in each checklist item.

## Recurrence controls in the GUI

### Editable recurrence rules

The GUI allows the user to create and edit recurrence rules for activities.

Supported options include:

- repeat frequency
- repeat interval
- end condition
- end date
- maximum number of occurrences

This makes the existing `RecurrenceRule` model directly usable from the interface.

### Conditional recurrence fields

The recurrence section shows only the fields that are relevant to the selected end condition.

For example:

- if the recurrence never ends, no extra end field is shown
- if it ends at a date, the date field is shown
- if it ends after a number of occurrences, the occurrences field is shown

This improves usability and avoids exposing irrelevant inputs at the same time.

### Persistence of recurring activities

Recurring activities can be saved to JSON and loaded again while preserving their recurrence settings.

This shows that recurrence is supported consistently across the model, GUI and persistence layers.

## Template-based activity creation

### Reusable activity templates

The GUI allows activities to be created from reusable templates.

A template stores a prototype activity, and the application can create a new independent activity from that prototype.

This is useful for repeated activity structures, such as exam deadlines, study checklists or recurring planning tasks.

### Save selected activity as template

The user can select an existing activity and save it as a template.

This makes the feature flexible because templates do not need to be hardcoded: they can be created from real activities already present in the agenda.

### Polymorphic template cloning

Template-based creation relies on polymorphic cloning.

The GUI does not need to know how to manually duplicate every concrete activity type. Instead, the model creates a new activity from the stored prototype.

This keeps template logic closer to the model and avoids duplicating type-specific construction logic inside the GUI.

### Current limitation

Templates are currently stored only during the active application session.

Persistence of templates in JSON is tracked as a separate future issue.

## Input validation and user feedback

### Form validation

The application validates user input before creating or editing activities.

Validation includes:

- non-empty activity titles
- event end time after start time
- checklist activities with at least one item
- recurrence end date after the activity primary date

This makes the application more robust and prevents invalid model states from being saved.

### Checklist safety checks

Checklist editing includes additional safeguards.

The user cannot remove the final remaining checklist item from the edit dialog. This prevents the checklist from temporarily reaching an invalid empty state.

This improves the user experience because the interface blocks the invalid action immediately instead of only showing an error later when saving.

### User feedback

The GUI provides feedback through message boxes and status bar messages.

Examples include:

- invalid activity warnings
- invalid checklist warnings
- invalid recurrence warnings
- save/load feedback
- unsaved changes feedback
- delete confirmations

This makes the application easier to understand and reduces the risk of accidental data loss.

## Undo/redo command system

### Command-based undo/redo

The application supports undo and redo through a command-based architecture.

User actions such as creating, deleting, editing and toggling completion are represented as command objects.

This makes the undo/redo system extensible because new reversible actions can be added by implementing new command classes.

### Supported undoable actions

The undo/redo system currently supports:

- adding activities
- creating activities from templates
- removing activities
- editing activities
- toggling activity completion

### GUI integration

Undo and redo are available directly from the GUI through:

- dedicated buttons
- Edit menu actions
- keyboard shortcuts

The interface enables or disables undo/redo controls depending on the current command history state.

### Safe history handling

The command history is cleared when a new agenda file is loaded.

This prevents undo/redo operations from being applied to activities that may no longer exist in the newly loaded agenda.

### Design value

The undo/redo system is separated from `MainWindow`.

`MainWindow` coordinates user interaction, while `CommandHistory` and the concrete command classes manage reversible operations.

This improves separation of responsibilities and makes the project architecture more robust.

## Persistent activity templates

### Template persistence in JSON

Activity templates are persisted in the same JSON file as the agenda activities.

Each template stores:

- template id
- template name
- prototype activity

This allows user-created templates to remain available after closing and reopening the application.

### Prototype serialization

Template prototypes are saved using the existing activity JSON serializer.

This means that template prototypes preserve the same type-specific data as normal activities, including:

- event fields
- deadline fields
- reminder fields
- checklist items
- priority
- category
- recurrence settings

### Backward-compatible loading

The `templates` array is optional when loading an agenda file.

This keeps older JSON files valid even if they were created before template persistence was added.

### Safe loading

Activities and templates are reconstructed and validated before replacing the current application state.

This reduces the risk of corrupting the current agenda when loading an invalid or incomplete JSON file.

## Advanced filtering and sorting GUI

### Main window filters

The main window exposes several filtering options directly in the GUI:

- activity type
- priority
- category
- completion status
- recurrence state
- overdue/due state

These filters allow the user to narrow the activity list without changing the underlying data.

### Sorting controls

The activity list can be sorted by different criteria, including date, title, priority, completion state and creation/update time.

Sorting is integrated with the existing filtering and search workflow.

### Collapsible filter panel

Filters are placed inside a collapsible panel.

This keeps the interface cleaner because filters are available when needed but do not permanently reduce the space available for the activity list.

### Scrollable left panel

The left control panel is scrollable.

This improves usability on smaller screens and prevents controls from becoming compressed when the window is resized.

### Layout refinement

The main window layout was adjusted to make the activity list the central navigation area.

The details panel remains available on the right, while the action buttons are grouped more clearly below the list.
