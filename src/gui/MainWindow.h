#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
class QTextEdit;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(ActivityManager* activityManager, QWidget* parent = nullptr);

private:
    void setupUi();
    void connectSignals();

    void refreshActivityList();
    void showActivityDetails(const Activity* activity);

    std::vector<const Activity*> collectVisibleActivities() const;
    const Activity* findActivityById(const QString& id) const;

    QString statusText(const Activity* activity) const;
    QString priorityText(Priority priority) const;
    QString recurrenceText(const Activity* activity) const;

    ActivityManager* m_activityManager = nullptr;

    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QListWidget* m_activityList = nullptr;
    QTextEdit* m_detailView = nullptr;
    QLabel* m_resultCountLabel = nullptr;
};

#endif