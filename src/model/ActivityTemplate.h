#ifndef ACTIVITYTEMPLATE_H
#define ACTIVITYTEMPLATE_H

#include "Activity.h"

#include <QString>
#include <memory>

class ActivityTemplate
{
public:
    ActivityTemplate(const QString& name = QString(),
                     std::unique_ptr<Activity> prototype = nullptr,
                     const QString& id = QString());

    ActivityTemplate(const ActivityTemplate& other);
    ActivityTemplate& operator=(const ActivityTemplate& other);

    ActivityTemplate(ActivityTemplate&&) noexcept = default;
    ActivityTemplate& operator=(ActivityTemplate&&) noexcept = default;

    QString id() const;

    QString name() const;
    void setName(const QString& name);

    bool hasPrototype() const;
    const Activity* prototype() const;

    void setPrototype(std::unique_ptr<Activity> prototype);

    std::unique_ptr<Activity> createActivity() const;

    bool isValid() const;

private:
    QString m_id;
    QString m_name;
    std::unique_ptr<Activity> m_prototype;

    static QString generateId();
};

#endif