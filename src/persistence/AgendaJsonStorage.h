#ifndef AGENDAJSONSTORAGE_H
#define AGENDAJSONSTORAGE_H

#include <QString>

class ActivityManager;
class ActivityTemplateManager;

class AgendaJsonStorage
{
public:
    static bool saveToFile(const ActivityManager& manager,
                           const ActivityTemplateManager& templateManager,
                           const QString& filePath,
                           QString* errorMessage = nullptr);

    static bool loadFromFile(ActivityManager& manager,
                             ActivityTemplateManager& templateManager,
                             const QString& filePath,
                             QString* errorMessage = nullptr);

private:
    static void setError(QString* errorMessage, const QString& message);
};

#endif