// Default JSON factories and validation for all concrete activity types.

#include "ActivityFactoryRegistry.h"

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/RecurrenceRule.h"
#include "model/ReminderActivity.h"

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>
#include <limits>
#include <optional>
#include <utility>

namespace {

struct CommonActivityFields
{
    QString title;
    QString description;
    QString category;
    Priority priority = Priority::Medium;
    bool completed = false;
    QString id;
    QDateTime createdAt;
    QDateTime updatedAt;
};

void setError(QString* errorMessage, const QString& message)
{
    if (errorMessage) {
        *errorMessage = message;
    }
}

bool readRequiredString(const QJsonObject& json,
                        const QString& fieldName,
                        QString& result,
                        QString* errorMessage,
                        bool allowEmpty = false)
{
    if (!json.contains(fieldName)) {
        setError(errorMessage,
                 QString("Missing required string field '%1'.").arg(fieldName));
        return false;
    }

    const QJsonValue value = json.value(fieldName);
    if (!value.isString()) {
        setError(errorMessage,
                 QString("Field '%1' must be a string.").arg(fieldName));
        return false;
    }

    result = value.toString();
    if (!allowEmpty && result.trimmed().isEmpty()) {
        setError(errorMessage,
                 QString("Field '%1' must not be empty.").arg(fieldName));
        return false;
    }

    return true;
}

bool readOptionalString(const QJsonObject& json,
                        const QString& fieldName,
                        const QString& defaultValue,
                        QString& result,
                        QString* errorMessage)
{
    if (!json.contains(fieldName)) {
        result = defaultValue;
        return true;
    }

    const QJsonValue value = json.value(fieldName);
    if (!value.isString()) {
        setError(errorMessage,
                 QString("Field '%1' must be a string.").arg(fieldName));
        return false;
    }

    result = value.toString();
    return true;
}

bool readOptionalBool(const QJsonObject& json,
                      const QString& fieldName,
                      bool defaultValue,
                      bool& result,
                      QString* errorMessage)
{
    if (!json.contains(fieldName)) {
        result = defaultValue;
        return true;
    }

    const QJsonValue value = json.value(fieldName);
    if (!value.isBool()) {
        setError(errorMessage,
                 QString("Field '%1' must be a boolean.").arg(fieldName));
        return false;
    }

    result = value.toBool();
    return true;
}

bool readOptionalInt(const QJsonObject& json,
                     const QString& fieldName,
                     int defaultValue,
                     int& result,
                     QString* errorMessage)
{
    if (!json.contains(fieldName)) {
        result = defaultValue;
        return true;
    }

    const QJsonValue value = json.value(fieldName);
    if (!value.isDouble()) {
        setError(errorMessage,
                 QString("Field '%1' must be an integer.").arg(fieldName));
        return false;
    }

    const double number = value.toDouble();
    if (std::floor(number) != number
        || number < static_cast<double>(std::numeric_limits<int>::min())
        || number > static_cast<double>(std::numeric_limits<int>::max())) {
        setError(errorMessage,
                 QString("Field '%1' must be an integer.").arg(fieldName));
        return false;
    }

    result = static_cast<int>(number);
    return true;
}

bool parseDateTimeValue(const QJsonValue& value,
                        const QString& fieldName,
                        QDateTime& result,
                        QString* errorMessage)
{
    if (!value.isString()) {
        setError(errorMessage,
                 QString("Field '%1' must be an ISO date-time string.").arg(fieldName));
        return false;
    }

    const QString text = value.toString();
    result = QDateTime::fromString(text, Qt::ISODate);

    if (!result.isValid()) {
        setError(errorMessage,
                 QString("Field '%1' contains an invalid ISO date-time.").arg(fieldName));
        return false;
    }

    return true;
}

bool readRequiredDateTime(const QJsonObject& json,
                          const QString& fieldName,
                          QDateTime& result,
                          QString* errorMessage)
{
    if (!json.contains(fieldName)) {
        setError(errorMessage,
                 QString("Missing required date-time field '%1'.").arg(fieldName));
        return false;
    }

    return parseDateTimeValue(json.value(fieldName), fieldName, result, errorMessage);
}

bool readOptionalDateTime(const QJsonObject& json,
                          const QString& fieldName,
                          const QDateTime& defaultValue,
                          QDateTime& result,
                          QString* errorMessage)
{
    if (!json.contains(fieldName)) {
        result = defaultValue;
        return true;
    }

    return parseDateTimeValue(json.value(fieldName), fieldName, result, errorMessage);
}

bool readPriority(const QJsonObject& json,
                  Priority& result,
                  QString* errorMessage)
{
    if (!json.contains("priority")) {
        result = Priority::Medium;
        return true;
    }

    QString value;
    if (!readRequiredString(json, "priority", value, errorMessage)) {
        return false;
    }

    const QString normalized = value.trimmed().toLower();
    if (normalized == "low") {
        result = Priority::Low;
        return true;
    }
    if (normalized == "medium") {
        result = Priority::Medium;
        return true;
    }
    if (normalized == "high") {
        result = Priority::High;
        return true;
    }
    if (normalized == "critical") {
        result = Priority::Critical;
        return true;
    }

    setError(errorMessage,
             QString("Field 'priority' contains unknown value '%1'.").arg(value));
    return false;
}

bool parseCommonFields(const QJsonObject& json,
                       CommonActivityFields& fields,
                       QString* errorMessage)
{
    const QDateTime now = QDateTime::currentDateTime();

    return readRequiredString(json, "title", fields.title, errorMessage)
        && readOptionalString(json, "description", QString(), fields.description, errorMessage)
        && readOptionalString(json, "category", QString(), fields.category, errorMessage)
        && readPriority(json, fields.priority, errorMessage)
        && readOptionalBool(json, "completed", false, fields.completed, errorMessage)
        && readOptionalString(json, "id", QString(), fields.id, errorMessage)
        && readOptionalDateTime(json, "createdAt", now, fields.createdAt, errorMessage)
        && readOptionalDateTime(json, "updatedAt", now, fields.updatedAt, errorMessage);
}

bool parseFrequency(const QString& value,
                    RecurrenceRule::Frequency& result,
                    QString* errorMessage)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == "daily") {
        result = RecurrenceRule::Frequency::Daily;
        return true;
    }
    if (normalized == "weekly") {
        result = RecurrenceRule::Frequency::Weekly;
        return true;
    }
    if (normalized == "monthly") {
        result = RecurrenceRule::Frequency::Monthly;
        return true;
    }
    if (normalized == "yearly") {
        result = RecurrenceRule::Frequency::Yearly;
        return true;
    }

    setError(errorMessage,
             QString("Recurrence field 'frequency' contains unknown value '%1'.").arg(value));
    return false;
}

