#include "ActivityTemplate.h"

#include <QUuid>

ActivityTemplate::ActivityTemplate(const QString& name,
                                   std::unique_ptr<Activity> prototype,
                                   const QString& id)
    : m_id(id.isEmpty() ? generateId() : id),
      m_name(name.trimmed()),
      m_prototype(std::move(prototype))
{
}

ActivityTemplate::ActivityTemplate(const ActivityTemplate& other)
    : m_id(other.m_id),
      m_name(other.m_name),
      m_prototype(other.m_prototype ? other.m_prototype->clone() : nullptr)
{
}

ActivityTemplate& ActivityTemplate::operator=(const ActivityTemplate& other)
{
    if (this == &other) {
        return *this;
    }

    m_id = other.m_id;
    m_name = other.m_name;
    m_prototype = other.m_prototype ? other.m_prototype->clone() : nullptr;

    return *this;
}

QString ActivityTemplate::id() const
{
    return m_id;
}

QString ActivityTemplate::name() const
{
    return m_name;
}

void ActivityTemplate::setName(const QString& name)
{
    m_name = name.trimmed();
}

bool ActivityTemplate::hasPrototype() const
{
    return m_prototype != nullptr;
}

const Activity* ActivityTemplate::prototype() const
{
    return m_prototype.get();
}

void ActivityTemplate::setPrototype(std::unique_ptr<Activity> prototype)
{
    m_prototype = std::move(prototype);
}

std::unique_ptr<Activity> ActivityTemplate::createActivity() const
{
    if (!m_prototype) {
        return nullptr;
    }

    return m_prototype->cloneWithNewId();
}

bool ActivityTemplate::isValid() const
{
    return !m_name.trimmed().isEmpty() && m_prototype != nullptr;
}

QString ActivityTemplate::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}