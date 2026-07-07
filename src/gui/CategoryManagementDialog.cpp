#include "CategoryManagementDialog.h"

#include "model/Activity.h"
#include "model/ActivityManager.h"
#include "model/Category.h"
#include "model/CategoryManager.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>
#include <vector>

namespace
{
constexpr int CategoryIdRole = Qt::UserRole;
constexpr int CategoryNameRole = Qt::UserRole + 1;
}

CategoryManagementDialog::CategoryManagementDialog(CategoryManager* categoryManager,
                                                   ActivityManager* activityManager,
                                                   QWidget* parent)
    : QDialog(parent),
      m_categoryManager(categoryManager),
      m_activityManager(activityManager)
{
    setupUi();
    refreshCategoryList();
    updateButtons();
}

bool CategoryManagementDialog::activitiesChanged() const
{
    return m_activitiesChanged;
}

bool CategoryManagementDialog::categoriesChanged() const
{
    return m_categoriesChanged;
}

void CategoryManagementDialog::setupUi()
{
    setWindowTitle("Manage categories");
    resize(520, 420);
    setMinimumSize(420, 320);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Categories", this);
    titleLabel->setObjectName("dialogTitleLabel");

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);

    m_categoryList = new QListWidget(this);
    m_categoryList->setSelectionMode(QAbstractItemView::SingleSelection);

    m_addButton = new QPushButton("Add category", this);
    m_addButton->setObjectName("primaryButton");

    m_renameButton = new QPushButton("Rename", this);
    m_renameButton->setObjectName("primaryButton");

    m_clearButton = new QPushButton("Clear from activities", this);

    m_removeButton = new QPushButton("Remove category", this);
    m_removeButton->setObjectName("dangerButton");

    m_closeButton = new QPushButton("Close", this);

    QHBoxLayout* topActionLayout = new QHBoxLayout();
    topActionLayout->addWidget(m_addButton);
    topActionLayout->addWidget(m_renameButton);

    QHBoxLayout* bottomActionLayout = new QHBoxLayout();
    bottomActionLayout->addWidget(m_clearButton);
    bottomActionLayout->addWidget(m_removeButton);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_closeButton, QDialogButtonBox::AcceptRole);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(m_summaryLabel);
    mainLayout->addWidget(m_categoryList, 1);
    mainLayout->addLayout(topActionLayout);
    mainLayout->addLayout(bottomActionLayout);
    mainLayout->addWidget(buttonBox);

    connect(m_categoryList, &QListWidget::currentRowChanged, this, [this](int) {
        updateButtons();
    });

    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        addCategory();
    });

    connect(m_renameButton, &QPushButton::clicked, this, [this]() {
        renameSelectedCategory();
    });

    connect(m_clearButton, &QPushButton::clicked, this, [this]() {
        clearSelectedCategory();
    });

    connect(m_removeButton, &QPushButton::clicked, this, [this]() {
        removeSelectedCategory();
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        accept();
    });
}

void CategoryManagementDialog::refreshCategoryList()
{
    if (!m_categoryList || !m_categoryManager) {
        return;
    }

    const QString previousCategoryId = selectedCategoryId();

    m_categoryList->clear();

    std::vector<Category> categories = m_categoryManager->categories();

    std::sort(categories.begin(), categories.end(), [](const Category& first, const Category& second) {
        return QString::localeAwareCompare(first.name(), second.name()) < 0;
    });

    for (const Category& category : categories) {
        const int count = activityCountForCategory(category.name());

        QListWidgetItem* item = new QListWidgetItem(
            QString("%1\n%2 %3")
                .arg(category.name())
                .arg(count)
                .arg(count == 1 ? "activity" : "activities")
        );

        item->setData(CategoryIdRole, category.id());
        item->setData(CategoryNameRole, category.name());
        m_categoryList->addItem(item);

        if (category.id() == previousCategoryId) {
            m_categoryList->setCurrentItem(item);
        }
    }

    if (m_categoryList->currentRow() < 0 && m_categoryList->count() > 0) {
        m_categoryList->setCurrentRow(0);
    }

    const int categoryCount = m_categoryManager->size();
    m_summaryLabel->setText(
        QString("%1 %2 available. Renaming or clearing a category also updates existing activities.")
            .arg(categoryCount)
            .arg(categoryCount == 1 ? "category" : "categories")
    );

    updateButtons();
}

void CategoryManagementDialog::updateButtons()
{
    const bool hasSelection = !selectedCategoryId().isEmpty();

    if (m_renameButton) {
        m_renameButton->setEnabled(hasSelection);
    }

    if (m_clearButton) {
        m_clearButton->setEnabled(hasSelection && activityCountForCategory(selectedCategoryName()) > 0);
    }

    if (m_removeButton) {
        m_removeButton->setEnabled(hasSelection);
    }
}

void CategoryManagementDialog::addCategory()
{
    if (!m_categoryManager) {
        return;
    }

    bool ok = false;

    const QString categoryName = QInputDialog::getText(
        this,
        "Add category",
        "Category name",
        QLineEdit::Normal,
        QString(),
        &ok
    ).trimmed();

    if (!ok) {
        return;
    }

    if (categoryName.isEmpty()) {
        QMessageBox::warning(this, "Invalid category", "The category name cannot be empty.");
        return;
    }

    if (m_categoryManager->containsName(categoryName)) {
        QMessageBox::warning(this, "Duplicate category", "A category with this name already exists.");
        return;
    }

    if (!m_categoryManager->addCategory(categoryName)) {
        QMessageBox::warning(this, "Add category failed", "The category could not be added.");
        return;
    }

    m_categoriesChanged = true;
    refreshCategoryList();
    selectCategoryByName(categoryName);
}

