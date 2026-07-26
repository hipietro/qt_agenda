#include "ActivityListMouseController.h"

#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QWidget>

#include <algorithm>

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

    if (m_window && watched == m_window && event->type() == QEvent::Resize) {
        QTimer::singleShot(0, this, [this]() {
            rebalanceDetailLayout();
        });
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
    QWidget* creationPage =
        window->findChild<QWidget*>(QStringLiteral("activityCreationPage"));
    QWidget* editingPage =
        window->findChild<QWidget*>(QStringLiteral("activityEditPage"));
    QSplitter* mainSplitter = window->findChild<QSplitter*>();

    if (!activityList || !workspaceStack || !detailPage || !creationPage ||
        !editingPage || !mainSplitter) {
        return;
    }

    m_window = window;
    m_activityList = activityList;
    m_mainSplitter = mainSplitter;
    m_workspaceStack = workspaceStack;
    m_detailPage = detailPage;
    m_creationPage = creationPage;
    m_editingPage = editingPage;
    m_editButton = buttonWithText(window, QStringLiteral("Edit activity"));
    m_toggleButton = buttonStartingWith(window, QStringLiteral("Mark "));
    m_deleteButton = buttonWithText(window, QStringLiteral("Delete activity"));
    m_saveAsTemplateAction =
        actionStartingWith(window, QStringLiteral("Save selected as template"));

    m_activityList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_activityList->setToolTip(
        QStringLiteral("Single-click to select, double-click to edit, or right-click for actions."));

    connect(m_activityList, &QListWidget::itemClicked,
            this, [this](QListWidgetItem* item) {
                if (!item || creationActive()) {
                    return;
                }

                m_activityList->setCurrentItem(item);
                item->setSelected(true);

                /*
                 * A normal click always means "show this activity". If editing is
                 * open, cancel only the editing workflow and return to the details
                 * page for the item that Qt has already selected.
                 */
                if (editingActive()) {
                    leaveEditingWorkflow();
                }
            });

    connect(m_activityList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
                editItem(item);
            });

    connect(m_activityList, &QWidget::customContextMenuRequested,
            this, [this](const QPoint& position) {
                showContextMenu(position);
            });

    connect(m_workspaceStack, &QStackedWidget::currentChanged,
            this, [this](int) {
                QTimer::singleShot(0, this, [this]() {
                    rebalanceDetailLayout();
                });
            });

    QTimer::singleShot(0, this, [this]() {
        rebalanceDetailLayout();
    });
}

void ActivityListMouseController::editItem(QListWidgetItem* item)
{
    if (!m_activityList || !item || creationActive() || !m_editButton) {
        return;
    }

    m_activityList->setCurrentItem(item);
    item->setSelected(true);
    triggerButton(m_editButton);
}

void ActivityListMouseController::showContextMenu(const QPoint& position)
{
    if (!m_activityList || creationActive()) {
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
    editAction->setEnabled(m_editButton != nullptr);

    const QString toggleText = m_toggleButton
        ? m_toggleButton->text()
        : QStringLiteral("Mark completed");
    QAction* toggleAction = menu.addAction(toggleText);
    toggleAction->setEnabled(m_toggleButton != nullptr);

    QAction* templateAction = menu.addAction(QStringLiteral("Save as template..."));
    templateAction->setEnabled(m_saveAsTemplateAction != nullptr);

    menu.addSeparator();

    QAction* deleteAction = menu.addAction(QStringLiteral("Delete activity"));
    deleteAction->setEnabled(m_deleteButton != nullptr);

    QAction* chosenAction =
        menu.exec(m_activityList->viewport()->mapToGlobal(position));

    if (chosenAction == editAction && m_editButton) {
        triggerButton(m_editButton);
        return;
    }

    if (!chosenAction) {
        return;
    }

    /*
     * Non-edit actions first leave the active edit page. This avoids keeping a
     * pointer to an activity that could be deleted or replaced by the command.
     * The selected list item remains selected, so the existing MainWindow
     * operation still acts on the item chosen with the secondary click.
     */
    leaveEditingWorkflow();

    if (chosenAction == toggleAction && m_toggleButton) {
        triggerButton(m_toggleButton);
    } else if (chosenAction == templateAction && m_saveAsTemplateAction) {
        m_saveAsTemplateAction->trigger();
    } else if (chosenAction == deleteAction && m_deleteButton) {
        triggerButton(m_deleteButton);
    }
}

void ActivityListMouseController::triggerButton(QPushButton* button)
{
    if (!button) {
        return;
    }

    const bool wasEnabled = button->isEnabled();
    if (!wasEnabled) {
        button->setEnabled(true);
    }

    button->click();

    if (!wasEnabled && button) {
        button->setEnabled(false);
    }
}

void ActivityListMouseController::leaveEditingWorkflow()
{
    if (!editingActive() || !m_editingPage) {
        return;
    }

    QDialogButtonBox* buttonBox =
        m_editingPage->findChild<QDialogButtonBox*>();

    if (!buttonBox) {
        return;
    }

    if (QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel)) {
        cancelButton->click();
    }
}

void ActivityListMouseController::rebalanceDetailLayout()
{
    if (!m_mainSplitter || !m_workspaceStack || !m_detailPage ||
        m_workspaceStack->currentWidget() != m_detailPage) {
        return;
    }

    /*
     * The list is the primary navigation surface, while the details panel only
     * needs enough width to keep its cards readable. On large windows the old
     * fixed maximum left a wide, mostly empty details area. A proportional split
     * uses the available space more effectively without making details cramped.
     */
    const int totalWidth = std::max(1, m_mainSplitter->width());
    const int desiredLeftWidth = totalWidth * 62 / 100;
    const int maximumLeftWidth = std::max(360, totalWidth - 440);
    const int leftWidth = std::max(360, std::min(desiredLeftWidth, maximumLeftWidth));

    m_mainSplitter->setSizes({leftWidth, std::max(1, totalWidth - leftWidth)});
}

bool ActivityListMouseController::creationActive() const
{
    return m_workspaceStack && m_creationPage &&
           m_workspaceStack->currentWidget() == m_creationPage;
}

bool ActivityListMouseController::editingActive() const
{
    return m_workspaceStack && m_editingPage &&
           m_workspaceStack->currentWidget() == m_editingPage;
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