bool parseEndMode(const QString& value,
                  RecurrenceRule::EndMode& result,
                  QString* errorMessage)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == "never") {
        result = RecurrenceRule::EndMode::Never;
        return true;
    }
    if (normalized == "until_date") {
        result = RecurrenceRule::EndMode::UntilDate;
        return true;
    }
    if (normalized == "after_occurrences") {
        result = RecurrenceRule::EndMode::AfterOccurrences;
        return true;
    }

    setError(errorMessage,
             QString("Recurrence field 'endMode' contains unknown value '%1'.").arg(value));
    return false;
}

bool parseRecurrence(const QJsonObject& activityJson,
                     std::optional<RecurrenceRule>& result,
                     QString* errorMessage)
{
    result.reset();

    if (!activityJson.contains("recurrence")) {
        return true;
    }

    const QJsonValue recurrenceValue = activityJson.value("recurrence");
    if (!recurrenceValue.isObject()) {
        setError(errorMessage, "Field 'recurrence' must be a JSON object.");
        return false;
    }

    const QJsonObject json = recurrenceValue.toObject();

    QString frequencyText;
    QString endModeText;
    if (!readRequiredString(json, "frequency", frequencyText, errorMessage)
        || !readRequiredString(json, "endMode", endModeText, errorMessage)) {
        return false;
    }

    RecurrenceRule::Frequency frequency;
    RecurrenceRule::EndMode endMode;
    if (!parseFrequency(frequencyText, frequency, errorMessage)
        || !parseEndMode(endModeText, endMode, errorMessage)) {
        return false;
    }

    int interval = 1;
    int maxOccurrences = 1;
    if (!readOptionalInt(json, "interval", 1, interval, errorMessage)
        || !readOptionalInt(json, "maxOccurrences", 1, maxOccurrences, errorMessage)) {
        return false;
    }

    if (interval < 1) {
        setError(errorMessage, "Recurrence field 'interval' must be at least 1.");
        return false;
    }

    if (endMode == RecurrenceRule::EndMode::AfterOccurrences && maxOccurrences < 1) {
        setError(errorMessage,
                 "Recurrence field 'maxOccurrences' must be at least 1.");
        return false;
    }

    QDateTime untilDate;
    if (endMode == RecurrenceRule::EndMode::UntilDate) {
        if (!readRequiredDateTime(json, "untilDate", untilDate, errorMessage)) {
            return false;
        }
    } else if (json.contains("untilDate")
               && !parseDateTimeValue(json.value("untilDate"),
                                      "untilDate",
                                      untilDate,
                                      errorMessage)) {
        return false;
    }

    const RecurrenceRule recurrence(
        frequency,
        interval,
        endMode,
        untilDate,
        maxOccurrences
    );

    if (!recurrence.isValid()) {
        setError(errorMessage, "The recurrence rule is not valid.");
        return false;
    }

    result = recurrence;
    return true;
}

