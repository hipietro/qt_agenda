#include <QtTest>

#include "gui/MainWindow.h"
#include "model/ActivityManager.h"
#include "model/ActivityTemplateManager.h"
#include "model/CategoryManager.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"

#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimeZone>
#include <QTimer>

#include <memory>

namespace {

QDateTime utcDateTime(int month, int day, int hour, int minute = 0)
{
    return QDateTime(QDate(2026, month, day),
                     QTime(hour, minute),
                     QTimeZone::UTC);
}

std::unique_ptr<EventActivity> makeEvent()
{
    return std::make_unique<EventActivity>(
        QStringLiteral("Architecture lecture"),
        utcDateTime(7, 27, 14),
        utcDateTime(7, 27, 16),
        QStringLiteral("Room A"),
        QStringList{QStringLiteral("Pietro")},
        QStringLiteral("Visitor pattern"),
        QStringLiteral("Study"),
        Priority::High,
        false,
        QStringLiteral("gui-event"));
}

std::unique_ptr<DeadlineActivity> makeDeadline()
{
    return std::make_unique<DeadlineActivity>(
        QStringLiteral("Register for exam"),
        utcDateTime(8, 2, 14, 30),
        QStringLiteral("University"),
        true,
        QStringLiteral("Submit registration"),
        QStringLiteral("Study"),
        Priority::Critical,
        false,
        QStringLiteral("gui-deadline"));
}

QPushButton* buttonWithText(QWidget* root, const QString& text)
{
    for (QPushButton* button : root->findChildren<QPushButton*>()) {
        if (button && button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

void closeContextMenus(QListWidget* list)
{
    if (!list) {
        return;
    }

    for (QMenu* menu : list->findChildren<QMenu*>()) {
        if (menu) {
            menu->close();
        }
    }
}

} // namespace

class GuiInteractionTests final : public QObject
{
    Q_OBJECT

private slots:
    void internalPagesAndContextMenu();
};

void GuiInteractionTests::internalPagesAndContextMenu()
{
    ActivityManager activities;
    ActivityTemplateManager templates;
    CategoryManager categories;

    QVERIFY(activities.addActivity(makeEvent()));
    QVERIFY(activities.addActivity(makeDeadline()));

    MainWindow window(&activities, &templates, &categories);
    QListWidget* list =
        window.findChild<QListWidget*>(QStringLiteral("activityList"));
    QStackedWidget* workspace =
        window.findChild<QStackedWidget*>(QStringLiteral("workspaceStack"));

    QVERIFY(list != nullptr);
    QVERIFY(workspace != nullptr);

    window.resize(1100, 760);
    window.show();
    QTest::qWait(100);

    QTRY_COMPARE(list->count(), 2);
    QCOMPARE(workspace->currentWidget()->objectName(),
             QStringLiteral("activityDetailPage"));

    QPushButton* addButton = buttonWithText(&window, QStringLiteral("Add activity"));
    QVERIFY(addButton != nullptr);
    QTest::mouseClick(addButton, Qt::LeftButton);
    QTRY_COMPARE(workspace->currentWidget()->objectName(),
                 QStringLiteral("activityCreationPage"));
    QVERIFY(QApplication::activeModalWidget() == nullptr);

    QDialogButtonBox* creationButtons =
        workspace->currentWidget()->findChild<QDialogButtonBox*>();
    QVERIFY(creationButtons != nullptr);
    QVERIFY(creationButtons->button(QDialogButtonBox::Cancel) != nullptr);
    creationButtons->button(QDialogButtonBox::Cancel)->click();
    QTRY_COMPARE(workspace->currentWidget()->objectName(),
                 QStringLiteral("activityDetailPage"));

    QListWidgetItem* first = list->item(0);
    QListWidgetItem* second = list->item(1);
    QVERIFY(first != nullptr);
    QVERIFY(second != nullptr);

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier,
                      list->visualItemRect(first).center());
    QTRY_COMPARE(list->currentRow(), 0);

    QTest::mouseDClick(list->viewport(), Qt::LeftButton, Qt::NoModifier,
                       list->visualItemRect(first).center());
    QTRY_COMPARE(workspace->currentWidget()->objectName(),
                 QStringLiteral("activityEditPage"));
    QVERIFY(QApplication::activeModalWidget() == nullptr);

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier,
                      list->visualItemRect(second).center());
    QTRY_COMPARE(workspace->currentWidget()->objectName(),
                 QStringLiteral("activityDetailPage"));
    QTRY_COMPARE(list->currentRow(), 1);

    bool contextActionTriggered = false;
    QPointer<QListWidget> guardedList(list);

    QTimer::singleShot(50, qApp, [guardedList, &contextActionTriggered]() {
        if (!guardedList) {
            return;
        }

        for (QMenu* menu : guardedList->findChildren<QMenu*>()) {
            if (!menu) {
                continue;
            }

            for (QAction* action : menu->actions()) {
                if (action && action->text().startsWith(QStringLiteral("Mark completed"))) {
                    contextActionTriggered = true;
                    action->trigger();
                    return;
                }
            }

            menu->close();
        }
    });

    // A safety close guarantees that a platform-specific native menu can never
    // leave the test process blocked indefinitely.
    QTimer::singleShot(1500, qApp, [guardedList]() {
        closeContextMenus(guardedList);
    });

    const QPoint secondPosition = list->visualItemRect(second).center();
    QVERIFY(QMetaObject::invokeMethod(
        list,
        "customContextMenuRequested",
        Qt::DirectConnection,
        Q_ARG(QPoint, secondPosition)));

    QVERIFY(contextActionTriggered);
    QTRY_VERIFY(activities.findActivityById(
                    QStringLiteral("gui-deadline"))->isCompleted());

    list->setMinimumHeight(360);
    window.resize(1100, 900);
    QCoreApplication::processEvents();

    const QRect lastRect = list->visualItemRect(list->item(list->count() - 1));
    const QPoint emptyPoint(
        10,
        qMin(list->viewport()->height() - 4, lastRect.bottom() + 20));

    QVERIFY(list->itemAt(emptyPoint) == nullptr);
    QVERIFY(QMetaObject::invokeMethod(
        list,
        "customContextMenuRequested",
        Qt::DirectConnection,
        Q_ARG(QPoint, emptyPoint)));
    QTRY_VERIFY(list->currentItem() == nullptr);
}

QTEST_MAIN(GuiInteractionTests)
#include "GuiInteractionTests.moc"
