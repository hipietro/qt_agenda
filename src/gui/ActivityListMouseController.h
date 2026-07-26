#ifndef ACTIVITYLISTMOUSECONTROLLER_H
#define ACTIVITYLISTMOUSECONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QString>

class QAction;
class QEvent;
class QListWidget;
class QListWidgetItem;
class QMainWindow;
class QPoint;
class QPushButton;
class QStackedWidget;
class QWidget;

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
    bool workflowActive() const;

    QPushButton* buttonWithText(QMainWindow* window, const QString& text) const;
    QPushButton* buttonStartingWith(QMainWindow* window, const QString& prefix) const;
    QAction* actionStartingWith(QMainWindow* window, const QString& prefix) const;

    QPointer<QMainWindow> m_window;
    QPointer<QListWidget> m_activityList;
    QPointer<QStackedWidget> m_workspaceStack;
    QPointer<QWidget> m_detailPage;
    QPointer<QPushButton> m_editButton;
    QPointer<QPushButton> m_toggleButton;
    QPointer<QPushButton> m_deleteButton;
    QPointer<QAction> m_saveAsTemplateAction;
};

#endif
