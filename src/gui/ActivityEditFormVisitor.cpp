#include "ActivityEditFormVisitor.h"

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
