#include "ActivityListMouseController.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

namespace {

void installActivityListMouseController()
{
    auto* controller = new ActivityListMouseController(qApp);
    qApp->installEventFilter(controller);
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(installActivityListMouseController)

ActivityListMouseController::ActivityListMouseController(QObject* parent)
    : QObject(parent)
{
}

bool ActivityListMouseController::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_window && event->type() == QEvent::Show) {
        if (auto* window = qobject_cast<QMainWindow*>(watched)) {
            configureWindow(window);
        }
    }

    return QObject::eventFilter(watched, event);
}

void ActivityListMouseController::configureWindow(QMainWindow* window)
{
    if (!window || m_window) {
        return;
    }

    QListWidget* activityList =
        window->findChild<QListWidget*>(QStringLiteral("activityList"));
    QStackedWidget* workspaceStack =
        window->findChild<QStackedWidget*>(QStringLiteral("workspaceStack"));
    QWidget* detailPage =
        window->findChild<QWidget*>(QStringLiteral("activityDetailPage"));

    if (!activityList || !workspaceStack || !detailPage) {
        return;
    }

    m_window = window;
    m_activityList = activityList;
    m_workspaceStack = workspaceStack;
    m_detailPage = detailPage;
    m_editButton = buttonWithText(window, QStringLiteral("Edit activity"));
    m_toggleButton = buttonStartingWith(window, QStringLiteral("Mark "));
    m_deleteButton = buttonWithText(window, QStringLiteral("Delete activity"));
    m_saveAsTemplateAction =
        actionStartingWith(window, QStringLiteral("Save selected as template"));

    m_activityList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_activityList->setToolTip(
        QStringLiteral("Single-click to select, double-click to edit, or right-click for actions."));

    connect(m_activityList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
                editItem(item);
            });

    connect(m_activityList, &QWidget::customContextMenuRequested,
            this, [this](const QPoint& position) {
                showContextMenu(position);
            });
}

void ActivityListMouseController::editItem(QListWidgetItem* item)
{
    if (!m_activityList || !item || workflowActive() || !m_editButton ||
        !m_editButton->isEnabled()) {
        return;
    }

    m_activityList->setCurrentItem(item);
    item->setSelected(true);
    m_editButton->click();
}

void ActivityListMouseController::showContextMenu(const QPoint& position)
{
    if (!m_activityList || workflowActive()) {
        return;
    }

    QListWidgetItem* item = m_activityList->itemAt(position);

    if (!item) {
        m_activityList->clearSelection();
        m_activityList->setCurrentItem(nullptr);
        return;
    }

    m_activityList->setCurrentItem(item);
    item->setSelected(true);

    QMenu menu(m_activityList);

    QAction* editAction = menu.addAction(QStringLiteral("Edit activity"));
    editAction->setEnabled(m_editButton && m_editButton->isEnabled());

    const QString toggleText = m_toggleButton
        ? m_toggleButton->text()
        : QStringLiteral("Mark completed");
    QAction* toggleAction = menu.addAction(toggleText);
    toggleAction->setEnabled(m_toggleButton && m_toggleButton->isEnabled());

    QAction* templateAction = menu.addAction(QStringLiteral("Save as template..."));
    templateAction->setEnabled(m_saveAsTemplateAction != nullptr);

    menu.addSeparator();

    QAction* deleteAction = menu.addAction(QStringLiteral("Delete activity"));
    deleteAction->setEnabled(m_deleteButton && m_deleteButton->isEnabled());

    QAction* chosenAction =
        menu.exec(m_activityList->viewport()->mapToGlobal(position));

    if (chosenAction == editAction && m_editButton) {
        m_editButton->click();
    } else if (chosenAction == toggleAction && m_toggleButton) {
        m_toggleButton->click();
    } else if (chosenAction == templateAction && m_saveAsTemplateAction) {
        m_saveAsTemplateAction->trigger();
    } else if (chosenAction == deleteAction && m_deleteButton) {
        m_deleteButton->click();
    }
}

bool ActivityListMouseController::workflowActive() const
{
    return !m_workspaceStack || !m_detailPage ||
           m_workspaceStack->currentWidget() != m_detailPage;
}

QPushButton* ActivityListMouseController::buttonWithText(
    QMainWindow* window,
    const QString& text) const
{
    if (!window) {
        return nullptr;
    }

    const QList<QPushButton*> buttons = window->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button && button->text() == text) {
            return button;
        }
    }

    return nullptr;
}

QPushButton* ActivityListMouseController::buttonStartingWith(
    QMainWindow* window,
    const QString& prefix) const
{
    if (!window) {
        return nullptr;
    }

    const QList<QPushButton*> buttons = window->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button && button->text().startsWith(prefix)) {
            return button;
        }
    }

    return nullptr;
}

QAction* ActivityListMouseController::actionStartingWith(
    QMainWindow* window,
    const QString& prefix) const
{
    if (!window) {
        return nullptr;
    }

    const QList<QAction*> actions = window->findChildren<QAction*>();
    for (QAction* action : actions) {
        if (action && action->text().startsWith(prefix)) {
            return action;
        }
    }

    return nullptr;
}
