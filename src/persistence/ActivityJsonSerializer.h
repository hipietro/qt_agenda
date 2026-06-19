#ifndef ACTIVITYJSONSERIALIZER_H
#define ACTIVITYJSONSERIALIZER_H

#include "model/Activity.h"
#include "model/RecurrenceRule.h"

#include <QJsonObject>
#include <QString>

class ActivityJsonSerializer
{
public:
    static QJsonObject toJson(const Activity& activity);

private:
    static QJsonObject commonFieldsToJson(const Activity& activity);
    static QJsonObject recurrenceToJson(const RecurrenceRule& recurrenceRule);

    static QString priorityToJsonString(Priority priority);
    static QString activityKindToJsonString(ActivityKind kind);
    static QString recurrenceFrequencyToJsonString(RecurrenceRule::Frequency frequency);
    static QString recurrenceEndModeToJsonString(RecurrenceRule::EndMode endMode);
};

#endif