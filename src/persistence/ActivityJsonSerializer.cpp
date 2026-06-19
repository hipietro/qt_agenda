#include "ActivityJsonSerializer.h"

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QJsonArray>

QJsonObject ActivityJsonSerializer::toJson(const Activity& activity)
{
    QJsonObject json = commonFieldsToJson(activity);

    switch (activity.kind()) {
    case ActivityKind::Event: {
        const EventActivity& event = static_cast<const EventActivity&>(activity);

        json["startDateTime"] = event.startDateTime().toString(Qt::ISODate);
        json["endDateTime"] = event.endDateTime().toString(Qt::ISODate);
        json["location"] = event.location();

        QJsonArray participantsArray;

        for (const QString& participant : event.participants()) {
            participantsArray.append(participant);
        }

        json["participants"] = participantsArray;
        break;
    }

    case ActivityKind::Deadline: {
        const DeadlineActivity& deadline = static_cast<const DeadlineActivity&>(activity);

        json["dueDate"] = deadline.dueDate().toString(Qt::ISODate);
        json["context"] = deadline.context();
        json["hardDeadline"] = deadline.isHardDeadline();
        break;
    }

    case ActivityKind::Reminder: {
        const ReminderActivity& reminder = static_cast<const ReminderActivity&>(activity);

        json["reminderDateTime"] = reminder.reminderDateTime().toString(Qt::ISODate);
        json["advanceMinutes"] = reminder.advanceMinutes();
        json["reminderNote"] = reminder.reminderNote();
        break;
    }

    case ActivityKind::Checklist: {
        const ChecklistActivity& checklist = static_cast<const ChecklistActivity&>(activity);

        json["targetDate"] = checklist.primaryDate().toString(Qt::ISODate); 
        QJsonArray itemsArray;

        for (const ChecklistItem& item : checklist.items()) {
            QJsonObject itemObject;
            itemObject["text"] = item.text;
            itemObject["completed"] = item.completed;
            itemsArray.append(itemObject);
        }

        json["items"] = itemsArray;
        break;
    }
    }

    return json;
}

QJsonObject ActivityJsonSerializer::commonFieldsToJson(const Activity& activity)
{
    QJsonObject json;

    json["id"] = activity.id();
    json["type"] = activityKindToJsonString(activity.kind());
    json["title"] = activity.title();
    json["description"] = activity.description();
    json["category"] = activity.category();
    json["priority"] = priorityToJsonString(activity.priority());
    json["completed"] = activity.isCompleted();
    json["createdAt"] = activity.createdAt().toString(Qt::ISODate);
    json["updatedAt"] = activity.updatedAt().toString(Qt::ISODate);

    if (activity.hasRecurrence()) {
        json["recurrence"] = recurrenceToJson(activity.recurrenceRule().value());
    }

    return json;
}

QJsonObject ActivityJsonSerializer::recurrenceToJson(const RecurrenceRule& recurrenceRule)
{
    QJsonObject json;

    json["frequency"] = recurrenceFrequencyToJsonString(recurrenceRule.frequency());
    json["interval"] = recurrenceRule.interval();
    json["endMode"] = recurrenceEndModeToJsonString(recurrenceRule.endMode());

    if (recurrenceRule.untilDate().isValid()) {
        json["untilDate"] = recurrenceRule.untilDate().toString(Qt::ISODate);
    }

    json["maxOccurrences"] = recurrenceRule.maxOccurrences();

    return json;
}

QString ActivityJsonSerializer::priorityToJsonString(Priority priority)
{
    switch (priority) {
    case Priority::Low:
        return "low";

    case Priority::Medium:
        return "medium";

    case Priority::High:
        return "high";

    case Priority::Critical:
        return "critical";
    }

    return "medium";
}

QString ActivityJsonSerializer::activityKindToJsonString(ActivityKind kind)
{
    switch (kind) {
    case ActivityKind::Event:
        return "event";

    case ActivityKind::Deadline:
        return "deadline";

    case ActivityKind::Reminder:
        return "reminder";

    case ActivityKind::Checklist:
        return "checklist";
    }

    return "activity";
}

QString ActivityJsonSerializer::recurrenceFrequencyToJsonString(RecurrenceRule::Frequency frequency)
{
    switch (frequency) {
    case RecurrenceRule::Frequency::Daily:
        return "daily";

    case RecurrenceRule::Frequency::Weekly:
        return "weekly";

    case RecurrenceRule::Frequency::Monthly:
        return "monthly";

    case RecurrenceRule::Frequency::Yearly:
        return "yearly";
    }

    return "weekly";
}

QString ActivityJsonSerializer::recurrenceEndModeToJsonString(RecurrenceRule::EndMode endMode)
{
    switch (endMode) {
    case RecurrenceRule::EndMode::Never:
        return "never";

    case RecurrenceRule::EndMode::UntilDate:
        return "until_date";

    case RecurrenceRule::EndMode::AfterOccurrences:
        return "after_occurrences";
    }

    return "after_occurrences";
}