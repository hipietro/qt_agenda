// File-level JSON storage for activities, templates and categories.

#ifndef AGENDAJSONSTORAGE_H
#define AGENDAJSONSTORAGE_H

#include <QString>

class ActivityManager;
class ActivityTemplateManager;
class CategoryManager;

class AgendaJsonStorage
{
public:
    static bool saveToFile(const ActivityManager& manager,
                           const ActivityTemplateManager& templateManager,
                           const CategoryManager& categoryManager,
                           const QString& filePath,
                           QString* errorMessage = nullptr);

    static bool loadFromFile(ActivityManager& manager,
                             ActivityTemplateManager& templateManager,
                             CategoryManager& categoryManager,
                             const QString& filePath,
                             QString* errorMessage = nullptr);

private:
    static void setError(QString* errorMessage, const QString& message);
};

#endif
