#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>

class Category
{
public:
    Category(const QString& name = QString(),
             const QString& colorHex = "#607D8B",
             const QString& id = QString());

    QString id() const;

    QString name() const;
    void setName(const QString& name);

    QString colorHex() const;
    void setColorHex(const QString& colorHex);

private:
    QString m_id;
    QString m_name;
    QString m_colorHex;

    static QString generateId();
};

#endif