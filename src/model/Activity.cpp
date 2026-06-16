#include "Activity.h"

#include <QUuid>

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

void Activity::touch()
{
    m_updatedAt = QDateTime::currentDateTime();
}

QString Activity::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}