std::unique_ptr<Activity> createEventActivity(const QJsonObject& json,
                                              QString* errorMessage)
{
    CommonActivityFields common;
    if (!parseCommonFields(json, common, errorMessage)) {
        return nullptr;
    }

    QDateTime startDateTime;
    QDateTime endDateTime;
    if (!readRequiredDateTime(json, "startDateTime", startDateTime, errorMessage)
        || !readRequiredDateTime(json, "endDateTime", endDateTime, errorMessage)) {
        return nullptr;
    }

    if (endDateTime < startDateTime) {
        setError(errorMessage,
                 "Field 'endDateTime' must not be earlier than 'startDateTime'.");
        return nullptr;
    }

    QString location;
    if (!readOptionalString(json, "location", QString(), location, errorMessage)) {
        return nullptr;
    }

    QStringList participants;
    if (json.contains("participants")) {
        const QJsonValue participantsValue = json.value("participants");
        if (!participantsValue.isArray()) {
            setError(errorMessage, "Field 'participants' must be an array of strings.");
            return nullptr;
        }

        const QJsonArray participantsArray = participantsValue.toArray();
        for (const QJsonValue& participantValue : participantsArray) {
            if (!participantValue.isString()) {
                setError(errorMessage,
                         "Every value in 'participants' must be a string.");
                return nullptr;
            }
            participants.append(participantValue.toString());
        }
    }

    return std::make_unique<EventActivity>(
        common.title,
        startDateTime,
        endDateTime,
        location,
        participants,
        common.description,
        common.category,
        common.priority,
        common.completed,
        common.id,
        common.createdAt,
        common.updatedAt
    );
}

std::unique_ptr<Activity> createDeadlineActivity(const QJsonObject& json,
                                                 QString* errorMessage)
{
    CommonActivityFields common;
    if (!parseCommonFields(json, common, errorMessage)) {
        return nullptr;
    }

    QDateTime dueDate;
    if (!readRequiredDateTime(json, "dueDate", dueDate, errorMessage)) {
        return nullptr;
    }

    QString context;
    bool hardDeadline = false;
    if (!readOptionalString(json, "context", QString(), context, errorMessage)
        || !readOptionalBool(json, "hardDeadline", false, hardDeadline, errorMessage)) {
        return nullptr;
    }

    return std::make_unique<DeadlineActivity>(
        common.title,
        dueDate,
        context,
        hardDeadline,
        common.description,
        common.category,
        common.priority,
        common.completed,
        common.id,
        common.createdAt,
        common.updatedAt
    );
}

std::unique_ptr<Activity> createReminderActivity(const QJsonObject& json,
                                                 QString* errorMessage)
{
    CommonActivityFields common;
    if (!parseCommonFields(json, common, errorMessage)) {
        return nullptr;
    }

    QDateTime reminderDateTime;
    if (!readRequiredDateTime(json,
                              "reminderDateTime",
                              reminderDateTime,
                              errorMessage)) {
        return nullptr;
    }

    int advanceMinutes = 0;
    QString reminderNote;
    if (!readOptionalInt(json, "advanceMinutes", 0, advanceMinutes, errorMessage)
        || !readOptionalString(json,
                               "reminderNote",
                               QString(),
                               reminderNote,
                               errorMessage)) {
        return nullptr;
    }

    if (advanceMinutes < 0) {
        setError(errorMessage, "Field 'advanceMinutes' must not be negative.");
        return nullptr;
    }

    return std::make_unique<ReminderActivity>(
        common.title,
        reminderDateTime,
        advanceMinutes,
        reminderNote,
        common.description,
        common.category,
        common.priority,
        common.completed,
        common.id,
        common.createdAt,
        common.updatedAt
    );
}

