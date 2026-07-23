// Converts polymorphic activities to and from the JSON representation.

#ifndef ACTIVITYJSONSERIALIZER_H
#define ACTIVITYJSONSERIALIZER_H

#include "model/Activity.h"
#include "model/RecurrenceRule.h"

#include <QJsonObject>
#include <QString>

#include <memory>

class ActivityJsonSerializationVisitor;

class ActivityJsonSerializer
{
public:
    static QJsonObject toJson(const Activity& activity);
    static std::unique_ptr<Activity> fromJson(const QJsonObject& json,
                                              QString* errorMessage = nullptr);

private:
    friend class ActivityJsonSerializationVisitor;

    static QJsonObject commonFieldsToJson(const Activity& activity,
                                          const QString& typeName);
    static QJsonObject recurrenceToJson(const RecurrenceRule& recurrenceRule);

    static QString priorityToJsonString(Priority priority);
    static QString recurrenceFrequencyToJsonString(RecurrenceRule::Frequency frequency);
    static QString recurrenceEndModeToJsonString(RecurrenceRule::EndMode endMode);
};

#endif
