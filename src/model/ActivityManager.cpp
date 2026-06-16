#include "ActivityManager.h"

#include <algorithm>

bool ActivityManager::addActivity(std::unique_ptr<Activity> activity)
{
    if (!activity) {
        return false;
    }

    if (containsActivity(activity->id())) {
        return false;
    }

    m_activities.push_back(std::move(activity));
    return true;
}

std::unique_ptr<Activity> ActivityManager::removeActivity(const QString& id)
{
    auto it = findIteratorById(id);

    if (it == m_activities.end()) {
        return nullptr;
    }

    std::unique_ptr<Activity> removedActivity = std::move(*it);
    m_activities.erase(it);

    return removedActivity;
}

bool ActivityManager::replaceActivity(const QString& id, std::unique_ptr<Activity> updatedActivity)
{
    if (!updatedActivity) {
        return false;
    }

    if (updatedActivity->id() != id) {
        return false;
    }

    auto it = findIteratorById(id);

    if (it == m_activities.end()) {
        return false;
    }

    *it = std::move(updatedActivity);
    return true;
}

Activity* ActivityManager::findActivityById(const QString& id)
{
    auto it = findIteratorById(id);

    if (it == m_activities.end()) {
        return nullptr;
    }

    return it->get();
}

const Activity* ActivityManager::findActivityById(const QString& id) const
{
    auto it = findIteratorById(id);

    if (it == m_activities.end()) {
        return nullptr;
    }

    return it->get();
}

bool ActivityManager::containsActivity(const QString& id) const
{
    return findIteratorById(id) != m_activities.end();
}

std::unique_ptr<Activity> ActivityManager::cloneActivity(const QString& id) const
{
    const Activity* activity = findActivityById(id);

    if (!activity) {
        return nullptr;
    }

    return activity->clone();
}

std::vector<const Activity*> ActivityManager::activities() const
{
    std::vector<const Activity*> result;
    result.reserve(m_activities.size());

    for (const std::unique_ptr<Activity>& activity : m_activities) {
        result.push_back(activity.get());
    }

    return result;
}

int ActivityManager::size() const
{
    return static_cast<int>(m_activities.size());
}

bool ActivityManager::isEmpty() const
{
    return m_activities.empty();
}

void ActivityManager::clear()
{
    m_activities.clear();
}

std::vector<std::unique_ptr<Activity>>::iterator ActivityManager::findIteratorById(const QString& id)
{
    return std::find_if(m_activities.begin(), m_activities.end(),
                        [&id](const std::unique_ptr<Activity>& activity) {
                            return activity && activity->id() == id;
                        });
}

std::vector<std::unique_ptr<Activity>>::const_iterator ActivityManager::findIteratorById(const QString& id) const
{
    return std::find_if(m_activities.begin(), m_activities.end(),
                        [&id](const std::unique_ptr<Activity>& activity) {
                            return activity && activity->id() == id;
                        });
} 