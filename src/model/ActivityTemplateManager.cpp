#include "ActivityTemplateManager.h"

#include <algorithm>
#include <utility>

bool ActivityTemplateManager::addTemplate(const QString& name, std::unique_ptr<Activity> prototype)
{
    return addTemplate(ActivityTemplate(name, std::move(prototype)));
}

bool ActivityTemplateManager::addTemplate(ActivityTemplate activityTemplate)
{
    if (!activityTemplate.isValid()) {
        return false;
    }

    if (containsId(activityTemplate.id())) {
        return false;
    }

    if (containsName(activityTemplate.name())) {
        return false;
    }

    m_templates.push_back(std::move(activityTemplate));
    return true;
}

bool ActivityTemplateManager::updateTemplateName(const QString& id, const QString& newName)
{
    auto it = findIteratorById(id);

    if (it == m_templates.end()) {
        return false;
    }

    if (!isValidName(newName)) {
        return false;
    }

    if (nameAlreadyUsedByAnotherTemplate(newName, id)) {
        return false;
    }

    it->setName(newName);
    return true;
}

bool ActivityTemplateManager::replaceTemplatePrototype(const QString& id,
                                                       std::unique_ptr<Activity> newPrototype)
{
    auto it = findIteratorById(id);

    if (it == m_templates.end()) {
        return false;
    }

    if (!newPrototype) {
        return false;
    }

    it->setPrototype(std::move(newPrototype));
    return true;
}

bool ActivityTemplateManager::removeTemplateById(const QString& id)
{
    auto it = findIteratorById(id);

    if (it == m_templates.end()) {
        return false;
    }

    m_templates.erase(it);
    return true;
}

bool ActivityTemplateManager::removeTemplateByName(const QString& name)
{
    auto it = findIteratorByName(name);

    if (it == m_templates.end()) {
        return false;
    }

    m_templates.erase(it);
    return true;
}

ActivityTemplate* ActivityTemplateManager::findTemplateById(const QString& id)
{
    auto it = findIteratorById(id);

    if (it == m_templates.end()) {
        return nullptr;
    }

    return &(*it);
}

const ActivityTemplate* ActivityTemplateManager::findTemplateById(const QString& id) const
{
    auto it = findIteratorById(id);

    if (it == m_templates.end()) {
        return nullptr;
    }

    return &(*it);
}

ActivityTemplate* ActivityTemplateManager::findTemplateByName(const QString& name)
{
    auto it = findIteratorByName(name);

    if (it == m_templates.end()) {
        return nullptr;
    }

    return &(*it);
}

const ActivityTemplate* ActivityTemplateManager::findTemplateByName(const QString& name) const
{
    auto it = findIteratorByName(name);

    if (it == m_templates.end()) {
        return nullptr;
    }

    return &(*it);
}

bool ActivityTemplateManager::containsId(const QString& id) const
{
    return findIteratorById(id) != m_templates.end();
}

bool ActivityTemplateManager::containsName(const QString& name) const
{
    return findIteratorByName(name) != m_templates.end();
}

std::unique_ptr<Activity> ActivityTemplateManager::createActivityFromTemplateId(const QString& id) const
{
    const ActivityTemplate* activityTemplate = findTemplateById(id);

    if (!activityTemplate) {
        return nullptr;
    }

    return activityTemplate->createActivity();
}

std::unique_ptr<Activity> ActivityTemplateManager::createActivityFromTemplateName(const QString& name) const
{
    const ActivityTemplate* activityTemplate = findTemplateByName(name);

    if (!activityTemplate) {
        return nullptr;
    }

    return activityTemplate->createActivity();
}

std::vector<const ActivityTemplate*> ActivityTemplateManager::templates() const
{
    std::vector<const ActivityTemplate*> result;
    result.reserve(m_templates.size());

    for (const ActivityTemplate& activityTemplate : m_templates) {
        result.push_back(&activityTemplate);
    }

    return result;
}

int ActivityTemplateManager::size() const
{
    return static_cast<int>(m_templates.size());
}

bool ActivityTemplateManager::isEmpty() const
{
    return m_templates.empty();
}

void ActivityTemplateManager::clear()
{
    m_templates.clear();
}

QString ActivityTemplateManager::normalizedName(const QString& name)
{
    return name.simplified().toLower();
}

bool ActivityTemplateManager::isValidName(const QString& name)
{
    return !name.trimmed().isEmpty();
}

std::vector<ActivityTemplate>::iterator ActivityTemplateManager::findIteratorById(const QString& id)
{
    return std::find_if(m_templates.begin(), m_templates.end(),
                        [&id](const ActivityTemplate& activityTemplate) {
                            return activityTemplate.id() == id;
                        });
}

std::vector<ActivityTemplate>::const_iterator ActivityTemplateManager::findIteratorById(const QString& id) const
{
    return std::find_if(m_templates.begin(), m_templates.end(),
                        [&id](const ActivityTemplate& activityTemplate) {
                            return activityTemplate.id() == id;
                        });
}

std::vector<ActivityTemplate>::iterator ActivityTemplateManager::findIteratorByName(const QString& name)
{
    const QString normalized = normalizedName(name);

    return std::find_if(m_templates.begin(), m_templates.end(),
                        [&normalized](const ActivityTemplate& activityTemplate) {
                            return normalizedName(activityTemplate.name()) == normalized;
                        });
}

std::vector<ActivityTemplate>::const_iterator ActivityTemplateManager::findIteratorByName(const QString& name) const
{
    const QString normalized = normalizedName(name);

    return std::find_if(m_templates.begin(), m_templates.end(),
                        [&normalized](const ActivityTemplate& activityTemplate) {
                            return normalizedName(activityTemplate.name()) == normalized;
                        });
}

bool ActivityTemplateManager::nameAlreadyUsedByAnotherTemplate(const QString& name,
                                                               const QString& currentTemplateId) const
{
    const QString normalized = normalizedName(name);

    for (const ActivityTemplate& activityTemplate : m_templates) {
        if (activityTemplate.id() != currentTemplateId &&
            normalizedName(activityTemplate.name()) == normalized) {
            return true;
        }
    }

    return false;
}