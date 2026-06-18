#ifndef CATEGORYMANAGER_H
#define CATEGORYMANAGER_H

#include "Category.h"

#include <QString>
#include <vector>

class CategoryManager
{
public:
    CategoryManager() = default;

    bool addCategory(const QString& name, const QString& colorHex = "#607D8B");
    bool addCategory(const Category& category);

    bool updateCategory(const QString& id,
                        const QString& newName,
                        const QString& newColorHex);

    bool removeCategoryById(const QString& id);
    bool removeCategoryByName(const QString& name);

    Category* findCategoryById(const QString& id);
    const Category* findCategoryById(const QString& id) const;

    Category* findCategoryByName(const QString& name);
    const Category* findCategoryByName(const QString& name) const;

    bool containsName(const QString& name) const;
    bool containsId(const QString& id) const;

    std::vector<Category> categories() const;

    int size() const;
    bool isEmpty() const;
    void clear();

private:
    static QString normalizedName(const QString& name);
    static bool isValidName(const QString& name);
    static bool isValidColorHex(const QString& colorHex);

    std::vector<Category>::iterator findIteratorById(const QString& id);
    std::vector<Category>::const_iterator findIteratorById(const QString& id) const;

    std::vector<Category>::iterator findIteratorByName(const QString& name);
    std::vector<Category>::const_iterator findIteratorByName(const QString& name) const;

    bool nameAlreadyUsedByAnotherCategory(const QString& name,
                                          const QString& currentCategoryId) const;

    std::vector<Category> m_categories;
};

#endif