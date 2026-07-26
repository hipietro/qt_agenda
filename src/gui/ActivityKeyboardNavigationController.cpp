#include "ActivityKeyboardNavigationController.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QWidget>

namespace {

void installActivityKeyboardNavigationController()
{
    auto* controller = new ActivityKeyboardNavigationController(qApp);
    qApp->installEventFilter(controller);
}

QString shortcutLabel(const QAction* action)
{
    return action
        ? action->shortcut().toString(QKeySequence::NativeText)
        : QString();
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(installActivityKeyboardNavigationController)

ActivityKeyboardNavigationController::ActivityKeyboardNavigationController(QObject* parent)
    : QObject(parent)
{
}

bool ActivityKeyboardNavigationController::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_window && event->type() == QEvent::Show) {
        if (auto* window = qobject_cast<QMainWindow*>(watched)) {
            configureWindow(window);
        }
    }

    if (!m_window) {
        return QObject::eventFilter(watched, event);
    }

    if ((watched == m_addButton || watched == m_editButton ||
         watched == m_toggleButton || watched == m_deleteButton) &&
        event->type() == QEvent::EnabledChange) {
        QTimer::singleShot(0, this, [this]() {
            updateActionState();
        });
    }

    if (watched == m_activityList && event->type() == QEvent::KeyPress) {
        if (handleActivityListKeyPress(static_cast<QKeyEvent*>(event))) {
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ActivityKeyboardNavigationController::configureWindow(QMainWindow* window)
{
    if (!window || m_window) {
        return;
    }

    m_window = window;
    m_workspaceStack = window->findChild<QStackedWidget*>(QStringLiteral("workspaceStack"));
    m_detailPage = window->findChild<QWidget*>(QStringLiteral("activityDetailPage"));
    m_creationPage = window->findChild<QWidget*>(QStringLiteral("activityCreationPage"));
    m_editingPage = window->findChild<QWidget*>(QStringLiteral("activityEditPage"));
    m_activityList = window->findChild<QListWidget*>(QStringLiteral("activityList"));

    const QList<QLineEdit*> lineEdits = window->findChildren<QLineEdit*>();
    for (QLineEdit* lineEdit : lineEdits) {
        if (lineEdit && lineEdit->placeholderText().startsWith(
                            QStringLiteral("Search by title"))) {
            m_searchEdit = lineEdit;
            break;
        }
    }

    m_addButton = buttonWithText(window, QStringLiteral("Add activity"));
    m_editButton = buttonWithText(window, QStringLiteral("Edit activity"));
    m_toggleButton = buttonWithText(window, QStringLiteral("Mark completed"));
    if (!m_toggleButton) {
        m_toggleButton = buttonWithText(window, QStringLiteral("Mark active"));
    }
    m_deleteButton = buttonWithText(window, QStringLiteral("Delete activity"));

    if (!m_workspaceStack || !m_detailPage || !m_creationPage ||
        !m_editingPage || !m_activityList || !m_searchEdit) {
        return;
    }

    m_previousWorkspacePage = m_workspaceStack->currentWidget();

    createActivityMenu();

    m_activityList->installEventFilter(this);
    if (m_addButton) {
        m_addButton->installEventFilter(this);
    }
    if (m_editButton) {
        m_editButton->installEventFilter(this);
    }
    if (m_toggleButton) {
        m_toggleButton->installEventFilter(this);
    }
    if (m_deleteButton) {
        m_deleteButton->installEventFilter(this);
    }

    connect(m_workspaceStack, &QStackedWidget::currentChanged,
            this, [this](int) {
                QWidget* previousPage = m_previousWorkspacePage;
                QWidget* currentPage = m_workspaceStack
                    ? m_workspaceStack->currentWidget()
                    : nullptr;
                m_previousWorkspacePage = currentPage;

                updateActionState();

                const bool returnedFromWorkflow =
                    currentPage == m_detailPage &&
                    (previousPage == m_creationPage || previousPage == m_editingPage);

                if (returnedFromWorkflow) {
                    QTimer::singleShot(0, this, [this]() {
                        restoreAgendaFocus();
                    });
                }
            });

    connect(m_activityList, &QListWidget::currentRowChanged,
            this, [this](int) {
                QTimer::singleShot(0, this, [this]() {
                    updateActionState();
                });
            });

    updateActionState();
    QTimer::singleShot(0, this, [this]() {
        updateDiscoverabilityText();
        updateActionState();
    });
}

void ActivityKeyboardNavigationController::createActivityMenu()
{
    if (!m_window || m_newActivityAction || m_editActivityAction ||
        m_focusSearchAction) {
        return;
    }

    QMenu* activityMenu = m_window->menuBar()->addMenu(QStringLiteral("Activity"));

    m_newActivityAction = activityMenu->addAction(QStringLiteral("New activity"));
    m_newActivityAction->setShortcut(QKeySequence(QKeySequence::New));
    m_newActivityAction->setShortcutContext(Qt::WindowShortcut);
    m_newActivityAction->setStatusTip(QStringLiteral("Open the activity creation page"));

    m_editActivityAction = activityMenu->addAction(
        QStringLiteral("Edit selected activity"));
    m_editActivityAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    m_editActivityAction->setShortcutContext(Qt::WindowShortcut);
    m_editActivityAction->setStatusTip(QStringLiteral("Edit the selected activity"));

    activityMenu->addSeparator();

    m_focusSearchAction = activityMenu->addAction(QStringLiteral("Focus search"));
    m_focusSearchAction->setShortcut(QKeySequence(QKeySequence::Find));
    m_focusSearchAction->setShortcutContext(Qt::WindowShortcut);
    m_focusSearchAction->setStatusTip(QStringLiteral("Focus and select the search field"));

    connect(m_newActivityAction, &QAction::triggered, this, [this]() {
        clickIfEnabled(m_addButton);
    });

    connect(m_editActivityAction, &QAction::triggered, this, [this]() {
        clickIfEnabled(m_editButton);
    });

    connect(m_focusSearchAction, &QAction::triggered, this, [this]() {
        if (!m_focusSearchAction || !m_focusSearchAction->isEnabled() ||
            !m_searchEdit) {
            return;
        }

        m_searchEdit->setFocus(Qt::ShortcutFocusReason);
        m_searchEdit->selectAll();
    });

    connect(activityMenu, &QMenu::aboutToShow, this, [this]() {
        updateActionState();
    });
}

void ActivityKeyboardNavigationController::updateActionState()
{
    const bool agendaActive = detailPageActive();

    if (m_newActivityAction) {
        m_newActivityAction->setEnabled(
            agendaActive && m_addButton && m_addButton->isEnabled());
    }

    if (m_editActivityAction) {
        m_editActivityAction->setEnabled(
            agendaActive && m_editButton && m_editButton->isEnabled());
    }

    if (m_focusSearchAction) {
        m_focusSearchAction->setEnabled(
            agendaActive && m_searchEdit && m_searchEdit->isEnabled() &&
            m_searchEdit->isVisibleTo(m_window));
    }
}

void ActivityKeyboardNavigationController::updateDiscoverabilityText()
{
    const QString newShortcut = shortcutLabel(m_newActivityAction);
    const QString editShortcut = shortcutLabel(m_editActivityAction);
    const QString findShortcut = shortcutLabel(m_focusSearchAction);

    if (m_addButton) {
        m_addButton->setToolTip(
            QStringLiteral("Create a new activity (%1)").arg(newShortcut));
        m_addButton->setStatusTip(m_addButton->toolTip());
    }

    if (m_editButton) {
        m_editButton->setToolTip(
            QStringLiteral("Edit the selected activity (%1 or Enter in the list)")
                .arg(editShortcut));
        m_editButton->setStatusTip(m_editButton->toolTip());
    }

    if (m_searchEdit) {
        m_searchEdit->setToolTip(
            QStringLiteral("Focus search (%1)").arg(findShortcut));
        m_searchEdit->setStatusTip(m_searchEdit->toolTip());
    }

    if (m_toggleButton) {
        m_toggleButton->setToolTip(
            QStringLiteral("Toggle completion (Space while the activity list is focused)"));
        m_toggleButton->setStatusTip(m_toggleButton->toolTip());
    }

    if (m_deleteButton) {
        m_deleteButton->setToolTip(
            QStringLiteral("Delete the selected activity (Delete/Backspace while the list is focused)"));
        m_deleteButton->setStatusTip(m_deleteButton->toolTip());
    }

    if (m_activityList) {
        m_activityList->setToolTip(
            QStringLiteral("Single-click to select, double-click or Enter to edit, "
                           "Space to toggle completion, Delete/Backspace to delete, "
                           "or right-click for actions."));
        m_activityList->setStatusTip(m_activityList->toolTip());
    }
}

void ActivityKeyboardNavigationController::restoreAgendaFocus()
{
    if (!detailPageActive()) {
        return;
    }

    if (m_activityList && m_activityList->isVisibleTo(m_window) &&
        m_activityList->isEnabled() && m_activityList->count() > 0) {
        m_activityList->setFocus(Qt::OtherFocusReason);
        return;
    }

    if (m_searchEdit && m_searchEdit->isVisibleTo(m_window) &&
        m_searchEdit->isEnabled()) {
        m_searchEdit->setFocus(Qt::OtherFocusReason);
    }
}

bool ActivityKeyboardNavigationController::handleActivityListKeyPress(QKeyEvent* event)
{
    if (!event || !m_window || !m_window->isActiveWindow() ||
        !detailPageActive() || event->isAutoRepeat()) {
        return false;
    }

    Qt::KeyboardModifiers modifiers = event->modifiers();
    modifiers.setFlag(Qt::KeypadModifier, false);
    if (modifiers != Qt::NoModifier) {
        return false;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return clickIfEnabled(m_editButton);

    case Qt::Key_Space:
        return clickIfEnabled(m_toggleButton);

    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        return clickIfEnabled(m_deleteButton);

    default:
        return false;
    }
}

bool ActivityKeyboardNavigationController::clickIfEnabled(QPushButton* button)
{
    if (!button || !button->isEnabled()) {
        return false;
    }

    button->click();
    return true;
}

QPushButton* ActivityKeyboardNavigationController::buttonWithText(
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

bool ActivityKeyboardNavigationController::detailPageActive() const
{
    return m_workspaceStack && m_detailPage &&
           m_workspaceStack->currentWidget() == m_detailPage;
}
