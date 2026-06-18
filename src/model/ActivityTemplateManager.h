#ifndef ACTIVITYTEMPLATEMANAGER_H
#define ACTIVITYTEMPLATEMANAGER_H

#include "ActivityTemplate.h"

#include <QString>
#include <memory>
#include <vector>

class ActivityTemplateManager
{
public:
    ActivityTemplateManager() = default;

    bool addTemplate(const QString& name, std::unique_ptr<Activity> prototype);
    bool addTemplate(ActivityTemplate activityTemplate);

    bool updateTemplateName(const QString& id, const QString& newName);
    bool replaceTemplatePrototype(const QString& id, std::unique_ptr<Activity> newPrototype);

    bool removeTemplateById(const QString& id);
    bool removeTemplateByName(const QString& name);

    ActivityTemplate* findTemplateById(const QString& id);
    const ActivityTemplate* findTemplateById(const QString& id) const;

    ActivityTemplate* findTemplateByName(const QString& name);
    const ActivityTemplate* findTemplateByName(const QString& name) const;

    bool containsId(const QString& id) const;
    bool containsName(const QString& name) const;

    std::unique_ptr<Activity> createActivityFromTemplateId(const QString& id) const;
    std::unique_ptr<Activity> createActivityFromTemplateName(const QString& name) const;

    std::vector<const ActivityTemplate*> templates() const;

    int size() const;
    bool isEmpty() const;
    void clear();

private:
    static QString normalizedName(const QString& name);
    static bool isValidName(const QString& name);

    std::vector<ActivityTemplate>::iterator findIteratorById(const QString& id);
    std::vector<ActivityTemplate>::const_iterator findIteratorById(const QString& id) const;

    std::vector<ActivityTemplate>::iterator findIteratorByName(const QString& name);
    std::vector<ActivityTemplate>::const_iterator findIteratorByName(const QString& name) const;

    bool nameAlreadyUsedByAnotherTemplate(const QString& name,
                                          const QString& currentTemplateId) const;

    std::vector<ActivityTemplate> m_templates;
};

#endif