# UML and architecture diagrams

These diagrams describe the corrected architecture used by the final report. They intentionally separate the logical model from Qt Widgets and show the non-trivial dynamic behavior introduced for the resubmission.

## Activity hierarchy

```mermaid
classDiagram
    class Activity {
        <<abstract>>
        -QString id
        -QString title
        -QString description
        -QString category
        -Priority priority
        -bool completed
        -QDateTime createdAt
        -QDateTime updatedAt
        -optional~RecurrenceRule~ recurrence
        +accept(ActivityVisitor&) *
        +primaryDate() QDateTime*
        +isOverdue(QDateTime) bool*
        +clone() unique_ptr~Activity~*
        +kind() ActivityKind*
    }

    class EventActivity {
        -QDateTime startDateTime
        -QDateTime endDateTime
        -QString location
        -QStringList participants
    }
    class DeadlineActivity {
        -QDateTime dueDate
        -QString context
        -bool hardDeadline
    }
    class ReminderActivity {
        -QDateTime reminderDateTime
        -int advanceMinutes
        -QString reminderNote
    }
    class ChecklistActivity {
        -QDateTime dueDate
        -QVector~ChecklistItem~ items
        +progressPercentage() double
        +isCompleted() bool
    }

    Activity <|-- EventActivity
    Activity <|-- DeadlineActivity
    Activity <|-- ReminderActivity
    Activity <|-- ChecklistActivity
```

`kind()` is a descriptive classifier. Existing-activity rendering, editing, and serialization are dispatched through `accept(...)`, not through a type switch.

## Visitor double dispatch

```mermaid
classDiagram
    class ActivityVisitor {
        <<interface>>
        +visit(EventActivity) *
        +visit(DeadlineActivity) *
        +visit(ReminderActivity) *
        +visit(ChecklistActivity) *
    }

    class ActivityListItemVisitor
    class ActivityDetailVisitor
    class ActivityEditFormVisitor
    class ActivityJsonSerializationVisitor

    ActivityVisitor <|.. ActivityListItemVisitor
    ActivityVisitor <|.. ActivityDetailVisitor
    ActivityVisitor <|.. ActivityEditFormVisitor
    ActivityVisitor <|.. ActivityJsonSerializationVisitor

    Activity --> ActivityVisitor : accept(visitor)
```

- `ActivityListItemVisitor` creates type-specific list cards.
- `ActivityDetailVisitor` creates different detail-page structures.
- `ActivityEditFormVisitor` populates, validates, and rebuilds each concrete edit form.
- `ActivityJsonSerializationVisitor` serializes different fields for each concrete type.

## Application layers

```mermaid
flowchart LR
    GUI[Qt Widgets GUI<br/>MainWindow and in-window pages]
    CMD[Command hierarchy<br/>Add / Remove / Update / Toggle]
    MODEL[Logical model<br/>activities, managers, search, filters]
    PERSIST[Persistence<br/>storage, serialization visitor, factory registry]
    JSON[(Local JSON file)]

    GUI -->|reversible actions| CMD
    CMD -->|execute / undo| MODEL
    GUI -->|queries and form data| MODEL
    GUI -->|Save / Load via QFileDialog| PERSIST
    PERSIST <--> MODEL
    PERSIST <--> JSON
```

## In-window creation and editing

```mermaid
flowchart TB
    MAIN[MainWindow]
    STACK[m_workspaceStack : QStackedWidget]
    DETAIL[activityDetailPage]
    CREATE[activityCreationPage : QWidget]
    EDIT[activityEditPage : QWidget]
    CATEGORY[CategoryManagementDialog<br/>secondary utility only]

    MAIN --> STACK
    STACK --> DETAIL
    STACK --> CREATE
    STACK --> EDIT
    MAIN -.-> CATEGORY

    DETAIL -->|Add activity| CREATE
    CREATE -->|Create or Cancel| DETAIL
    DETAIL -->|Edit or double click| EDIT
    EDIT -->|Save or Cancel| DETAIL
```

Creation and editing remain inside `MainWindow`. The remaining category dialog is a secondary management utility and is not part of the mandatory creation/editing workflow.

## Command hierarchy

```mermaid
classDiagram
    class Command {
        <<interface>>
        +execute() bool*
        +undo() bool*
        +description() QString*
        +undoDescription() QString*
        +redoDescription() QString*
    }

    class AddActivityCommand
    class RemoveActivityCommand
    class UpdateActivityCommand
    class ToggleCompletionCommand
    class CommandHistory {
        -undoStack
        -redoStack
        +executeCommand(Command)
        +undo()
        +redo()
    }

    Command <|.. AddActivityCommand
    Command <|.. RemoveActivityCommand
    Command <|.. UpdateActivityCommand
    Command <|.. ToggleCompletionCommand
    CommandHistory o-- Command
```
