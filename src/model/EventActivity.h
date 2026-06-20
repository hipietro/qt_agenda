#ifndef EVENTACTIVITY_H
#define EVENTACTIVITY_H

#include "Activity.h"

#include <QStringList>

class EventActivity : public Activity
{
public:
    EventActivity(const QString& title,
                const QDateTime& startDateTime,
                const QDateTime& endDateTime,
                const QString& location = QString(),
                const QStringList& participants = QStringList(),
                const QString& description = QString(),
                const QString& category = QString(),
                Priority priority = Priority::Medium,
                bool completed = false,
                const QString& id = QString(),
                const QDateTime& createdAt = QDateTime::currentDateTime(),
                const QDateTime& updatedAt = QDateTime::currentDateTime());

    QDateTime startDateTime() const;
    void setStartDateTime(const QDateTime& startDateTime);

    QDateTime endDateTime() const;
    void setEndDateTime(const QDateTime& endDateTime);

    QString location() const;
    void setLocation(const QString& location);

    QStringList participants() const;
    void setParticipants(const QStringList& participants);

    QDateTime primaryDate() const override;
    bool isOverdue(const QDateTime& now) const override;
    QString summary() const override;
    std::unique_ptr<Activity> clone() const override;
    ActivityKind kind() const override;

private:
    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    QString m_location;
    QStringList m_participants;
};

#endif