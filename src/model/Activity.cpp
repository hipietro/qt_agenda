// Shared Activity behavior: common fields, recurrence handling and generated ids.

#include "Activity.h"

#include <QUuid>
#include <utility>

Activity::Activity(const QString& title,
                   const QString& description,
                   const QString& category,
                   Priority priority,
                   bool completed,
                   const QString& id,
                   const QDateTime& createdAt,
                   const QDateTime& updatedAt)
    : m_id(id.isEmpty() ? generateId() : id),
      m_title(title),
      m_description(description),
      m_category(category),
      m_priority(priority),
      m_completed(completed),
      m_createdAt(createdAt),
      m_updatedAt(updatedAt)
{
}

QString Activity::id() const
{
    return m_id;
}

QString Activity::title() const
{
    return m_title;
}

void Activity::setTitle(const QString& title)
{
    m_title = title;
    touch();
}

QString Activity::description() const
{
    return m_description;
}

void Activity::setDescription(const QString& description)
{
    m_description = description;
    touch();
}

QString Activity::category() const
{
    return m_category;
}

void Activity::setCategory(const QString& category)
{
    m_category = category;
    touch();
}

Priority Activity::priority() const
{
    return m_priority;
}

void Activity::setPriority(Priority priority)
{
    m_priority = priority;
    touch();
}

bool Activity::isCompleted() const
{
    return m_completed;
}

void Activity::setCompleted(bool completed)
{
    m_completed = completed;
    touch();
}

QDateTime Activity::createdAt() const
{
    return m_createdAt;
}

QDateTime Activity::updatedAt() const
{
    return m_updatedAt;
}

bool Activity::hasRecurrence() const
{
    return m_recurrenceRule.has_value();
}

std::optional<RecurrenceRule> Activity::recurrenceRule() const
{
    return m_recurrenceRule;
}

void Activity::setRecurrenceRule(const RecurrenceRule& recurrenceRule)
{
    if (recurrenceRule.isValid()) {
        m_recurrenceRule = recurrenceRule;
        touch();
    }
}

void Activity::clearRecurrenceRule()
{
    if (m_recurrenceRule.has_value()) {
        m_recurrenceRule.reset();
        touch();
    }
}

QDateTime Activity::nextOccurrenceAfter(const QDateTime& after) const
{
    const QDateTime firstOccurrence = primaryDate();

    if (!firstOccurrence.isValid() || !after.isValid()) {
        return QDateTime();
    }

    if (!m_recurrenceRule.has_value()) {
        return firstOccurrence > after ? firstOccurrence : QDateTime();
    }

    return m_recurrenceRule->nextOccurrenceAfter(firstOccurrence, after);
}

void Activity::touch()
{
    m_updatedAt = QDateTime::currentDateTime();
}

std::unique_ptr<Activity> Activity::cloneWithNewId() const
{
    std::unique_ptr<Activity> copy = clone();

    const QDateTime now = QDateTime::currentDateTime();

    copy->m_id = generateId();
    copy->m_createdAt = now;
    copy->m_updatedAt = now;

    return copy;
}

QString Activity::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}