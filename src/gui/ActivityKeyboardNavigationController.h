#ifndef ACTIVITYKEYBOARDNAVIGATIONCONTROLLER_H
#define ACTIVITYKEYBOARDNAVIGATIONCONTROLLER_H

#include <QObject>
#include <QPointer>

class QAction;
class QEvent;
class QKeyEvent;
class QLineEdit;
class QListWidget;
class QMainWindow;
class QPushButton;
class QStackedWidget;
class QWidget;

class ActivityKeyboardNavigationController final : public QObject
{
public:
    explicit ActivityKeyboardNavigationController(QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void configureWindow(QMainWindow* window);
    void createActivityMenu();
    void updateActionState();
    void updateDiscoverabilityText();
    void restoreAgendaFocus();

    bool handleActivityListKeyPress(QKeyEvent* event);
    bool clickIfEnabled(QPushButton* button);
    QPushButton* buttonWithText(QMainWindow* window, const QString& text) const;
    bool detailPageActive() const;

    QPointer<QMainWindow> m_window;
    QPointer<QStackedWidget> m_workspaceStack;
    QPointer<QWidget> m_detailPage;
    QPointer<QWidget> m_creationPage;
    QPointer<QWidget> m_editingPage;
    QPointer<QWidget> m_previousWorkspacePage;

    QPointer<QListWidget> m_activityList;
    QPointer<QLineEdit> m_searchEdit;
    QPointer<QPushButton> m_addButton;
    QPointer<QPushButton> m_editButton;
    QPointer<QPushButton> m_toggleButton;
    QPointer<QPushButton> m_deleteButton;

    QPointer<QAction> m_newActivityAction;
    QPointer<QAction> m_editActivityAction;
    QPointer<QAction> m_focusSearchAction;
};

#endif
