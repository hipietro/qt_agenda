#include "ActivityWorkflowPolishController.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace {

const QString activeWorkflowStyle = QStringLiteral(
    "QPushButton {"
    " background-color: #3F51B5;"
    " color: #ffffff;"
    " border: 1px solid #303F9F;"
    " border-radius: 6px;"
    " font-weight: 600;"
    "}"
    "QPushButton:disabled {"
    " background-color: #3F51B5;"
    " color: #ffffff;"
    " border: 1px solid #303F9F;"
    "}"
);

const QString inactiveWorkflowStyle = QStringLiteral(
    "QPushButton {"
    " background-color: #e0e0e0;"
    " color: #888888;"
    " border: 1px solid #cfcfcf;"
    " border-radius: 6px;"
    "}"
    "QPushButton:disabled {"
    " background-color: #e0e0e0;"
    " color: #888888;"
    " border: 1px solid #cfcfcf;"
    "}"
);

void installActivityWorkflowPolishController()
{
    auto* controller = new ActivityWorkflowPolishController(qApp);
    qApp->installEventFilter(controller);
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(installActivityWorkflowPolishController)

ActivityWorkflowPolishController::ActivityWorkflowPolishController(QObject* parent)
    : QObject(parent)
{
}

bool ActivityWorkflowPolishController::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_window && event->type() == QEvent::Show) {
        if (auto* window = qobject_cast<QMainWindow*>(watched)) {
            configureWindow(window);
        }
    }

    if (!m_window || !m_workspaceStack || !m_window->isActiveWindow() ||
        event->type() != QEvent::KeyPress) {
        return QObject::eventFilter(watched, event);
    }

    QWidget* activePage = m_workspaceStack->currentWidget();
    if (activePage != m_creationPage && activePage != m_editingPage) {
        return QObject::eventFilter(watched, event);
    }

    if (QApplication::activePopupWidget()) {
        return QObject::eventFilter(watched, event);
    }

    const auto* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->isAutoRepeat()) {
        return QObject::eventFilter(watched, event);
    }

    QWidget* focusWidget = QApplication::focusWidget();
    QDialogButtonBox* buttonBox = buttonBoxFor(activePage);
    if (!buttonBox) {
        return QObject::eventFilter(watched, event);
    }

    if (keyEvent->key() == Qt::Key_Escape) {
        const bool inlineChecklistEditorActive =
            activePage == m_editingPage &&
            focusWidget &&
            focusWidget != m_editChecklistList &&
            isInside(focusWidget, m_editChecklistList);

        if (inlineChecklistEditorActive) {
            return QObject::eventFilter(watched, event);
        }

        if (QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel)) {
            cancelButton->click();
            return true;
        }
    }

    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
        if (qobject_cast<QPushButton*>(focusWidget)) {
            return QObject::eventFilter(watched, event);
        }

        const bool forcedSubmit =
            keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
            keyEvent->modifiers().testFlag(Qt::MetaModifier);

        if (!forcedSubmit) {
            const QList<QTextEdit*> textEditors = activePage->findChildren<QTextEdit*>();
            for (QTextEdit* editor : textEditors) {
                if (isInside(focusWidget, editor)) {
                    return QObject::eventFilter(watched, event);
                }
            }

            const QList<QListWidget*> lists = activePage->findChildren<QListWidget*>();
            for (QListWidget* list : lists) {
                if (isInside(focusWidget, list)) {
                    return QObject::eventFilter(watched, event);
                }
            }

            if (focusWidget == m_newChecklistItemEdit) {
                return QObject::eventFilter(watched, event);
            }
        }

        if (QPushButton* submitButton = buttonBox->button(QDialogButtonBox::Ok)) {
            submitButton->click();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ActivityWorkflowPolishController::configureWindow(QMainWindow* window)
{
    if (!window) {
        return;
    }

    m_window = window;
    m_workspaceStack = window->findChild<QStackedWidget*>(QStringLiteral("workspaceStack"));
    m_creationPage = window->findChild<QWidget*>(QStringLiteral("activityCreationPage"));
    m_editingPage = window->findChild<QWidget*>(QStringLiteral("activityEditPage"));
    m_addButton = buttonWithText(window, QStringLiteral("Add activity"));
    m_editButton = buttonWithText(window, QStringLiteral("Edit activity"));
    m_undoButton = buttonWithText(window, QStringLiteral("Undo"));
    m_redoButton = buttonWithText(window, QStringLiteral("Redo"));

    const QList<QAction*> actions = window->findChildren<QAction*>();
    for (QAction* action : actions) {
        if (!action) {
            continue;
        }

        if (!m_undoAction &&
            action->shortcut().matches(QKeySequence(QKeySequence::Undo)) ==
                QKeySequence::ExactMatch) {
            m_undoAction = action;
        }

        if (!m_redoAction &&
            action->shortcut().matches(QKeySequence(QKeySequence::Redo)) ==
                QKeySequence::ExactMatch) {
            m_redoAction = action;
        }
    }

    if (!m_workspaceStack || !m_creationPage || !m_editingPage) {
        return;
    }

    configureFormPage(m_creationPage, false);
    configureFormPage(m_editingPage, true);

    connect(m_workspaceStack, &QStackedWidget::currentChanged, this, [this](int) {
        updateWorkflowButtonState();

        QWidget* currentPage = m_workspaceStack ? m_workspaceStack->currentWidget() : nullptr;
        if (currentPage == m_creationPage || currentPage == m_editingPage) {
            QTimer::singleShot(0, this, [this, currentPage]() {
                compactTypeSpecificSection(currentPage);
            });
        }
    });

    if (m_undoAction) {
        connect(m_undoAction, &QAction::changed,
                this, &ActivityWorkflowPolishController::updateUndoRedoPresentation);
    }

    if (m_redoAction) {
        connect(m_redoAction, &QAction::changed,
                this, &ActivityWorkflowPolishController::updateUndoRedoPresentation);
    }

    updateWorkflowButtonState();
    updateUndoRedoPresentation();
}

void ActivityWorkflowPolishController::configureFormPage(QWidget* page, bool editingPage)
{
    if (!page) {
        return;
    }

    if (QDialogButtonBox* buttonBox = buttonBoxFor(page)) {
        if (QPushButton* submitButton = buttonBox->button(QDialogButtonBox::Ok)) {
            submitButton->setToolTip(
                editingPage
                    ? QStringLiteral("Save changes (Enter; Ctrl/Cmd+Enter in multi-line fields)")
                    : QStringLiteral("Create activity (Enter; Ctrl/Cmd+Enter in multi-line fields)"));
            submitButton->setStatusTip(submitButton->toolTip());
        }

        if (QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel)) {
            cancelButton->setToolTip(
                QStringLiteral("Cancel and return to activity details (Esc)"));
            cancelButton->setStatusTip(cancelButton->toolTip());
        }
    }

    if (editingPage) {
        const QList<QLineEdit*> lineEdits = page->findChildren<QLineEdit*>();
        for (QLineEdit* lineEdit : lineEdits) {
            if (lineEdit->placeholderText() == QStringLiteral("New checklist item")) {
                m_newChecklistItemEdit = lineEdit;
                break;
            }
        }

        const QList<QListWidget*> lists = page->findChildren<QListWidget*>();
        if (!lists.isEmpty()) {
            m_editChecklistList = lists.first();
        }

        QPushButton* addItemButton = buttonWithText(page, QStringLiteral("Add item"));
        if (m_newChecklistItemEdit && addItemButton) {
            connect(m_newChecklistItemEdit, &QLineEdit::returnPressed,
                    addItemButton, &QPushButton::click, Qt::UniqueConnection);
        }
    } else {
        const QList<QComboBox*> comboBoxes = page->findChildren<QComboBox*>();
        for (QComboBox* comboBox : comboBoxes) {
            if (comboBox->count() == 4 &&
                comboBox->itemText(0) == QStringLiteral("Event") &&
                comboBox->itemText(1) == QStringLiteral("Deadline") &&
                comboBox->itemText(2) == QStringLiteral("Reminder") &&
                comboBox->itemText(3) == QStringLiteral("Checklist")) {
                connect(comboBox, &QComboBox::currentIndexChanged, this, [this, page](int) {
                    QTimer::singleShot(0, this, [this, page]() {
                        compactTypeSpecificSection(page);
                    });
                });
                break;
            }
        }
    }

    QTimer::singleShot(0, this, [this, page]() {
        compactTypeSpecificSection(page);
    });
}

