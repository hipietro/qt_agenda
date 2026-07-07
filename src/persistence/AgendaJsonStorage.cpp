#include "AgendaJsonStorage.h"

#include "ActivityJsonSerializer.h"
#include "model/Activity.h"
#include "model/ActivityManager.h"
#include "model/ActivityTemplate.h"
#include "model/ActivityTemplateManager.h"
#include "model/Category.h"
#include "model/CategoryManager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#include <memory>
#include <vector>

bool AgendaJsonStorage::saveToFile(const ActivityManager& manager,
                                   const ActivityTemplateManager& templateManager,
                                   const CategoryManager& categoryManager,
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

    /*
     * I template vengono salvati nello stesso file dell'agenda.
     * Ogni template conserva nome, id e attività prototipo.
     */
    QJsonArray templatesArray;

    for (const ActivityTemplate* activityTemplate : templateManager.templates()) {
        if (!activityTemplate || !activityTemplate->isValid() || !activityTemplate->hasPrototype()) {
            continue;
        }

        const Activity* prototype = activityTemplate->prototype();

        if (!prototype) {
            continue;
        }

        QJsonObject templateObject;
        templateObject["id"] = activityTemplate->id();
        templateObject["name"] = activityTemplate->name();
        templateObject["prototype"] = ActivityJsonSerializer::toJson(*prototype);

        templatesArray.append(templateObject);
    }

    root["templates"] = templatesArray;

    QJsonArray categoriesArray;

    for (const Category& category : categoryManager.categories()) {
        if (category.name().trimmed().isEmpty()) {
            continue;
        }

        QJsonObject categoryObject;
        categoryObject["id"] = category.id();
        categoryObject["name"] = category.name();
        categoryObject["colorHex"] = category.colorHex();

        categoriesArray.append(categoryObject);
    }

    root["categories"] = categoriesArray;

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
                                     ActivityTemplateManager& templateManager,
                                     CategoryManager& categoryManager,
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

    std::vector<ActivityTemplate> loadedTemplates;

    /*
     * Il campo templates è opzionale per mantenere compatibilità con i vecchi file JSON.
     * Se è presente, però, deve essere un array valido.
     */
    if (root.contains("templates")) {
        if (!root["templates"].isArray()) {
            setError(errorMessage, "Invalid agenda file: templates must be an array.");
            return false;
        }

        const QJsonArray templatesArray = root["templates"].toArray();
        loadedTemplates.reserve(static_cast<std::size_t>(templatesArray.size()));

        for (const QJsonValue& value : templatesArray) {
            if (!value.isObject()) {
                setError(errorMessage, "Invalid agenda file: every template must be a JSON object.");
                return false;
            }

            const QJsonObject templateObject = value.toObject();

            const QString templateId = templateObject["id"].toString();
            const QString templateName = templateObject["name"].toString();

            if (templateId.trimmed().isEmpty() || templateName.trimmed().isEmpty()) {
                setError(errorMessage, "Invalid agenda file: template id and name are required.");
                return false;
            }

            if (!templateObject.contains("prototype") || !templateObject["prototype"].isObject()) {
                setError(errorMessage, "Invalid agenda file: template prototype is missing.");
                return false;
            }

            std::unique_ptr<Activity> prototype =
                ActivityJsonSerializer::fromJson(templateObject["prototype"].toObject());

            if (!prototype) {
                setError(errorMessage, "Invalid agenda file: one template prototype could not be reconstructed.");
                return false;
            }

            ActivityTemplate activityTemplate(
                templateName,
                std::move(prototype),
                templateId
            );

            if (!activityTemplate.isValid()) {
                setError(errorMessage, "Invalid agenda file: one template is not valid.");
                return false;
            }

            loadedTemplates.push_back(std::move(activityTemplate));
        }
    }

    std::vector<Category> loadedCategories;

    if (root.contains("categories")) {
        if (!root["categories"].isArray()) {
            setError(errorMessage, "Invalid agenda file: categories must be an array.");
            return false;
        }

        const QJsonArray categoriesArray = root["categories"].toArray();
        loadedCategories.reserve(static_cast<std::size_t>(categoriesArray.size()));

        for (const QJsonValue& value : categoriesArray) {
            if (!value.isObject()) {
                setError(errorMessage, "Invalid agenda file: every category must be a JSON object.");
                return false;
            }

            const QJsonObject categoryObject = value.toObject();

            const QString categoryId = categoryObject["id"].toString();
            const QString categoryName = categoryObject["name"].toString();
            const QString colorHex = categoryObject["colorHex"].toString("#607D8B");

            if (categoryId.trimmed().isEmpty() || categoryName.trimmed().isEmpty()) {
                setError(errorMessage, "Invalid agenda file: category id and name are required.");
                return false;
            }

            loadedCategories.push_back(Category(categoryName, colorHex, categoryId));
        }
    }

    /*
     * Applico i dati caricati solo dopo aver validato tutto.
     * Così un file parzialmente invalido non corrompe lo stato corrente.
     */
    manager.clear();

    for (std::unique_ptr<Activity>& activity : loadedActivities) {
        if (!manager.addActivity(std::move(activity))) {
            setError(errorMessage, "Invalid agenda file: duplicated activity id found.");
            manager.clear();
            return false;
        }
    }

    templateManager.clear();

    for (ActivityTemplate& activityTemplate : loadedTemplates) {
        if (!templateManager.addTemplate(std::move(activityTemplate))) {
            setError(errorMessage, "Invalid agenda file: duplicated template id or name found.");
            manager.clear();
            templateManager.clear();
            return false;
        }
    }

    categoryManager.clear();

    for (const Category& category : loadedCategories) {
        if (!categoryManager.addCategory(category)) {
            setError(errorMessage, "Invalid agenda file: duplicated or invalid category found.");
            manager.clear();
            templateManager.clear();
            categoryManager.clear();
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