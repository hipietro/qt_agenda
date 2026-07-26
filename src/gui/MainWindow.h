// Main application window. It coordinates GUI actions but keeps data logic in the managers.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QString>

#include <memory>
#include <vector>

#include "model/Activity.h"
#include "commands/CommandHistory.h"

class ActivityCreationPage;
class ActivityEditPage;
class ActivityManager;
class ActivityTemplateManager;
class CategoryManager;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QResizeEvent;
class QScrollArea;
class QSplitter;
class QTextEdit;
class QStackedWidget;
class QWidget;
class QAction;

class MainWindow : public QMainWindow
{
public:
explicit MainWindow(ActivityManager* activityManager,
                    ActivityTemplateManager* templateManager,
                    CategoryManager* categoryManager,
                    QWidget* parent = nullptr);
protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void setupMenuBar();
    void connectSignals();

    void openAgendaPage();
    void openCreationPage();
    void openEditingPage();
    void showPage(QWidget* page);
    void updateResponsiveWorkspace();

    void refreshActivityList();
    void updateActionButtons();
    void updateCategoryFilterOptions();
    void synchronizeCategoryManagerFromActivities();
    void updateWindowTitle();
    void setUnsavedChanges(bool hasUnsavedChanges);
    bool confirmDiscardUnsavedChanges();
    void undoLastCommand();
    void redoLastCommand();

    std::vector<const Activity*> collectVisibleActivities() const;
    const Activity* findActivityById(const QString& id) const;
    QString selectedActivityId() const;


    QString fileDisplayName(const QString& filePath) const;
    QString storageSummaryText() const;

    void toggleSelectedActivityCompletion();
    void deleteSelectedActivity();
    void createActivity();
    void addCreatedActivity(std::unique_ptr<Activity> activity);
    void editSelectedActivity();
    bool applyEditedActivity(const QString& activityId,
                             std::unique_ptr<Activity> activity);
    void createActivityFromTemplate();
    void saveSelectedActivityAsTemplate();
    void manageCategories();

    bool saveAgenda();
    bool saveAgendaAs();
    void loadAgenda();

    ActivityManager* m_activityManager = nullptr;
    ActivityTemplateManager* m_templateManager = nullptr;
    CategoryManager* m_categoryManager = nullptr;

    CommandHistory m_commandHistory;

    QStackedWidget* m_pageStack = nullptr;
    QWidget* m_agendaPage = nullptr;
    QSplitter* m_mainSplitter = nullptr;
    QScrollArea* m_leftScrollArea = nullptr;
    QStackedWidget* m_workspaceStack = nullptr;
    QWidget* m_detailPage = nullptr;
    ActivityCreationPage* m_creationPage = nullptr;
    ActivityEditPage* m_editingPage = nullptr;

    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QComboBox* m_priorityCombo = nullptr;
    QComboBox* m_categoryCombo = nullptr;
    QComboBox* m_completionCombo = nullptr;
    QComboBox* m_recurrenceCombo = nullptr;
    QComboBox* m_overdueCombo = nullptr;
    QComboBox* m_sortCombo = nullptr;
    QListWidget* m_activityList = nullptr;
    QTextEdit* m_detailView = nullptr;
    QLabel* m_resultCountLabel = nullptr;

    QPushButton* m_addButton = nullptr;
    QPushButton* m_toggleCompletedButton = nullptr;
    QPushButton* m_deleteButton = nullptr;
    QPushButton* m_editButton = nullptr;
    QPushButton* m_templateButton = nullptr;
    QPushButton* m_undoButton = nullptr;
    QPushButton* m_redoButton = nullptr;

    QString m_currentFilePath;
    bool m_hasUnsavedChanges = false;

    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;
};

#endif