// Converts polymorphic activities to and from the JSON representation.

#ifndef ACTIVITYJSONSERIALIZER_H
#define ACTIVITYJSONSERIALIZER_H

#include "model/Activity.h"
#include "model/RecurrenceRule.h"

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include <memory>
#include <optional>

class ActivityJsonSerializer
{
public:
    static QJsonObject toJson(const Activity& activity);
    static std::unique_ptr<Activity> fromJson(const QJsonObject& json);

private:
    static QJsonObject commonFieldsToJson(const Activity& activity);
    static QJsonObject recurrenceToJson(const RecurrenceRule& recurrenceRule);

    static QString priorityToJsonString(Priority priority);
    static QString activityKindToJsonString(ActivityKind kind);
    static QString recurrenceFrequencyToJsonString(RecurrenceRule::Frequency frequency);
    static QString recurrenceEndModeToJsonString(RecurrenceRule::EndMode endMode);

    static Priority priorityFromJsonString(const QString& value);
    static std::optional<ActivityKind> activityKindFromJsonString(const QString& value);
    static RecurrenceRule::Frequency recurrenceFrequencyFromJsonString(const QString& value);
    static RecurrenceRule::EndMode recurrenceEndModeFromJsonString(const QString& value);

    static std::optional<RecurrenceRule> recurrenceFromJson(const QJsonObject& json);

    static QDateTime dateTimeFromJsonString(const QString& value);
};

#endif