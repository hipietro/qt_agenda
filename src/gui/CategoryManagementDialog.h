// Dialog for maintaining the custom category list.

#ifndef CATEGORYMANAGEMENTDIALOG_H
#define CATEGORYMANAGEMENTDIALOG_H

#include <QDialog>
#include <QString>

class ActivityManager;
class CategoryManager;
class QLabel;
class QListWidget;
class QPushButton;

/*
 * Dialog per gestire le categorie personalizzate.
 *
 * Ho scelto di tenerlo separato da MainWindow perché la finestra principale
 * deve solo aprire il dialog e aggiornare la vista quando la gestione termina.
 */
class CategoryManagementDialog : public QDialog
{
public:
    explicit CategoryManagementDialog(CategoryManager* categoryManager,
                                      ActivityManager* activityManager,
                                      QWidget* parent = nullptr);

    bool activitiesChanged() const;
    bool categoriesChanged() const;

private:
    void setupUi();
    void refreshCategoryList();
    void updateButtons();

    void addCategory();
    void renameSelectedCategory();
    void clearSelectedCategory();
    void removeSelectedCategory();

    QString selectedCategoryId() const;
    QString selectedCategoryName() const;
    int activityCountForCategory(const QString& categoryName) const;
    void selectCategoryByName(const QString& categoryName);

    CategoryManager* m_categoryManager = nullptr;
    ActivityManager* m_activityManager = nullptr;

    QLabel* m_summaryLabel = nullptr;
    QListWidget* m_categoryList = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_renameButton = nullptr;
    QPushButton* m_clearButton = nullptr;
    QPushButton* m_removeButton = nullptr;
    QPushButton* m_closeButton = nullptr;

    bool m_activitiesChanged = false;
    bool m_categoriesChanged = false;
};

#endif
