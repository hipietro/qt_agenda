#!/usr/bin/env python3
"""Apply the ActivityKind audit refactoring on the checked-out feature branch.

The helper uses guarded replacements so it fails instead of silently applying a
partial refactor. It removes itself after completing the source changes.
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def write(relative: str, content: str) -> None:
    path = ROOT / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def regex_once(text: str, pattern: str, replacement: str, label: str) -> str:
    updated, count = re.subn(pattern, replacement, text, count=1, flags=re.S)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one regex match, found {count}")
    return updated


def patch_main_window() -> None:
    path = "src/gui/MainWindow.cpp"
    text = read(path)

    for include in (
        '#include "model/ChecklistActivity.h"\n',
        '#include <QBrush>\n',
        '#include <QColor>\n',
        '#include <QFont>\n',
    ):
        text = text.replace(include, "")

    text = replace_once(
        text,
        """        if (currentRow < 0) {
            showActivityDetails(nullptr);
            updateActionButtons();
            return;
        }
""",
        """        if (currentRow < 0) {
            updateActionButtons();
            return;
        }
""",
        "MainWindow negative selection",
    )

    text = replace_once(
        text,
        """        if (!item) {
            showActivityDetails(nullptr);
            updateActionButtons();
            return;
        }

        const QString activityId = item->data(Qt::UserRole).toString();
        showActivityDetails(findActivityById(activityId));
        updateActionButtons();
""",
        """        if (!item) {
            updateActionButtons();
            return;
        }

        updateActionButtons();
""",
        "MainWindow selected detail rendering",
    )

    text = replace_once(
        text,
        """        QListWidgetItem* item = new QListWidgetItem(activityListItemText(activity));
        item->setData(Qt::UserRole, activity->id());
        item->setToolTip(activity->summary());
        item->setSizeHint(QSize(0, 68));
        applyActivityListItemVisualState(item, activity);
""",
        """        // The presentation controller renders the concrete card through ActivityVisitor.
        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, activity->id());
        item->setToolTip(activity->summary());
""",
        "MainWindow visitor list item",
    )

    text = regex_once(
        text,
        r"\nvoid MainWindow::showActivityDetails\(const Activity\* activity\)\n\{.*?\n\}\n\nvoid MainWindow::updateActionButtons",
        "\nvoid MainWindow::updateActionButtons",
        "remove legacy detail renderer",
    )

    text = regex_once(
        text,
        r"\nQString MainWindow::activityListItemText\(const Activity\* activity\) const\n\{.*?\n\}\n\nvoid MainWindow::updateCategoryFilterOptions",
        "\nvoid MainWindow::updateCategoryFilterOptions",
        "remove legacy list renderer",
    )

    text = regex_once(
        text,
        r"\nQString MainWindow::statusText\(const Activity\* activity\) const\n\{.*?\n\}\n\nvoid MainWindow::toggleSelectedActivityCompletion",
        "\nvoid MainWindow::toggleSelectedActivityCompletion",
        "remove legacy text helpers",
    )

    text = text.replace("    showActivityDetails(updatedActivity);\n", "")
    text = text.replace("    showActivityDetails(findActivityById(createdActivityId));\n", "")
    text = text.replace("    showActivityDetails(findActivityById(activityId));\n", "")

    text = replace_once(
        text,
        """        if (selectedKindValue >= 0) {
            criteria.kind = static_cast<ActivityKind>(selectedKindValue);
        }
""",
        """        // ActivityKind is retained here only as user-selected filter metadata.
        if (selectedKindValue >= 0) {
            criteria.kind = static_cast<ActivityKind>(selectedKindValue);
        }
