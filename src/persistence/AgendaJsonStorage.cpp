#include "AgendaJsonStorage.h"

#include "ActivityJsonSerializer.h"
#include "model/ActivityManager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <memory>
#include <vector>

bool AgendaJsonStorage::saveToFile(const ActivityManager& manager,
                                   const QString& filePath,
                                   QString* errorMessage)
{
    if (filePath.trimmed().isEmpty()) {
        setError(errorMessage, "The file path is empty.");
        return false;
    }

    QJsonObject root;
    root["version"] = 1;

    QJsonArray activitiesArray;

    for (const Activity* activity : manager.activities()) {
        if (!activity) {
            continue;
        }

        activitiesArray.append(ActivityJsonSerializer::toJson(*activity));
    }

    root["activities"] = activitiesArray;

    const QJsonDocument document(root);

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(errorMessage, QString("Cannot open file for writing: %1").arg(file.errorString()));
        return false;
    }

    const qint64 bytesWritten = file.write(document.toJson(QJsonDocument::Indented));

    if (bytesWritten == -1) {
        setError(errorMessage, QString("Cannot write JSON data: %1").arg(file.errorString()));
        return false;
    }

    return true;
}

bool AgendaJsonStorage::loadFromFile(ActivityManager& manager,
                                     const QString& filePath,
                                     QString* errorMessage)
{
    if (filePath.trimmed().isEmpty()) {
        setError(errorMessage, "The file path is empty.");
        return false;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        setError(errorMessage, QString("Cannot open file for reading: %1").arg(file.errorString()));
        return false;
    }

    const QByteArray fileContent = file.readAll();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(fileContent, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        setError(errorMessage, QString("Invalid JSON file: %1").arg(parseError.errorString()));
        return false;
    }

    if (!document.isObject()) {
        setError(errorMessage, "Invalid agenda file: root JSON value must be an object.");
        return false;
    }

    const QJsonObject root = document.object();

    const int version = root["version"].toInt(-1);

    if (version != 1) {
        setError(errorMessage, QString("Unsupported agenda file version: %1").arg(version));
        return false;
    }

    if (!root.contains("activities") || !root["activities"].isArray()) {
        setError(errorMessage, "Invalid agenda file: missing activities array.");
        return false;
    }

    const QJsonArray activitiesArray = root["activities"].toArray();

    std::vector<std::unique_ptr<Activity>> loadedActivities;
    loadedActivities.reserve(static_cast<std::size_t>(activitiesArray.size()));

    for (const QJsonValue& value : activitiesArray) {
        if (!value.isObject()) {
            setError(errorMessage, "Invalid agenda file: every activity must be a JSON object.");
            return false;
        }

        std::unique_ptr<Activity> activity =
            ActivityJsonSerializer::fromJson(value.toObject());

        if (!activity) {
            setError(errorMessage, "Invalid agenda file: one activity could not be reconstructed.");
            return false;
        }

        loadedActivities.push_back(std::move(activity));
    }

    manager.clear();

    for (std::unique_ptr<Activity>& activity : loadedActivities) {
        if (!manager.addActivity(std::move(activity))) {
            setError(errorMessage, "Invalid agenda file: duplicated activity id found.");
            manager.clear();
            return false;
        }
    }

    return true;
}

void AgendaJsonStorage::setError(QString* errorMessage, const QString& message)
{
    if (errorMessage) {
        *errorMessage = message;
    }
}