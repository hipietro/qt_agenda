// Centralizes list selection, double-click editing, and contextual pointer actions.

#ifndef ACTIVITYLISTMOUSECONTROLLER_H
#define ACTIVITYLISTMOUSECONTROLLER_H

#include <QObject>
#include <QPointer>

class QAction;
class QEvent;
class QListWidget;
class QListWidgetItem;
class QMainWindow;
class QPoint;
class QPushButton;
class QSplitter;
class QStackedWidget;
class QWidget;

/*
 * Translates standard Qt mouse and trackpad events into the same actions
 * exposed by MainWindow. Context operations first select the item under
 * the pointer so commands never target a stale list selection.
 */
class ActivityListMouseController final : public QObject
{
public:
    explicit ActivityListMouseController(QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void configureWindow(QMainWindow* window);
    void editItem(QListWidgetItem* item);
    void showContextMenu(const QPoint& position);
    void triggerButton(QPushButton* button);
    void leaveEditingWorkflow();
    void rebalanceDetailLayout();

    bool creationActive() const;
    bool editingActive() const;

    QPushButton* buttonWithText(QMainWindow* window, const QString& text) const;
    QPushButton* buttonStartingWith(QMainWindow* window, const QString& prefix) const;
    QAction* actionStartingWith(QMainWindow* window, const QString& prefix) const;

    QPointer<QMainWindow> m_window;
    QPointer<QListWidget> m_activityList;
    QPointer<QSplitter> m_mainSplitter;
    QPointer<QStackedWidget> m_workspaceStack;
    QPointer<QWidget> m_detailPage;
    QPointer<QWidget> m_creationPage;
    QPointer<QWidget> m_editingPage;
    QPointer<QPushButton> m_editButton;
    QPointer<QPushButton> m_toggleButton;
    QPointer<QPushButton> m_deleteButton;
    QPointer<QAction> m_saveAsTemplateAction;
};

#endif