""",
        "MainWindow filter rationale",
    )

    write(path, text)

    path = "src/gui/MainWindow.h"
    text = read(path)
    text = text.replace('#include "model/ActivityKind.h"\n', "")
    text = text.replace('#include "model/Priority.h"\n', "")
    for declaration in (
        "    void showActivityDetails(const Activity* activity);\n",
        "    QString activityListItemText(const Activity* activity) const;\n",
        "    void applyActivityListItemVisualState(QListWidgetItem* item, const Activity* activity) const;\n",
        "    QString statusText(const Activity* activity) const;\n",
        "    QString priorityText(Priority priority) const;\n",
        "    QString recurrenceText(const Activity* activity) const;\n",
    ):
        text = text.replace(declaration, "")
    write(path, text)


def patch_edit_dialog() -> None:
    path = "src/gui/ActivityEditDialog.h"
    text = read(path)
    text = text.replace('#include "model/ActivityKind.h"\n', "")
    text = text.replace("class CategoryManager;\n", "class CategoryManager;\nclass ActivityEditFormVisitor;\n")
    text = text.replace(
        "class ActivityEditDialog : public QDialog\n{\n",
        "class ActivityEditDialog : public QDialog\n{\n    friend class ActivityEditFormVisitor;\n",
    )
    text = replace_once(
        text,
        "    void populateChecklistItems(const Activity& activity);\n",
        "    void populateChecklistItems(const ChecklistActivity& activity);\n",
        "checklist populate signature",
    )
    text = replace_once(
        text,
        "    ActivityKind m_activityKind;\n",
        "    const Activity* m_originalActivity = nullptr;\n",
        "replace edit kind state",
    )
    write(path, text)

    path = "src/gui/ActivityEditDialog.cpp"
    text = read(path)
    text = replace_once(
        text,
        '#include "ActivityEditDialog.h"\n',
        '#include "ActivityEditDialog.h"\n\n#include "ActivityEditFormVisitor.h"\n',
        "include edit visitor",
    )
    text = replace_once(
        text,
        """      m_categoryManager(categoryManager),
      m_activityKind(activity.kind()),
      m_originalId(activity.id()),
""",
        """      m_categoryManager(categoryManager),
      m_originalActivity(&activity),
      m_originalId(activity.id()),
