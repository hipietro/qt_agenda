// Registry-based construction of concrete activities from persisted JSON data.

#ifndef ACTIVITYFACTORYREGISTRY_H
#define ACTIVITYFACTORYREGISTRY_H

#include "model/Activity.h"

#include <QHash>
#include <QJsonObject>
#include <QString>

#include <functional>
#include <memory>

class ActivityFactoryRegistry
{
public:
    using Factory = std::function<std::unique_ptr<Activity>(const QJsonObject&, QString*)>;

    ActivityFactoryRegistry() = default;

    void registerFactory(const QString& typeIdentifier, Factory factory);
    bool contains(const QString& typeIdentifier) const;

    std::unique_ptr<Activity> create(const QJsonObject& json,
                                     QString* errorMessage = nullptr) const;

    static ActivityFactoryRegistry createDefault();

private:
    QHash<QString, Factory> m_factories;

    static QString normalizeTypeIdentifier(const QString& typeIdentifier);
};

#endif
