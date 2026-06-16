#include "EventActivity.h"

EventActivity::EventActivity(const QString& title,
                             const QDateTime& startDateTime,
                             const QDateTime& endDateTime,
                             const QString& location,
                             const QStringList& participants,
                             const QString& description,
                             const QString& category,
                             Priority priority,
                             bool completed,
                             const QString& id)
    : Activity(title, description, category, priority, completed, id),
      m_startDateTime(startDateTime),
      m_endDateTime(endDateTime),
      m_location(location),
      m_participants(participants)
{
}

QDateTime EventActivity::startDateTime() const
{
    return m_startDateTime;
}

void EventActivity::setStartDateTime(const QDateTime& startDateTime)
{
    m_startDateTime = startDateTime;
    touch();
}

QDateTime EventActivity::endDateTime() const
{
    return m_endDateTime;
}

void EventActivity::setEndDateTime(const QDateTime& endDateTime)
{
    m_endDateTime = endDateTime;
    touch();
}

QString EventActivity::location() const
{
    return m_location;
}

void EventActivity::setLocation(const QString& location)
{
    m_location = location;
    touch();
}

QStringList EventActivity::participants() const
{
    return m_participants;
}

void EventActivity::setParticipants(const QStringList& participants)
{
    m_participants = participants;
    touch();
}

QDateTime EventActivity::primaryDate() const
{
    return m_startDateTime;
}

bool EventActivity::isOverdue(const QDateTime& now) const
{
    return !isCompleted() && m_endDateTime.isValid() && m_endDateTime < now;
}

QString EventActivity::summary() const
{
    QString text = QString("Event: %1 | %2 - %3")
            .arg(title(),
                 m_startDateTime.toString("yyyy-MM-dd hh:mm"),
                 m_endDateTime.toString("yyyy-MM-dd hh:mm"));

    if (!m_location.isEmpty()) {
        text += QString(" | Location: %1").arg(m_location);
    }

    if (!m_participants.isEmpty()) {
        text += QString(" | Participants: %1").arg(m_participants.join(", "));
    }

    return text;
}

std::unique_ptr<Activity> EventActivity::clone() const
{
    return std::make_unique<EventActivity>(*this);
}