""",
        "edit constructor state",
    )

    populate = """void ActivityEditDialog::populateFromActivity(const Activity& activity)
{
    m_typeLabel->setText(activityKindToString(activity.kind()));
    m_titleEdit->setText(activity.title());
    m_descriptionEdit->setPlainText(activity.description());
    populateCategoryCombo(activity.category());

    const int priorityIndex = m_priorityCombo->findData(static_cast<int>(activity.priority()));
    m_priorityCombo->setCurrentIndex(priorityIndex >= 0 ? priorityIndex : 1);

    ActivityEditFormVisitor visitor(*this, ActivityEditFormVisitor::Operation::Populate);
    activity.accept(visitor);
    populateRecurrence(activity);
}
"""
    text = regex_once(
        text,
        r"void ActivityEditDialog::populateFromActivity\(const Activity& activity\)\n\{.*?\n\}\n\nbool ActivityEditDialog::validateForm",
        populate + "\nbool ActivityEditDialog::validateForm",
        "edit populate visitor dispatch",
    )

    validate = """bool ActivityEditDialog::validateForm() const
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Invalid activity", "The title cannot be empty.");
        return false;
    }

    if (!m_originalActivity) {
        QMessageBox::warning(nullptr, "Invalid activity", "The original activity is not available.");
        return false;
    }

    ActivityEditFormVisitor visitor(
        const_cast<ActivityEditDialog&>(*this),
        ActivityEditFormVisitor::Operation::Validate);
    m_originalActivity->accept(visitor);

    if (!visitor.isValid()) {
        QMessageBox::warning(nullptr, visitor.errorTitle(), visitor.errorMessage());
        return false;
    }

    if (m_repeatsCheck->isChecked() &&
        selectedRecurrenceEndMode() == RecurrenceRule::EndMode::UntilDate &&
        m_recurrenceUntilEdit->dateTime() <= visitor.primaryDate()) {
        QMessageBox::warning(
            nullptr,
            "Invalid recurrence",
            "The recurrence end date must be after the activity date."
        );
        return false;
    }

    return true;
}
"""
    text = regex_once(
        text,
        r"bool ActivityEditDialog::validateForm\(\) const\n\{.*?\n\}\n\nstd::unique_ptr<Activity> ActivityEditDialog::createUpdatedActivityFromForm",
        validate + "\nstd::unique_ptr<Activity> ActivityEditDialog::createUpdatedActivityFromForm",
        "edit validation visitor dispatch",
    )

    build = """std::unique_ptr<Activity> ActivityEditDialog::createUpdatedActivityFromForm() const
{
    if (!m_originalActivity) {
        return nullptr;
    }

    ActivityEditFormVisitor visitor(
        const_cast<ActivityEditDialog&>(*this),
        ActivityEditFormVisitor::Operation::Build);
    m_originalActivity->accept(visitor);

    std::unique_ptr<Activity> activity = visitor.takeActivity();
    if (!activity) {
        return nullptr;
    }

    if (const std::optional<RecurrenceRule> recurrence = recurrenceRuleFromForm();
        recurrence.has_value()) {
        activity->setRecurrenceRule(recurrence.value());
    }

    return activity;
}
"""
    text = regex_once(
        text,
        r"std::unique_ptr<Activity> ActivityEditDialog::createUpdatedActivityFromForm\(\) const\n\{.*?\n\}\n\nQString ActivityEditDialog::selectedCategoryText",
        build + "\nQString ActivityEditDialog::selectedCategoryText",
        "edit build visitor dispatch",
    )

    text = replace_once(
        text,
        """void ActivityEditDialog::populateChecklistItems(const Activity& activity)
{
    const ChecklistActivity& checklist = static_cast<const ChecklistActivity&>(activity);

    m_checklistItemsList->clear();
""",
        """void ActivityEditDialog::populateChecklistItems(const ChecklistActivity& activity)
{
    m_checklistItemsList->clear();
""",
        "checklist concrete overload",
    )
    text = text.replace("for (const ChecklistItem& checklistItem : checklist.items())", "for (const ChecklistItem& checklistItem : activity.items())")
    write(path, text)


def patch_creation_and_filter_comments() -> None:
    path = "src/gui/ActivityCreationDialog.cpp"
    text = read(path)
    text = replace_once(
        text,
        """        QDateTime primaryDate;

        switch (selectedActivityKind()) {
""",
        """        QDateTime primaryDate;

        // No Activity object exists yet: this selects the date field for the type
        // explicitly requested by the user in the creation form.
        switch (selectedActivityKind()) {
""",
        "creation validation rationale",
    )
    text = replace_once(
        text,
        """    std::unique_ptr<Activity> activity;

    switch (selectedActivityKind()) {
""",
        """    std::unique_ptr<Activity> activity;

    // Creation is the justified boundary where the user's selected type chooses
    // which concrete object must be constructed.
    switch (selectedActivityKind()) {
""",
        "creation factory rationale",
    )
    write(path, text)

    path = "src/model/ActivityFilter.cpp"
    text = read(path)
    text = replace_once(
        text,
        """    if (criteria.kind.has_value() && activity->kind() != criteria.kind.value()) {
""",
        """    // ActivityKind is used as filter data, not to dispatch activity behavior.
    if (criteria.kind.has_value() && activity->kind() != criteria.kind.value()) {
""",
        "filter rationale",
    )
    write(path, text)

    path = "src/model/ActivityKind.h"
    text = read(path)
    text = replace_once(
        text,
        "// Small enum used to identify the concrete activity family when needed by UI or JSON.\n",
        "// Descriptive classifier retained for filtering, creation choices and simple labels.\n// Type-specific runtime behavior must use ActivityVisitor instead.\n",
        "ActivityKind role comment",
    )
    write(path, text)

    path = "src/model/Activity.h"
    text = read(path)
    text = replace_once(
        text,
        "    virtual ActivityKind kind() const = 0;\n",
        "    // Descriptive classifier only; never use it to dispatch type-specific behavior.\n    virtual ActivityKind kind() const = 0;\n",
        "Activity kind contract comment",
    )
    write(path, text)


def add_visitor_files() -> None:
    header = r'''// Visitor that owns all concrete-type-specific edit form behavior.

#ifndef ACTIVITYEDITFORMVISITOR_H
#define ACTIVITYEDITFORMVISITOR_H

#include "model/Activity.h"
#include "model/ActivityVisitor.h"

#include <QDateTime>
#include <QString>

#include <memory>

class ActivityEditDialog;

class ActivityEditFormVisitor final : public ActivityVisitor
{
public:
    enum class Operation
    {
        Populate,
        Validate,
        Build
    };

    ActivityEditFormVisitor(ActivityEditDialog& dialog, Operation operation);

    bool isValid() const;
    QString errorTitle() const;
    QString errorMessage() const;
    QDateTime primaryDate() const;
    std::unique_ptr<Activity> takeActivity();

    void visit(const EventActivity& activity) override;
    void visit(const DeadlineActivity& activity) override;
    void visit(const ReminderActivity& activity) override;
    void visit(const ChecklistActivity& activity) override;

private:
    void setError(const QString& title, const QString& message);

    ActivityEditDialog& m_dialog;
    Operation m_operation;
    bool m_valid = true;
    QString m_errorTitle;
    QString m_errorMessage;
    QDateTime m_primaryDate;
    std::unique_ptr<Activity> m_activity;
};

#endif
'''

    source = r'''#include "ActivityEditFormVisitor.h"

#include "ActivityEditDialog.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QCheckBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTextEdit>

#include <utility>

ActivityEditFormVisitor::ActivityEditFormVisitor(ActivityEditDialog& dialog,
                                                 Operation operation)
    : m_dialog(dialog),
      m_operation(operation)
{
}

bool ActivityEditFormVisitor::isValid() const
{
    return m_valid;
}

QString ActivityEditFormVisitor::errorTitle() const
{
    return m_errorTitle;
}

QString ActivityEditFormVisitor::errorMessage() const
{
    return m_errorMessage;
}

QDateTime ActivityEditFormVisitor::primaryDate() const
{
    return m_primaryDate;
}

std::unique_ptr<Activity> ActivityEditFormVisitor::takeActivity()
{
    return std::move(m_activity);
}

void ActivityEditFormVisitor::visit(const EventActivity& activity)
{
    if (m_operation == Operation::Populate) {
        m_dialog.m_typeStack->setCurrentIndex(0);
        m_dialog.m_eventStartEdit->setDateTime(activity.startDateTime());
        m_dialog.m_eventEndEdit->setDateTime(activity.endDateTime());
        m_dialog.m_eventLocationEdit->setText(activity.location());
        m_dialog.m_eventParticipantsEdit->setText(activity.participants().join(", "));
        return;
    }

    m_primaryDate = m_dialog.m_eventStartEdit->dateTime();

    if (m_operation == Operation::Validate) {
        if (m_dialog.m_eventEndEdit->dateTime() <= m_primaryDate) {
            setError("Invalid event", "The event end time must be after the start time.");
        }
        return;
    }

    QStringList participants;
    const QStringList rawParticipants =
        m_dialog.m_eventParticipantsEdit->text().split(",", Qt::SkipEmptyParts);

    for (const QString& participant : rawParticipants) {
        const QString trimmedParticipant = participant.trimmed();
        if (!trimmedParticipant.isEmpty()) {
            participants.append(trimmedParticipant);
        }
    }

    m_activity = std::make_unique<EventActivity>(
        m_dialog.m_titleEdit->text().trimmed(),
        m_dialog.m_eventStartEdit->dateTime(),
        m_dialog.m_eventEndEdit->dateTime(),
        m_dialog.m_eventLocationEdit->text().trimmed(),
        participants,
        m_dialog.m_descriptionEdit->toPlainText().trimmed(),
        m_dialog.selectedCategoryText(),
        m_dialog.selectedPriority(),
        m_dialog.m_originalCompleted,
        m_dialog.m_originalId,
        m_dialog.m_originalCreatedAt,
        QDateTime::currentDateTime()
    );
}

void ActivityEditFormVisitor::visit(const DeadlineActivity& activity)
{
    if (m_operation == Operation::Populate) {
        m_dialog.m_typeStack->setCurrentIndex(1);
        m_dialog.m_deadlineDueEdit->setDateTime(activity.dueDate());
        m_dialog.m_deadlineContextEdit->setText(activity.context());
        m_dialog.m_deadlineHardCheck->setChecked(activity.isHardDeadline());
        return;
    }

    m_primaryDate = m_dialog.m_deadlineDueEdit->dateTime();
    if (m_operation == Operation::Validate) {
        return;
    }

    m_activity = std::make_unique<DeadlineActivity>(
        m_dialog.m_titleEdit->text().trimmed(),
        m_dialog.m_deadlineDueEdit->dateTime(),
        m_dialog.m_deadlineContextEdit->text().trimmed(),
        m_dialog.m_deadlineHardCheck->isChecked(),
        m_dialog.m_descriptionEdit->toPlainText().trimmed(),
        m_dialog.selectedCategoryText(),
        m_dialog.selectedPriority(),
        m_dialog.m_originalCompleted,
        m_dialog.m_originalId,
        m_dialog.m_originalCreatedAt,
        QDateTime::currentDateTime()
    );
}

void ActivityEditFormVisitor::visit(const ReminderActivity& activity)
{
    if (m_operation == Operation::Populate) {
        m_dialog.m_typeStack->setCurrentIndex(2);
        m_dialog.m_reminderDateEdit->setDateTime(activity.reminderDateTime());
        m_dialog.m_reminderAdvanceSpin->setValue(activity.advanceMinutes());
        m_dialog.m_reminderNoteEdit->setText(activity.reminderNote());
        return;
    }

    m_primaryDate = m_dialog.m_reminderDateEdit->dateTime();
    if (m_operation == Operation::Validate) {
        return;
    }

    m_activity = std::make_unique<ReminderActivity>(
        m_dialog.m_titleEdit->text().trimmed(),
        m_dialog.m_reminderDateEdit->dateTime(),
        m_dialog.m_reminderAdvanceSpin->value(),
        m_dialog.m_reminderNoteEdit->text().trimmed(),
        m_dialog.m_descriptionEdit->toPlainText().trimmed(),
        m_dialog.selectedCategoryText(),
        m_dialog.selectedPriority(),
        m_dialog.m_originalCompleted,
        m_dialog.m_originalId,
        m_dialog.m_originalCreatedAt,
        QDateTime::currentDateTime()
    );
}

void ActivityEditFormVisitor::visit(const ChecklistActivity& activity)
{
    if (m_operation == Operation::Populate) {
        m_dialog.m_typeStack->setCurrentIndex(3);
        m_dialog.m_checklistDueEdit->setDateTime(activity.dueDate());
        m_dialog.populateChecklistItems(activity);
        return;
    }

    m_primaryDate = m_dialog.m_checklistDueEdit->dateTime();

    if (m_operation == Operation::Validate) {
        if (m_dialog.checklistItemsFromList().isEmpty()) {
            setError("Invalid checklist", "A checklist must contain at least one item.");
        }
        return;
    }

    m_activity = std::make_unique<ChecklistActivity>(
        m_dialog.m_titleEdit->text().trimmed(),
        m_dialog.m_checklistDueEdit->dateTime(),
        m_dialog.checklistItemsFromList(),
        m_dialog.m_descriptionEdit->toPlainText().trimmed(),
        m_dialog.selectedCategoryText(),
        m_dialog.selectedPriority(),
        m_dialog.m_originalCompleted,
        m_dialog.m_originalId,
        m_dialog.m_originalCreatedAt,
        QDateTime::currentDateTime()
    );
}

void ActivityEditFormVisitor::setError(const QString& title, const QString& message)
{
    m_valid = false;
    m_errorTitle = title;
    m_errorMessage = message;
}
'''

    write("src/gui/ActivityEditFormVisitor.h", header)
    write("src/gui/ActivityEditFormVisitor.cpp", source)

    path = "agenda_qt.pro"
    text = read(path)
    text = replace_once(
        text,
        "    src/gui/ActivityEditDialog.cpp \\\n",
        "    src/gui/ActivityEditDialog.cpp \\\n    src/gui/ActivityEditFormVisitor.cpp \\\n",
        "qmake visitor source",
    )
    text = replace_once(
        text,
        "    src/gui/ActivityEditDialog.h \\\n",
        "    src/gui/ActivityEditDialog.h \\\n    src/gui/ActivityEditFormVisitor.h \\\n",
        "qmake visitor header",
    )
    write(path, text)


def main() -> None:
    patch_main_window()
    patch_edit_dialog()
    patch_creation_and_filter_comments()
    add_visitor_files()

    script_path = Path(__file__).resolve()
    script_path.unlink()
    print("Issue #45 source refactoring applied successfully.")
    print("The temporary patch helper removed itself; review with git diff.")


if __name__ == "__main__":
    main()
