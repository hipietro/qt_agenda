#include "Category.h"

#include <QUuid>

Category::Category(const QString& name,
                   const QString& colorHex,
                   const QString& id)
    : m_id(id.isEmpty() ? generateId() : id),
      m_name(name.trimmed()),
      m_colorHex(colorHex.trimmed())
{
}

QString Category::id() const
{
    return m_id;
}

QString Category::name() const
{
    return m_name;
}

void Category::setName(const QString& name)
{
    m_name = name.trimmed();
}

QString Category::colorHex() const
{
    return m_colorHex;
}

void Category::setColorHex(const QString& colorHex)
{
    m_colorHex = colorHex.trimmed();
}

QString Category::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}