std::unique_ptr<Activity> createChecklistActivity(const QJsonObject& json,
                                                  QString* errorMessage)
{
    CommonActivityFields common;
    if (!parseCommonFields(json, common, errorMessage)) {
        return nullptr;
    }

    QDateTime targetDate;
    if (!readRequiredDateTime(json, "targetDate", targetDate, errorMessage)) {
        return nullptr;
    }

    QVector<ChecklistItem> items;
    if (json.contains("items")) {
        const QJsonValue itemsValue = json.value("items");
        if (!itemsValue.isArray()) {
            setError(errorMessage, "Field 'items' must be an array.");
            return nullptr;
        }

        const QJsonArray itemsArray = itemsValue.toArray();
        for (const QJsonValue& itemValue : itemsArray) {
            if (!itemValue.isObject()) {
                setError(errorMessage,
                         "Every value in 'items' must be a JSON object.");
                return nullptr;
            }

            const QJsonObject itemJson = itemValue.toObject();
            ChecklistItem item;
            if (!readRequiredString(itemJson, "text", item.text, errorMessage)
                || !readOptionalBool(itemJson,
                                     "completed",
                                     false,
                                     item.completed,
                                     errorMessage)) {
                return nullptr;
            }

            items.append(item);
        }
    }

    return std::make_unique<ChecklistActivity>(
        common.title,
        targetDate,
        items,
        common.description,
        common.category,
        common.priority,
        common.completed,
        common.id,
        common.createdAt,
        common.updatedAt
    );
}

} // namespace

void ActivityFactoryRegistry::registerFactory(const QString& typeIdentifier,
                                              Factory factory)
{
    const QString normalized = normalizeTypeIdentifier(typeIdentifier);
    if (normalized.isEmpty() || !factory) {
        return;
    }

    m_factories.insert(normalized, std::move(factory));
}

bool ActivityFactoryRegistry::contains(const QString& typeIdentifier) const
{
    return m_factories.contains(normalizeTypeIdentifier(typeIdentifier));
}

std::unique_ptr<Activity> ActivityFactoryRegistry::create(const QJsonObject& json,
                                                          QString* errorMessage) const
{
    if (errorMessage) {
        errorMessage->clear();
    }

    QString typeIdentifier;
    if (!readRequiredString(json, "type", typeIdentifier, errorMessage)) {
        return nullptr;
    }

    const QString normalized = normalizeTypeIdentifier(typeIdentifier);
    const auto factoryIt = m_factories.constFind(normalized);
    if (factoryIt == m_factories.constEnd()) {
        setError(errorMessage,
                 QString("Unknown activity type '%1'.").arg(typeIdentifier));
        return nullptr;
    }

    std::unique_ptr<Activity> activity = factoryIt.value()(json, errorMessage);
    if (!activity) {
        if (errorMessage && errorMessage->isEmpty()) {
            setError(errorMessage,
                     QString("Factory for activity type '%1' failed.").arg(typeIdentifier));
        }
        return nullptr;
    }

    std::optional<RecurrenceRule> recurrence;
    if (!parseRecurrence(json, recurrence, errorMessage)) {
        return nullptr;
    }

    activity->m_recurrenceRule = recurrence;
    return activity;
}

ActivityFactoryRegistry ActivityFactoryRegistry::createDefault()
{
    ActivityFactoryRegistry registry;
    registry.registerFactory("event", createEventActivity);
    registry.registerFactory("deadline", createDeadlineActivity);
    registry.registerFactory("reminder", createReminderActivity);
    registry.registerFactory("checklist", createChecklistActivity);
    return registry;
}

QString ActivityFactoryRegistry::normalizeTypeIdentifier(const QString& typeIdentifier)
{
    return typeIdentifier.trimmed().toLower();
}