void ActivityWorkflowPolishController::compactTypeSpecificSection(QWidget* page)
{
    QGroupBox* group = typeSpecificGroup(page);
    QStackedWidget* stack = typeStack(page);

    if (!group || !stack || !stack->currentWidget()) {
        return;
    }

    group->setMaximumHeight(QWIDGETSIZE_MAX);
    group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    stack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWidget* currentPage = stack->currentWidget();
    currentPage->ensurePolished();

    if (currentPage->layout()) {
        currentPage->layout()->activate();
    }

    const int stackHeight = std::max(96, currentPage->sizeHint().height() + 8);
    stack->setFixedHeight(stackHeight);

    if (group->layout()) {
        group->layout()->activate();
    }

    group->setMaximumHeight(group->sizeHint().height());
    group->updateGeometry();
}

void ActivityWorkflowPolishController::updateWorkflowButtonState()
{
    if (!m_workspaceStack) {
        return;
    }

    const bool creationActive = m_workspaceStack->currentWidget() == m_creationPage;
    const bool editingActive = m_workspaceStack->currentWidget() == m_editingPage;
    const bool workflowActive = creationActive || editingActive;

    if (m_addButton) {
        if (!workflowActive) {
            m_addButton->setStyleSheet(QString());
        } else {
            m_addButton->setStyleSheet(
                creationActive ? activeWorkflowStyle : inactiveWorkflowStyle);
        }
    }

    if (m_editButton) {
        if (!workflowActive) {
            m_editButton->setStyleSheet(QString());
        } else {
            m_editButton->setStyleSheet(
                editingActive ? activeWorkflowStyle : inactiveWorkflowStyle);
        }
    }
}

