#include "ReminderActivity.h"

ReminderActivity::ReminderActivity(const QString& title,
                                   const QDateTime& reminderDateTime,
                                   int advanceMinutes,
                                   const QString& reminderNote,
                                   const QString& description,
                                   const QString& category,
                                   Priority priority,
                                   bool completed,
                                   const QString& id,
                                   const QDateTime& createdAt,
                                   const QDateTime& updatedAt)
    : Activity(title, description, category, priority, completed, id, createdAt, updatedAt),
      m_reminderDateTime(reminderDateTime),
      m_advanceMinutes(advanceMinutes),
      m_reminderNote(reminderNote)
{
}

QDateTime ReminderActivity::reminderDateTime() const
{
    return m_reminderDateTime;
}

void ReminderActivity::setReminderDateTime(const QDateTime& reminderDateTime)
{
    m_reminderDateTime = reminderDateTime;
    touch();
}

int ReminderActivity::advanceMinutes() const
{
    return m_advanceMinutes;
}

void ReminderActivity::setAdvanceMinutes(int advanceMinutes)
{
    m_advanceMinutes = advanceMinutes < 0 ? 0 : advanceMinutes;
    touch();
}

QString ReminderActivity::reminderNote() const
{
    return m_reminderNote;
}

void ReminderActivity::setReminderNote(const QString& reminderNote)
{
    m_reminderNote = reminderNote;
    touch();
}

QDateTime ReminderActivity::primaryDate() const
{
    return m_reminderDateTime;
}

bool ReminderActivity::isOverdue(const QDateTime& now) const
{
    return !isCompleted() && m_reminderDateTime.isValid() && m_reminderDateTime < now;
}

QString ReminderActivity::summary() const
{
    QString text = QString("Reminder: %1 | At: %2")
            .arg(title(), m_reminderDateTime.toString("yyyy-MM-dd HH:mm"));

    if (m_advanceMinutes > 0) {
        text += QString(" | Advance: %1 min").arg(m_advanceMinutes);
    }

    if (!m_reminderNote.isEmpty()) {
        text += QString(" | Note: %1").arg(m_reminderNote);
    }

    return text;
}

std::unique_ptr<Activity> ReminderActivity::clone() const
{
    return std::make_unique<ReminderActivity>(*this);
}

ActivityKind ReminderActivity::kind() const
{
    return ActivityKind::Reminder;
}