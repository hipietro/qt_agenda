#include "ReminderActivity.h"

ReminderActivity::ReminderActivity(const QString& title,
                                   const QDateTime& reminderDateTime,
                                   int advanceMinutes,
                                   const QString& reminderNote,
                                   const QString& description,
                                   const QString& category,
                                   Priority priority,
                                   bool completed,
                                   const QString& id)
    : Activity(title, description, category, priority, completed, id),
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
    QString text = QString("Promemoria: %1 | Alle: %2")
            .arg(title(), m_reminderDateTime.toString("dd/MM/yyyy HH:mm"));

    if (m_advanceMinutes > 0) {
        text += QString(" | Anticipo: %1 min").arg(m_advanceMinutes);
    }

    if (!m_reminderNote.isEmpty()) {
        text += QString(" | Nota: %1").arg(m_reminderNote);
    }

    return text;
}

std::unique_ptr<Activity> ReminderActivity::clone() const
{
    return std::make_unique<ReminderActivity>(*this);
}

