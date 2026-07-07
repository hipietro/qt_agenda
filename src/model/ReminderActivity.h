// Concrete activity for reminders, including a configurable alert offset.

#ifndef REMINDERACTIVITY_H
#define REMINDERACTIVITY_H

#include "Activity.h"

class ReminderActivity : public Activity
{
public:
    ReminderActivity(const QString& title,
                    const QDateTime& reminderDateTime,
                    int advanceMinutes = 0,
                    const QString& reminderNote = QString(),
                    const QString& description = QString(),
                    const QString& category = QString(),
                    Priority priority = Priority::Medium,
                    bool completed = false,
                    const QString& id = QString(),
                    const QDateTime& createdAt = QDateTime::currentDateTime(),
                    const QDateTime& updatedAt = QDateTime::currentDateTime());

    QDateTime reminderDateTime() const;
    void setReminderDateTime(const QDateTime& reminderDateTime);

    int advanceMinutes() const;
    void setAdvanceMinutes(int advanceMinutes);

    QString reminderNote() const;
    void setReminderNote(const QString& reminderNote);

    QDateTime primaryDate() const override;
    bool isOverdue(const QDateTime& now) const override;
    QString summary() const override;
    std::unique_ptr<Activity> clone() const override;
    ActivityKind kind() const override;

private:
    QDateTime m_reminderDateTime;
    int m_advanceMinutes;
    QString m_reminderNote;
};

#endif