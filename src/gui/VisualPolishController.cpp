#include "VisualPolishController.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSize>
#include <QSplitter>
#include <QStyle>
#include <QWidget>

namespace {

void installVisualPolishController()
{
    auto* controller = new VisualPolishController(qApp);
    qApp->installEventFilter(controller);
}

void refreshStyle(QWidget* widget)
{
    if (!widget || !widget->style()) {
        return;
    }

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

QString normalizedActionText(const QAction* action)
{
    if (!action) {
        return QString();
    }

    QString text = action->text();
    text.remove(QLatin1Char('&'));
    return text.trimmed();
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(installVisualPolishController)

VisualPolishController::VisualPolishController(QObject* parent)
    : QObject(parent)
{
}

bool VisualPolishController::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_window && event && event->type() == QEvent::Show) {
        if (auto* window = qobject_cast<QMainWindow*>(watched)) {
            configureWindow(window);
        }
    }

    return QObject::eventFilter(watched, event);
}

void VisualPolishController::configureWindow(QMainWindow* window)
{
    if (!window || m_window) {
        return;
    }

    m_window = window;

    configureSplitters(window);
    configureActionButtons(window);
    configureCalendarButtons(window);
    configureSearchField(window);
    configureMenuIcons(window);
}

void VisualPolishController::configureSplitters(QMainWindow* window) const
{
    const QList<QSplitter*> splitters = window->findChildren<QSplitter*>();

    for (QSplitter* splitter : splitters) {
        if (!splitter) {
            continue;
        }

        splitter->setOpaqueResize(true);

        if (splitter->orientation() == Qt::Horizontal) {
            splitter->setObjectName(QStringLiteral("mainWorkspaceSplitter"));
            splitter->setHandleWidth(9);
        } else {
            splitter->setObjectName(QStringLiteral("detailOverviewSplitter"));
            splitter->setHandleWidth(8);
        }

        refreshStyle(splitter);
    }
}

void VisualPolishController::configureActionButtons(QMainWindow* window) const
{
    QPushButton* addButton = buttonWithText(window, QStringLiteral("Add activity"));
    QPushButton* editButton = buttonWithText(window, QStringLiteral("Edit activity"));
    QPushButton* templateButton = buttonWithText(window, QStringLiteral("From template"));
    QPushButton* toggleButton = buttonStartingWith(window, QStringLiteral("Mark "));
    QPushButton* deleteButton = buttonWithText(window, QStringLiteral("Delete activity"));
    QPushButton* undoButton = buttonStartingWith(window, QStringLiteral("Undo"));
    QPushButton* redoButton = buttonStartingWith(window, QStringLiteral("Redo"));

    const auto configureButton = [](QPushButton* button,
                                    const QString& iconPath,
                                    const QSize& iconSize = QSize(16, 16)) {
        if (!button) {
            return;
        }

        button->setProperty("compactAction", true);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(iconSize);
        refreshStyle(button);
    };

    configureButton(addButton, QStringLiteral(":/icons/action_add_white.svg"));
    configureButton(editButton, QStringLiteral(":/icons/action_edit.svg"));
    configureButton(templateButton, QStringLiteral(":/icons/action_template.svg"));
    configureButton(toggleButton, QStringLiteral(":/icons/action_complete.svg"));
    configureButton(deleteButton, QStringLiteral(":/icons/action_delete.svg"));
    configureButton(undoButton, QStringLiteral(":/icons/action_undo.svg"));
    configureButton(redoButton, QStringLiteral(":/icons/action_redo.svg"));

    const QList<QGridLayout*> gridLayouts = window->findChildren<QGridLayout*>();
    for (QGridLayout* layout : gridLayouts) {
        if (!layout || !editButton || !deleteButton) {
            continue;
        }

        if (layout->indexOf(editButton) >= 0 && layout->indexOf(deleteButton) >= 0) {
            layout->setHorizontalSpacing(7);
            layout->setVerticalSpacing(6);
            break;
        }
    }
}

void VisualPolishController::configureCalendarButtons(QMainWindow* window) const
{
    QPushButton* previousButton = buttonWithText(window, QStringLiteral("‹"));
    QPushButton* todayButton = buttonWithText(window, QStringLiteral("Today"));
    QPushButton* nextButton = buttonWithText(window, QStringLiteral("›"));

    const auto configureNavigationButton = [](QPushButton* button,
                                              const QString& iconPath,
                                              const QString& objectName) {
        if (!button) {
            return;
        }

        button->setObjectName(objectName);
        button->setProperty("calendarControl", true);
        button->setText(QString());
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(16, 16));
        button->setFixedSize(32, 32);
        refreshStyle(button);
    };

    configureNavigationButton(previousButton,
                              QStringLiteral(":/icons/chevron_left.svg"),
                              QStringLiteral("monthNavigationButton"));
    configureNavigationButton(nextButton,
                              QStringLiteral(":/icons/chevron_right.svg"),
                              QStringLiteral("monthNavigationButton"));

    if (todayButton) {
        todayButton->setObjectName(QStringLiteral("monthTodayButton"));
        todayButton->setProperty("calendarControl", true);
        todayButton->setIcon(QIcon(QStringLiteral(":/icons/calendar.svg")));
        todayButton->setIconSize(QSize(15, 15));
        todayButton->setFixedSize(82, 32);
        refreshStyle(todayButton);
    }
}

void VisualPolishController::configureSearchField(QMainWindow* window) const
{
    const QList<QLineEdit*> lineEdits = window->findChildren<QLineEdit*>();

    for (QLineEdit* lineEdit : lineEdits) {
        if (!lineEdit || !lineEdit->placeholderText().startsWith(
                             QStringLiteral("Search by title"))) {
            continue;
        }

        lineEdit->setObjectName(QStringLiteral("activitySearchField"));
        lineEdit->setClearButtonEnabled(true);
        lineEdit->addAction(QIcon(QStringLiteral(":/icons/search.svg")),
                            QLineEdit::LeadingPosition);
        refreshStyle(lineEdit);
        break;
    }
}

void VisualPolishController::configureMenuIcons(QMainWindow* window) const
{
    const QList<QAction*> actions = window->findChildren<QAction*>();

    for (QAction* action : actions) {
        const QString text = normalizedActionText(action);

        if (text == QStringLiteral("Load...")) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_open.svg")));
        } else if (text == QStringLiteral("Save") ||
                   text == QStringLiteral("Save As...")) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_save.svg")));
        } else if (text == QStringLiteral("Create from template...") ||
                   text == QStringLiteral("Save selected as template...")) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_template.svg")));
        } else if (text == QStringLiteral("Manage categories...")) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_category.svg")));
        } else if (text == QStringLiteral("New activity")) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_add.svg")));
        } else if (text == QStringLiteral("Edit selected activity")) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_edit.svg")));
        } else if (action->shortcut().matches(QKeySequence(QKeySequence::Undo)) ==
                   QKeySequence::ExactMatch) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_undo.svg")));
        } else if (action->shortcut().matches(QKeySequence(QKeySequence::Redo)) ==
                   QKeySequence::ExactMatch) {
            action->setIcon(QIcon(QStringLiteral(":/icons/action_redo.svg")));
        }
    }
}

QPushButton* VisualPolishController::buttonWithText(QWidget* root,
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

QPushButton* VisualPolishController::buttonStartingWith(QWidget* root,
                                                         const QString& prefix) const
{
    if (!root) {
        return nullptr;
    }

    const QList<QPushButton*> buttons = root->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button && button->text().startsWith(prefix)) {
            return button;
        }
    }

    return nullptr;
}
