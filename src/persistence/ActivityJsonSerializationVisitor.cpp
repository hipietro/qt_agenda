// Serializes concrete activity fields after writing the shared JSON representation.

#include "ActivityJsonSerializationVisitor.h"

#include "ActivityJsonSerializer.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QJsonArray>

QJsonObject ActivityJsonSerializationVisitor::json() const
{
    return m_json;
}

void ActivityJsonSerializationVisitor::visit(const EventActivity& activity)
{
    // Events persist interval data and the participant collection.
    m_json = ActivityJsonSerializer::commonFieldsToJson(activity, "event");
    m_json["startDateTime"] = activity.startDateTime().toString(Qt::ISODateWithMs);
    m_json["endDateTime"] = activity.endDateTime().toString(Qt::ISODateWithMs);
    m_json["location"] = activity.location();

    QJsonArray participantsArray;
    for (const QString& participant : activity.participants()) {
        participantsArray.append(participant);
    }

    m_json["participants"] = participantsArray;
}

void ActivityJsonSerializationVisitor::visit(const DeadlineActivity& activity)
{
    // Deadline serialization keeps context and rigidity separate from common fields.
    m_json = ActivityJsonSerializer::commonFieldsToJson(activity, "deadline");
    m_json["dueDate"] = activity.dueDate().toString(Qt::ISODateWithMs);
    m_json["context"] = activity.context();
    m_json["hardDeadline"] = activity.isHardDeadline();
}

void ActivityJsonSerializationVisitor::visit(const ReminderActivity& activity)
{
    // Reminder serialization stores both schedule and advance-notice policy.
    m_json = ActivityJsonSerializer::commonFieldsToJson(activity, "reminder");
    m_json["reminderDateTime"] = activity.reminderDateTime().toString(Qt::ISODateWithMs);
    m_json["advanceMinutes"] = activity.advanceMinutes();
    m_json["reminderNote"] = activity.reminderNote();
}

void ActivityJsonSerializationVisitor::visit(const ChecklistActivity& activity)
{
    // Checklist items are nested objects so each child preserves its completion state.
    m_json = ActivityJsonSerializer::commonFieldsToJson(activity, "checklist");
    m_json["targetDate"] = activity.primaryDate().toString(Qt::ISODateWithMs);

    QJsonArray itemsArray;
    for (const ChecklistItem& item : activity.items()) {
        QJsonObject itemObject;
        itemObject["text"] = item.text;
        itemObject["completed"] = item.completed;
        itemsArray.append(itemObject);
    }

    m_json["items"] = itemsArray;
}
