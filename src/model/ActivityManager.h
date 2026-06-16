#ifndef ACTIVITYMANAGER_H
#define ACTIVITYMANAGER_H

#include "Activity.h"

#include <QString>
#include <memory>
#include <vector>

class ActivityManager
{
public:
    ActivityManager() = default;

    ActivityManager(const ActivityManager&) = delete;
    ActivityManager& operator=(const ActivityManager&) = delete;

    ActivityManager(ActivityManager&&) = default;
    ActivityManager& operator=(ActivityManager&&) = default;

    bool addActivity(std::unique_ptr<Activity> activity);
    std::unique_ptr<Activity> removeActivity(const QString& id);
    bool replaceActivity(const QString& id, std::unique_ptr<Activity> updatedActivity);

    Activity* findActivityById(const QString& id);
    const Activity* findActivityById(const QString& id) const;

    bool containsActivity(const QString& id) const;
    std::unique_ptr<Activity> cloneActivity(const QString& id) const;

    std::vector<const Activity*> activities() const;

    int size() const;
    bool isEmpty() const;
    void clear();

private:
    std::vector<std::unique_ptr<Activity>>::iterator findIteratorById(const QString& id);
    std::vector<std::unique_ptr<Activity>>::const_iterator findIteratorById(const QString& id) const;

    std::vector<std::unique_ptr<Activity>> m_activities;
};

#endif