void ActivityWorkflowPolishController::updateUndoRedoPresentation()
{
    const auto synchronizeButton = [](QPushButton* button,
                                      QAction* action,
                                      const QString& fallbackText) {
        if (!button) {
            return;
        }

        QString text = action ? action->text() : fallbackText;
        text.remove(QLatin1Char('&'));
        if (text.trimmed().isEmpty()) {
            text = fallbackText;
        }

        button->setText(text);
        button->setToolTip(text);
        button->setStatusTip(text);
    };

    synchronizeButton(m_undoButton, m_undoAction, QStringLiteral("Undo"));
    synchronizeButton(m_redoButton, m_redoAction, QStringLiteral("Redo"));
}

QDialogButtonBox* ActivityWorkflowPolishController::buttonBoxFor(QWidget* page) const
{
    return page ? page->findChild<QDialogButtonBox*>() : nullptr;
}

QPushButton* ActivityWorkflowPolishController::buttonWithText(QWidget* root,
                                                               const QString& text) const
{
    if (!root) {
        return nullptr;
    }

    const QList<QPushButton*> buttons = root->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button && button->text() == text) {
            return button;
        }
    }

    return nullptr;
}

QGroupBox* ActivityWorkflowPolishController::typeSpecificGroup(QWidget* page) const
{
    if (!page) {
        return nullptr;
    }

    const QList<QGroupBox*> groups = page->findChildren<QGroupBox*>();
    for (QGroupBox* group : groups) {
        if (group && group->title() == QStringLiteral("Type-specific fields")) {
            return group;
        }
    }

    return nullptr;
}

QStackedWidget* ActivityWorkflowPolishController::typeStack(QWidget* page) const
{
    QGroupBox* group = typeSpecificGroup(page);
    return group ? group->findChild<QStackedWidget*>() : nullptr;
}

bool ActivityWorkflowPolishController::isInside(QWidget* widget,
                                                 const QWidget* container) const
{
    while (widget) {
        if (widget == container) {
            return true;
        }
        widget = widget->parentWidget();
    }

    return false;
}