void CategoryManagementDialog::renameSelectedCategory()
{
    if (!m_categoryManager || !m_activityManager) {
        return;
    }

    const QString categoryId = selectedCategoryId();
    const QString oldCategoryName = selectedCategoryName();

    if (categoryId.isEmpty() || oldCategoryName.isEmpty()) {
        return;
    }

    bool ok = false;

    const QString newCategoryName = QInputDialog::getText(
        this,
        "Rename category",
        "New category name",
        QLineEdit::Normal,
        oldCategoryName,
        &ok
    ).trimmed();

    if (!ok) {
        return;
    }

    if (newCategoryName.isEmpty()) {
        QMessageBox::warning(this, "Invalid category", "The category name cannot be empty.");
        return;
    }

    if (newCategoryName.compare(oldCategoryName, Qt::CaseInsensitive) == 0) {
        return;
    }

    if (m_categoryManager->containsName(newCategoryName)) {
        QMessageBox::warning(this, "Duplicate category", "A category with this name already exists.");
        return;
    }

    const Category* category = m_categoryManager->findCategoryById(categoryId);

    if (!category) {
        QMessageBox::warning(this, "Rename failed", "The selected category could not be found.");
        refreshCategoryList();
        return;
    }

    const QString colorHex = category->colorHex();

    if (!m_categoryManager->updateCategory(categoryId, newCategoryName, colorHex)) {
        QMessageBox::warning(this, "Rename failed", "The category could not be renamed.");
        return;
    }

    const int updatedActivities = m_activityManager->replaceCategory(oldCategoryName, newCategoryName);

    m_categoriesChanged = true;

    if (updatedActivities > 0) {
        m_activitiesChanged = true;
    }

    refreshCategoryList();
    selectCategoryByName(newCategoryName);
}

void CategoryManagementDialog::clearSelectedCategory()
{
    if (!m_activityManager) {
        return;
    }

    const QString categoryName = selectedCategoryName();

    if (categoryName.isEmpty()) {
        return;
    }

    const int count = activityCountForCategory(categoryName);

    if (count == 0) {
        QMessageBox::information(this, "No activities", "No activities currently use this category.");
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Clear category",
        QString("Clear category \"%1\" from %2 %3?\n\nThe category will remain available, but the affected activities will have no category.")
            .arg(categoryName)
            .arg(count)
            .arg(count == 1 ? "activity" : "activities"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    const int updatedActivities = m_activityManager->clearCategory(categoryName);

    if (updatedActivities > 0) {
        m_activitiesChanged = true;
    }

    refreshCategoryList();
    selectCategoryByName(categoryName);
}

void CategoryManagementDialog::removeSelectedCategory()
{
    if (!m_categoryManager || !m_activityManager) {
        return;
    }

    const QString categoryId = selectedCategoryId();
    const QString categoryName = selectedCategoryName();

    if (categoryId.isEmpty() || categoryName.isEmpty()) {
        return;
    }

    const int count = activityCountForCategory(categoryName);

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Remove category",
        QString("Remove category \"%1\"?\n\n%2 %3 currently use this category. Removing it will also clear the category from those activities.")
            .arg(categoryName)
            .arg(count)
            .arg(count == 1 ? "activity" : "activities"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    const int updatedActivities = m_activityManager->clearCategory(categoryName);

    if (!m_categoryManager->removeCategoryById(categoryId)) {
        QMessageBox::warning(this, "Remove failed", "The selected category could not be removed.");
        return;
    }

    m_categoriesChanged = true;

    if (updatedActivities > 0) {
        m_activitiesChanged = true;
    }

    refreshCategoryList();
}

QString CategoryManagementDialog::selectedCategoryId() const
{
    if (!m_categoryList || !m_categoryList->currentItem()) {
        return QString();
    }

    return m_categoryList->currentItem()->data(CategoryIdRole).toString();
}

QString CategoryManagementDialog::selectedCategoryName() const
{
    if (!m_categoryList || !m_categoryList->currentItem()) {
        return QString();
    }

    return m_categoryList->currentItem()->data(CategoryNameRole).toString();
}

int CategoryManagementDialog::activityCountForCategory(const QString& categoryName) const
{
    if (!m_activityManager || categoryName.trimmed().isEmpty()) {
        return 0;
    }

    const QString normalizedCategory = categoryName.simplified().toLower();
    int count = 0;

    for (const Activity* activity : m_activityManager->activities()) {
        if (activity && activity->category().simplified().toLower() == normalizedCategory) {
            ++count;
        }
    }

    return count;
}

void CategoryManagementDialog::selectCategoryByName(const QString& categoryName)
{
    if (!m_categoryList) {
        return;
    }

    for (int row = 0; row < m_categoryList->count(); ++row) {
        QListWidgetItem* item = m_categoryList->item(row);

        if (item && item->data(CategoryNameRole).toString().compare(categoryName, Qt::CaseInsensitive) == 0) {
            m_categoryList->setCurrentRow(row);
            return;
        }
    }
}
