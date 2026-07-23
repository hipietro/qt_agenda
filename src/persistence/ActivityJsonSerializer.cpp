// JSON serializer for all concrete activity types.

#include "ActivityJsonSerializer.h"

#include "ActivityFactoryRegistry.h"
#include "ActivityJsonSerializationVisitor.h"

QJsonObject ActivityJsonSerializer::toJson(const Activity& activity)
{
    ActivityJsonSerializationVisitor visitor;
    activity.accept(visitor);
    return visitor.json();
}

std::unique_ptr<Activity> ActivityJsonSerializer::fromJson(const QJsonObject& json,
                                                           QString* errorMessage)
{
    static const ActivityFactoryRegistry registry =
        ActivityFactoryRegistry::createDefault();

    return registry.create(json, errorMessage);
}

QJsonObject ActivityJsonSerializer::commonFieldsToJson(const Activity& activity,
                                                        const QString& typeName)
{
    QJsonObject json;

    json["id"] = activity.id();
    json["type"] = typeName;
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

QString ActivityJsonSerializer::recurrenceFrequencyToJsonString(
    RecurrenceRule::Frequency frequency)
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

QString ActivityJsonSerializer::recurrenceEndModeToJsonString(
    RecurrenceRule::EndMode endMode)
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
