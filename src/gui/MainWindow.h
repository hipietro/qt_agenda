#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QString>

#include <vector>

#include "model/Activity.h"
#include "model/ActivityKind.h"
#include "model/Priority.h"
#include "commands/CommandHistory.h"

class ActivityManager;
class ActivityTemplateManager;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTextEdit;
class QAction;

class MainWindow : public QMainWindow
{
public:
explicit MainWindow(ActivityManager* activityManager,
                    ActivityTemplateManager* templateManager,
                    QWidget* parent = nullptr);
protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void setupMenuBar();
    void connectSignals();

    void refreshActivityList();
    void showActivityDetails(const Activity* activity);
    void updateActionButtons();
    void updateCategoryFilterOptions();
    void updateWindowTitle();
    void setUnsavedChanges(bool hasUnsavedChanges);
    bool confirmDiscardUnsavedChanges();
    void undoLastCommand();
    void redoLastCommand();

    std::vector<const Activity*> collectVisibleActivities() const;
    const Activity* findActivityById(const QString& id) const;
    QString selectedActivityId() const;

    QString activityListItemText(const Activity* activity) const;
    void applyActivityListItemVisualState(QListWidgetItem* item, const Activity* activity) const;

    QString statusText(const Activity* activity) const;
    QString priorityText(Priority priority) const;
    QString recurrenceText(const Activity* activity) const;

    void toggleSelectedActivityCompletion();
    void deleteSelectedActivity();
    void createActivity();
    void editSelectedActivity();
    void createActivityFromTemplate();
    void saveSelectedActivityAsTemplate();

    void saveAgenda();
    void saveAgendaAs();
    void loadAgenda();

    ActivityManager* m_activityManager = nullptr;
    ActivityTemplateManager* m_templateManager = nullptr;

    CommandHistory m_commandHistory;

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