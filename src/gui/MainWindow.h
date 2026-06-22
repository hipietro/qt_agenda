#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QString>

#include <vector>

#include "model/Activity.h"
#include "model/ActivityKind.h"
#include "model/Priority.h"

class ActivityManager;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTextEdit;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(ActivityManager* activityManager, QWidget* parent = nullptr);
protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void setupMenuBar();
    void connectSignals();

    void refreshActivityList();
    void showActivityDetails(const Activity* activity);
    void updateActionButtons();
    void updateWindowTitle();
    void setUnsavedChanges(bool hasUnsavedChanges);
    bool confirmDiscardUnsavedChanges();

    std::vector<const Activity*> collectVisibleActivities() const;
    const Activity* findActivityById(const QString& id) const;
    QString selectedActivityId() const;

    QString statusText(const Activity* activity) const;
    QString priorityText(Priority priority) const;
    QString recurrenceText(const Activity* activity) const;

    void toggleSelectedActivityCompletion();
    void deleteSelectedActivity();
    void createActivity();
    void editSelectedActivity();

    void saveAgenda();
    void saveAgendaAs();
    void loadAgenda();

    ActivityManager* m_activityManager = nullptr;

    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QListWidget* m_activityList = nullptr;
    QTextEdit* m_detailView = nullptr;
    QLabel* m_resultCountLabel = nullptr;

    QPushButton* m_addButton = nullptr;
    QPushButton* m_toggleCompletedButton = nullptr;
    QPushButton* m_deleteButton = nullptr;
    QPushButton* m_editButton = nullptr;

    QString m_currentFilePath;
    bool m_hasUnsavedChanges = false;
};

#endif