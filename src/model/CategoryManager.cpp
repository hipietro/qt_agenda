#include "CategoryManager.h"

#include <QRegularExpression>

#include <algorithm>

bool CategoryManager::addCategory(const QString& name, const QString& colorHex)
{
    return addCategory(Category(name, colorHex));
}

bool CategoryManager::addCategory(const Category& category)
{
    if (!isValidName(category.name())) {
        return false;
    }

    if (!isValidColorHex(category.colorHex())) {
        return false;
    }

    if (containsId(category.id())) {
        return false;
    }

    if (containsName(category.name())) {
        return false;
    }

    m_categories.push_back(category);
    return true;
}

bool CategoryManager::updateCategory(const QString& id,
                                     const QString& newName,
                                     const QString& newColorHex)
{
    auto it = findIteratorById(id);

    if (it == m_categories.end()) {
        return false;
    }

    if (!isValidName(newName)) {
        return false;
    }

    if (!isValidColorHex(newColorHex)) {
        return false;
    }

    if (nameAlreadyUsedByAnotherCategory(newName, id)) {
        return false;
    }

    it->setName(newName);
    it->setColorHex(newColorHex);

    return true;
}

bool CategoryManager::removeCategoryById(const QString& id)
{
    auto it = findIteratorById(id);

    if (it == m_categories.end()) {
        return false;
    }

    m_categories.erase(it);
    return true;
}

bool CategoryManager::removeCategoryByName(const QString& name)
{
    auto it = findIteratorByName(name);

    if (it == m_categories.end()) {
        return false;
    }

    m_categories.erase(it);
    return true;
}

Category* CategoryManager::findCategoryById(const QString& id)
{
    auto it = findIteratorById(id);

    if (it == m_categories.end()) {
        return nullptr;
    }

    return &(*it);
}

const Category* CategoryManager::findCategoryById(const QString& id) const
{
    auto it = findIteratorById(id);

    if (it == m_categories.end()) {
        return nullptr;
    }

    return &(*it);
}

Category* CategoryManager::findCategoryByName(const QString& name)
{
    auto it = findIteratorByName(name);

    if (it == m_categories.end()) {
        return nullptr;
    }

    return &(*it);
}

const Category* CategoryManager::findCategoryByName(const QString& name) const
{
    auto it = findIteratorByName(name);

    if (it == m_categories.end()) {
        return nullptr;
    }

    return &(*it);
}

bool CategoryManager::containsName(const QString& name) const
{
    return findIteratorByName(name) != m_categories.end();
}

bool CategoryManager::containsId(const QString& id) const
{
    return findIteratorById(id) != m_categories.end();
}

std::vector<Category> CategoryManager::categories() const
{
    return m_categories;
}

int CategoryManager::size() const
{
    return static_cast<int>(m_categories.size());
}

bool CategoryManager::isEmpty() const
{
    return m_categories.empty();
}

void CategoryManager::clear()
{
    m_categories.clear();
}

QString CategoryManager::normalizedName(const QString& name)
{
    return name.simplified().toLower();
}

bool CategoryManager::isValidName(const QString& name)
{
    return !name.trimmed().isEmpty();
}

bool CategoryManager::isValidColorHex(const QString& colorHex)
{
    static const QRegularExpression colorPattern("^#[0-9A-Fa-f]{6}$");
    return colorPattern.match(colorHex.trimmed()).hasMatch();
}

std::vector<Category>::iterator CategoryManager::findIteratorById(const QString& id)
{
    return std::find_if(m_categories.begin(), m_categories.end(),
                        [&id](const Category& category) {
                            return category.id() == id;
                        });
}

std::vector<Category>::const_iterator CategoryManager::findIteratorById(const QString& id) const
{
    return std::find_if(m_categories.begin(), m_categories.end(),
                        [&id](const Category& category) {
                            return category.id() == id;
                        });
}

std::vector<Category>::iterator CategoryManager::findIteratorByName(const QString& name)
{
    const QString normalized = normalizedName(name);

    return std::find_if(m_categories.begin(), m_categories.end(),
                        [&normalized](const Category& category) {
                            return normalizedName(category.name()) == normalized;
                        });
}

std::vector<Category>::const_iterator CategoryManager::findIteratorByName(const QString& name) const
{
    const QString normalized = normalizedName(name);

    return std::find_if(m_categories.begin(), m_categories.end(),
                        [&normalized](const Category& category) {
                            return normalizedName(category.name()) == normalized;
                        });
}

bool CategoryManager::nameAlreadyUsedByAnotherCategory(const QString& name,
                                                       const QString& currentCategoryId) const
{
    const QString normalized = normalizedName(name);

    for (const Category& category : m_categories) {
        if (category.id() != currentCategoryId &&
            normalizedName(category.name()) == normalized) {
            return true;
        }
    }

    return false;
}