// JSON serializer for all concrete activity types.

#include "ActivityJsonSerializer.h"

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QJsonArray>
#include <QJsonValue>

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

std::unique_ptr<Activity> ActivityJsonSerializer::fromJson(const QJsonObject& json)
{
    const std::optional<ActivityKind> kind =
        activityKindFromJsonString(json["type"].toString());

    if (!kind.has_value()) {
        return nullptr;
    }

    const QString id = json["id"].toString();
    const QString title = json["title"].toString();
    const QString description = json["description"].toString();
    const QString category = json["category"].toString();
    const Priority priority = priorityFromJsonString(json["priority"].toString());
    const bool completed = json["completed"].toBool(false);

    const QDateTime createdAt = dateTimeFromJsonString(json["createdAt"].toString());
    const QDateTime updatedAt = dateTimeFromJsonString(json["updatedAt"].toString());

    std::unique_ptr<Activity> activity;

    switch (kind.value()) {
    case ActivityKind::Event: {
        const QDateTime startDateTime = dateTimeFromJsonString(json["startDateTime"].toString());
        const QDateTime endDateTime = dateTimeFromJsonString(json["endDateTime"].toString());
        const QString location = json["location"].toString();

        QStringList participants;

        const QJsonArray participantsArray = json["participants"].toArray();

        for (const QJsonValue& value : participantsArray) {
            participants.append(value.toString());
        }

        activity = std::make_unique<EventActivity>(
            title,
            startDateTime,
            endDateTime,
            location,
            participants,
            description,
            category,
            priority,
            completed,
            id,
            createdAt,
            updatedAt
        );

        break;
    }

    case ActivityKind::Deadline: {
        const QDateTime dueDate = dateTimeFromJsonString(json["dueDate"].toString());
        const QString context = json["context"].toString();
        const bool hardDeadline = json["hardDeadline"].toBool(false);

        activity = std::make_unique<DeadlineActivity>(
            title,
            dueDate,
            context,
            hardDeadline,
            description,
            category,
            priority,
            completed,
            id,
            createdAt,
            updatedAt
        );

        break;
    }

    case ActivityKind::Reminder: {
        const QDateTime reminderDateTime = dateTimeFromJsonString(json["reminderDateTime"].toString());
        const int advanceMinutes = json["advanceMinutes"].toInt(0);
        const QString reminderNote = json["reminderNote"].toString();

        activity = std::make_unique<ReminderActivity>(
            title,
            reminderDateTime,
            advanceMinutes,
            reminderNote,
            description,
            category,
            priority,
            completed,
            id,
            createdAt,
            updatedAt
        );

        break;
    }

    case ActivityKind::Checklist: {
        const QDateTime targetDate = dateTimeFromJsonString(json["targetDate"].toString());

        QVector<ChecklistItem> items;

        const QJsonArray itemsArray = json["items"].toArray();

        for (const QJsonValue& value : itemsArray) {
            const QJsonObject itemObject = value.toObject();

            ChecklistItem item;
            item.text = itemObject["text"].toString();
            item.completed = itemObject["completed"].toBool(false);

            items.append(item);
        }

        activity = std::make_unique<ChecklistActivity>(
            title,
            targetDate,
            items,
            description,
            category,
            priority,
            completed,
            id,
            createdAt,
            updatedAt
        );

        break;
    }
    }

    if (!activity) {
        return nullptr;
    }

    if (json.contains("recurrence") && json["recurrence"].isObject()) {
        const std::optional<RecurrenceRule> recurrence =
            recurrenceFromJson(json["recurrence"].toObject());

        if (recurrence.has_value()) {
            activity->setRecurrenceRule(recurrence.value());
        }
    }

    return activity;
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

Priority ActivityJsonSerializer::priorityFromJsonString(const QString& value)
{
    const QString normalized = value.trimmed().toLower();

    if (normalized == "low") {
        return Priority::Low;
    }

    if (normalized == "high") {
        return Priority::High;
    }

    if (normalized == "critical") {
        return Priority::Critical;
    }

    return Priority::Medium;
}

std::optional<ActivityKind> ActivityJsonSerializer::activityKindFromJsonString(const QString& value)
{
    const QString normalized = value.trimmed().toLower();

    if (normalized == "event") {
        return ActivityKind::Event;
    }

    if (normalized == "deadline") {
        return ActivityKind::Deadline;
    }

    if (normalized == "reminder") {
        return ActivityKind::Reminder;
    }

    if (normalized == "checklist") {
        return ActivityKind::Checklist;
    }

    return std::nullopt;
}

RecurrenceRule::Frequency ActivityJsonSerializer::recurrenceFrequencyFromJsonString(const QString& value)
{
    const QString normalized = value.trimmed().toLower();

    if (normalized == "daily") {
        return RecurrenceRule::Frequency::Daily;
    }

    if (normalized == "monthly") {
        return RecurrenceRule::Frequency::Monthly;
    }

    if (normalized == "yearly") {
        return RecurrenceRule::Frequency::Yearly;
    }

    return RecurrenceRule::Frequency::Weekly;
}

RecurrenceRule::EndMode ActivityJsonSerializer::recurrenceEndModeFromJsonString(const QString& value)
{
    const QString normalized = value.trimmed().toLower();

    if (normalized == "never") {
        return RecurrenceRule::EndMode::Never;
    }

    if (normalized == "until_date") {
        return RecurrenceRule::EndMode::UntilDate;
    }

    return RecurrenceRule::EndMode::AfterOccurrences;
}

std::optional<RecurrenceRule> ActivityJsonSerializer::recurrenceFromJson(const QJsonObject& json)
{
    const RecurrenceRule::Frequency frequency =
        recurrenceFrequencyFromJsonString(json["frequency"].toString());

    const int interval = json["interval"].toInt(1);

    const RecurrenceRule::EndMode endMode =
        recurrenceEndModeFromJsonString(json["endMode"].toString());

    const QDateTime untilDate = dateTimeFromJsonString(json["untilDate"].toString());
    const int maxOccurrences = json["maxOccurrences"].toInt(1);

    RecurrenceRule recurrenceRule(
        frequency,
        interval,
        endMode,
        untilDate,
        maxOccurrences
    );

    if (!recurrenceRule.isValid()) {
        return std::nullopt;
    }

    return recurrenceRule;
}

QDateTime ActivityJsonSerializer::dateTimeFromJsonString(const QString& value)
{
    if (value.trimmed().isEmpty()) {
        return QDateTime();
    }

    return QDateTime::fromString(value, Qt::ISODate);
}