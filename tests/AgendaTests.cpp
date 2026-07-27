#include <QtTest>

#include "commands/AddActivityCommand.h"
#include "commands/CommandHistory.h"
#include "commands/RemoveActivityCommand.h"
#include "commands/ToggleCompletionCommand.h"
#include "commands/UpdateActivityCommand.h"
#include "gui/ActivityDetailVisitor.h"
#include "gui/ActivityListItemVisitor.h"
#include "gui/MainWindow.h"
#include "model/ActivityFilter.h"
#include "model/ActivityManager.h"
#include "model/ActivityTemplate.h"
#include "model/ActivityTemplateManager.h"
#include "model/ActivityVisitor.h"
#include "model/CategoryManager.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"
#include "model/SearchEngine.h"
#include "persistence/ActivityJsonSerializer.h"
#include "persistence/AgendaJsonStorage.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QTimer>

#include <memory>
#include <vector>

namespace {

QDateTime dateTime(int year, int month, int day, int hour, int minute)
{
    QDateTime result(QDate(year, month, day), QTime(hour, minute));
    result.setTimeSpec(Qt::UTC);
    return result;
}

std::unique_ptr<EventActivity> makeEvent(const QString& id = QStringLiteral("event-id"))
{
    auto activity = std::make_unique<EventActivity>(
        QStringLiteral("Object-Oriented Programming lecture"),
        dateTime(2026, 7, 27, 14, 30),
        dateTime(2026, 7, 27, 16, 30),
        QStringLiteral("Room A"),
        QStringList{QStringLiteral("Pietro"), QStringLiteral("Alice")},
        QStringLiteral("Visitor pattern lecture"),
        QStringLiteral("Study"),
        Priority::High,
        false,
        id,
        dateTime(2026, 7, 20, 10, 0),
        dateTime(2026, 7, 21, 11, 0));

    activity->setRecurrenceRule(RecurrenceRule(
        RecurrenceRule::Frequency::Weekly,
        2,
        RecurrenceRule::EndMode::AfterOccurrences,
        QDateTime(),
        5));
    return activity;
}

std::unique_ptr<DeadlineActivity> makeDeadline(const QString& id = QStringLiteral("deadline-id"))
{
    return std::make_unique<DeadlineActivity>(
        QStringLiteral("Register for OOP exam"),
        dateTime(2026, 8, 2, 14, 30),
        QStringLiteral("University exam"),
        true,
        QStringLiteral("Submit exam registration"),
        QStringLiteral("Study"),
        Priority::Critical,
        false,
        id,
        dateTime(2026, 7, 20, 10, 0),
        dateTime(2026, 7, 22, 12, 0));
}

std::unique_ptr<ReminderActivity> makeReminder(const QString& id = QStringLiteral("reminder-id"))
{
    return std::make_unique<ReminderActivity>(
        QStringLiteral("Call the doctor"),
        dateTime(2026, 7, 28, 16, 45),
        15,
        QStringLiteral("Ask about appointment confirmation"),
        QStringLiteral("Personal reminder"),
        QStringLiteral("Health"),
        Priority::Medium,
        true,
        id,
        dateTime(2026, 7, 20, 10, 0),
        dateTime(2026, 7, 23, 13, 0));
}

std::unique_ptr<ChecklistActivity> makeChecklist(const QString& id = QStringLiteral("checklist-id"))
{
    const QVector<ChecklistItem> items{
        {QStringLiteral("Read chapter"), true},
        {QStringLiteral("Solve exercises"), false},
        {QStringLiteral("Review notes"), false}
    };

    return std::make_unique<ChecklistActivity>(
        QStringLiteral("Prepare study session"),
        dateTime(2026, 7, 29, 14, 30),
        items,
        QStringLiteral("Prepare all study material"),
        QStringLiteral("Study"),
        Priority::Medium,
        false,
        id,
        dateTime(2026, 7, 20, 10, 0),
        dateTime(2026, 7, 24, 14, 0));
}

std::vector<std::unique_ptr<Activity>> makeAllActivities()
{
    std::vector<std::unique_ptr<Activity>> activities;
    activities.push_back(makeEvent());
    activities.push_back(makeDeadline());
    activities.push_back(makeReminder());
    activities.push_back(makeChecklist());
    return activities;
}

void compareRecurrence(const Activity& expected, const Activity& actual)
{
    QCOMPARE(actual.hasRecurrence(), expected.hasRecurrence());
    if (!expected.hasRecurrence()) {
        return;
    }

    QVERIFY(actual.recurrenceRule().has_value());
    const RecurrenceRule expectedRule = expected.recurrenceRule().value();
    const RecurrenceRule actualRule = actual.recurrenceRule().value();
    QCOMPARE(static_cast<int>(actualRule.frequency()), static_cast<int>(expectedRule.frequency()));
    QCOMPARE(actualRule.interval(), expectedRule.interval());
    QCOMPARE(static_cast<int>(actualRule.endMode()), static_cast<int>(expectedRule.endMode()));
    QCOMPARE(actualRule.untilDate(), expectedRule.untilDate());
    QCOMPARE(actualRule.maxOccurrences(), expectedRule.maxOccurrences());
}

void compareActivities(const Activity& expected, const Activity& actual)
{
    QCOMPARE(actual.id(), expected.id());
    QCOMPARE(actual.title(), expected.title());
    QCOMPARE(actual.description(), expected.description());
    QCOMPARE(actual.category(), expected.category());
    QCOMPARE(static_cast<int>(actual.priority()), static_cast<int>(expected.priority()));
    QCOMPARE(actual.isCompleted(), expected.isCompleted());
    QCOMPARE(actual.createdAt(), expected.createdAt());
    QCOMPARE(actual.updatedAt(), expected.updatedAt());
    compareRecurrence(expected, actual);

    if (const auto* expectedEvent = dynamic_cast<const EventActivity*>(&expected)) {
        const auto* actualEvent = dynamic_cast<const EventActivity*>(&actual);
        QVERIFY(actualEvent != nullptr);
        QCOMPARE(actualEvent->startDateTime(), expectedEvent->startDateTime());
        QCOMPARE(actualEvent->endDateTime(), expectedEvent->endDateTime());
        QCOMPARE(actualEvent->location(), expectedEvent->location());
        QCOMPARE(actualEvent->participants(), expectedEvent->participants());
        return;
    }

    if (const auto* expectedDeadline = dynamic_cast<const DeadlineActivity*>(&expected)) {
        const auto* actualDeadline = dynamic_cast<const DeadlineActivity*>(&actual);
        QVERIFY(actualDeadline != nullptr);
        QCOMPARE(actualDeadline->dueDate(), expectedDeadline->dueDate());
        QCOMPARE(actualDeadline->context(), expectedDeadline->context());
        QCOMPARE(actualDeadline->isHardDeadline(), expectedDeadline->isHardDeadline());
        return;
    }

    if (const auto* expectedReminder = dynamic_cast<const ReminderActivity*>(&expected)) {
        const auto* actualReminder = dynamic_cast<const ReminderActivity*>(&actual);
        QVERIFY(actualReminder != nullptr);
        QCOMPARE(actualReminder->reminderDateTime(), expectedReminder->reminderDateTime());
        QCOMPARE(actualReminder->advanceMinutes(), expectedReminder->advanceMinutes());
        QCOMPARE(actualReminder->reminderNote(), expectedReminder->reminderNote());
        return;
    }

    const auto* expectedChecklist = dynamic_cast<const ChecklistActivity*>(&expected);
    const auto* actualChecklist = dynamic_cast<const ChecklistActivity*>(&actual);
    QVERIFY(expectedChecklist != nullptr);
    QVERIFY(actualChecklist != nullptr);
    QCOMPARE(actualChecklist->dueDate(), expectedChecklist->dueDate());
    QCOMPARE(actualChecklist->items().size(), expectedChecklist->items().size());
    for (int index = 0; index < expectedChecklist->items().size(); ++index) {
        QCOMPARE(actualChecklist->items().at(index).text,
                 expectedChecklist->items().at(index).text);
        QCOMPARE(actualChecklist->items().at(index).completed,
                 expectedChecklist->items().at(index).completed);
    }
}

QString widgetText(QWidget* widget)
{
    QStringList text;
    const QList<QLabel*> labels = widget->findChildren<QLabel*>();
    for (const QLabel* label : labels) {
        text.append(label->text());
    }
    return text.join(QLatin1Char('\n'));
}

QPushButton* buttonWithText(QWidget* root, const QString& text)
{
    const QList<QPushButton*> buttons = root->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

class RecordingVisitor final : public ActivityVisitor
{
public:
    void visit(const EventActivity&) override { visited.append(QStringLiteral("event")); }
    void visit(const DeadlineActivity&) override { visited.append(QStringLiteral("deadline")); }
    void visit(const ReminderActivity&) override { visited.append(QStringLiteral("reminder")); }
    void visit(const ChecklistActivity&) override { visited.append(QStringLiteral("checklist")); }

    QStringList visited;
};

} // namespace

class AgendaTests final : public QObject
{
    Q_OBJECT

private slots:
    void visitorDoubleDispatchCoversEveryConcreteType();
    void visitorsSelectTypeSpecificListAndDetailRenderers();
    void activityJsonRoundTripPreservesAllFields();
    void malformedAndUnknownJsonFailsSafely();
    void agendaFileRoundTripPreservesActivitiesTemplatesAndCategories();
    void commandHistorySupportsRepeatedUndoRedoAndBranchTruncation();
    void filteringSortingAndSearchingRemainStable();
    void mainWorkflowsStayInsideMainWindowAndMouseNavigationWorks();
};

void AgendaTests::visitorDoubleDispatchCoversEveryConcreteType()
{
    RecordingVisitor visitor;
    const auto activities = makeAllActivities();

    for (const std::unique_ptr<Activity>& activity : activities) {
        const Activity& baseReference = *activity;
        baseReference.accept(visitor);
    }

    QCOMPARE(visitor.visited,
             QStringList({QStringLiteral("event"),
                          QStringLiteral("deadline"),
                          QStringLiteral("reminder"),
                          QStringLiteral("checklist")}));
}

void AgendaTests::visitorsSelectTypeSpecificListAndDetailRenderers()
{
    const auto activities = makeAllActivities();
    const QStringList typeNames{
        QStringLiteral("EVENT"),
        QStringLiteral("DEADLINE"),
        QStringLiteral("REMINDER"),
        QStringLiteral("CHECKLIST")
    };
    const QStringList detailSections{
        QStringLiteral("Event schedule"),
        QStringLiteral("Deadline status"),
        QStringLiteral("Reminder settings"),
        QStringLiteral("Checklist progress")
    };

    for (int index = 0; index < static_cast<int>(activities.size()); ++index) {
        const Activity& baseReference = *activities.at(index);

        ActivityListItemVisitor listVisitor;
        baseReference.accept(listVisitor);
        std::unique_ptr<QWidget> listWidget(listVisitor.takeWidget());
        QVERIFY(listWidget != nullptr);
        QVERIFY2(widgetText(listWidget.get()).contains(typeNames.at(index)),
                 qPrintable(QStringLiteral("Missing list renderer for %1").arg(typeNames.at(index))));

        ActivityDetailVisitor detailVisitor;
        baseReference.accept(detailVisitor);
        std::unique_ptr<QWidget> detailWidget(detailVisitor.takeWidget());
        QVERIFY(detailWidget != nullptr);
        QVERIFY2(widgetText(detailWidget.get()).contains(detailSections.at(index)),
                 qPrintable(QStringLiteral("Missing detail renderer section %1")
                                .arg(detailSections.at(index))));
    }
}

void AgendaTests::activityJsonRoundTripPreservesAllFields()
{
    const auto activities = makeAllActivities();

    for (const std::unique_ptr<Activity>& original : activities) {
        const QJsonObject json = ActivityJsonSerializer::toJson(*original);
        QVERIFY(!json.isEmpty());
        QVERIFY(json.contains(QStringLiteral("type")));

        QString errorMessage;
        std::unique_ptr<Activity> restored = ActivityJsonSerializer::fromJson(json, &errorMessage);
        QVERIFY2(restored != nullptr, qPrintable(errorMessage));
        QVERIFY(errorMessage.isEmpty());
        compareActivities(*original, *restored);
    }
}

void AgendaTests::malformedAndUnknownJsonFailsSafely()
{
    QString errorMessage;

    std::unique_ptr<Activity> missingType = ActivityJsonSerializer::fromJson(QJsonObject(), &errorMessage);
    QVERIFY(missingType == nullptr);
    QVERIFY(!errorMessage.isEmpty());

    errorMessage.clear();
    QJsonObject unknownType;
    unknownType.insert(QStringLiteral("type"), QStringLiteral("meeting"));
    unknownType.insert(QStringLiteral("title"), QStringLiteral("Unknown"));
    std::unique_ptr<Activity> unknown = ActivityJsonSerializer::fromJson(unknownType, &errorMessage);
    QVERIFY(unknown == nullptr);
    QVERIFY(errorMessage.contains(QStringLiteral("unknown"), Qt::CaseInsensitive));

    auto event = makeEvent();
    QJsonObject invalidEvent = ActivityJsonSerializer::toJson(*event);
    invalidEvent.insert(QStringLiteral("endDateTime"), dateTime(2026, 7, 27, 12, 0).toString(Qt::ISODate));
    errorMessage.clear();
    std::unique_ptr<Activity> reversedEvent = ActivityJsonSerializer::fromJson(invalidEvent, &errorMessage);
    QVERIFY(reversedEvent == nullptr);
    QVERIFY(!errorMessage.isEmpty());

    QJsonObject invalidRecurrence = ActivityJsonSerializer::toJson(*event);
    QJsonObject recurrence = invalidRecurrence.value(QStringLiteral("recurrence")).toObject();
    recurrence.insert(QStringLiteral("interval"), 0);
    invalidRecurrence.insert(QStringLiteral("recurrence"), recurrence);
    errorMessage.clear();
    std::unique_ptr<Activity> badRecurrence =
        ActivityJsonSerializer::fromJson(invalidRecurrence, &errorMessage);
    QVERIFY(badRecurrence == nullptr);
    QVERIFY(errorMessage.contains(QStringLiteral("interval"), Qt::CaseInsensitive));
}

void AgendaTests::agendaFileRoundTripPreservesActivitiesTemplatesAndCategories()
{
    ActivityManager sourceActivities;
    const auto activities = makeAllActivities();
    for (const std::unique_ptr<Activity>& activity : activities) {
        QVERIFY(sourceActivities.addActivity(activity->clone()));
    }

    ActivityTemplateManager sourceTemplates;
    QVERIFY(sourceTemplates.addTemplate(QStringLiteral("Lecture template"), makeEvent(QStringLiteral("template-event"))));

    CategoryManager sourceCategories;
    QVERIFY(sourceCategories.addCategory(QStringLiteral("Study"), QStringLiteral("#3F51B5")));
    QVERIFY(sourceCategories.addCategory(QStringLiteral("Health"), QStringLiteral("#8E44AD")));

    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString filePath = temporaryDirectory.filePath(QStringLiteral("agenda-round-trip.json"));

    QString errorMessage;
    QVERIFY2(AgendaJsonStorage::saveToFile(sourceActivities,
                                           sourceTemplates,
                                           sourceCategories,
                                           filePath,
                                           &errorMessage),
             qPrintable(errorMessage));

    ActivityManager restoredActivities;
    ActivityTemplateManager restoredTemplates;
    CategoryManager restoredCategories;
    errorMessage.clear();
    QVERIFY2(AgendaJsonStorage::loadFromFile(restoredActivities,
                                             restoredTemplates,
                                             restoredCategories,
                                             filePath,
                                             &errorMessage),
             qPrintable(errorMessage));

    QCOMPARE(restoredActivities.size(), sourceActivities.size());
    for (const Activity* expected : sourceActivities.activities()) {
        QVERIFY(expected != nullptr);
        const Activity* actual = restoredActivities.findActivityById(expected->id());
        QVERIFY(actual != nullptr);
        compareActivities(*expected, *actual);
    }

    QCOMPARE(restoredTemplates.size(), 1);
    const ActivityTemplate* restoredTemplate =
        restoredTemplates.findTemplateByName(QStringLiteral("Lecture template"));
    QVERIFY(restoredTemplate != nullptr);
    QVERIFY(restoredTemplate->prototype() != nullptr);
    const ActivityTemplate* sourceTemplate =
        sourceTemplates.findTemplateByName(QStringLiteral("Lecture template"));
    QVERIFY(sourceTemplate != nullptr);
    compareActivities(*sourceTemplate->prototype(), *restoredTemplate->prototype());

    QCOMPARE(restoredCategories.size(), 2);
    const Category* study = restoredCategories.findCategoryByName(QStringLiteral("Study"));
    const Category* health = restoredCategories.findCategoryByName(QStringLiteral("Health"));
    QVERIFY(study != nullptr);
    QVERIFY(health != nullptr);
    QCOMPARE(study->colorHex(), QStringLiteral("#3F51B5"));
    QCOMPARE(health->colorHex(), QStringLiteral("#8E44AD"));
}

void AgendaTests::commandHistorySupportsRepeatedUndoRedoAndBranchTruncation()
{
    {
        ActivityManager manager;
        CommandHistory history;
        auto event = makeEvent(QStringLiteral("add-command"));
        const QString id = event->id();

        QVERIFY(history.executeCommand(
            std::make_unique<AddActivityCommand>(&manager, std::move(event))));
        QCOMPARE(manager.size(), 1);
        QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Add activity"));

        for (int cycle = 0; cycle < 3; ++cycle) {
            QVERIFY(history.undo());
            QVERIFY(manager.findActivityById(id) == nullptr);
            QCOMPARE(history.redoDescription(), QStringLiteral("Redo: Add activity"));
            QVERIFY(history.redo());
            QVERIFY(manager.findActivityById(id) != nullptr);
        }
    }

    {
        ActivityManager manager;
        CommandHistory history;
        auto original = makeDeadline(QStringLiteral("update-command"));
        const QString id = original->id();
        QVERIFY(manager.addActivity(std::move(original)));

        auto edited = std::make_unique<DeadlineActivity>(
            QStringLiteral("Register for advanced OOP exam"),
            dateTime(2026, 8, 3, 9, 0),
            QStringLiteral("Updated context"),
            false,
            QStringLiteral("Updated description"),
            QStringLiteral("University"),
            Priority::High,
            true,
            id,
            dateTime(2026, 7, 20, 10, 0),
            dateTime(2026, 7, 25, 15, 0));

        QVERIFY(history.executeCommand(
            std::make_unique<UpdateActivityCommand>(&manager, id, std::move(edited))));
        QCOMPARE(dynamic_cast<const DeadlineActivity*>(manager.findActivityById(id))->context(),
                 QStringLiteral("Updated context"));
        QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Edit activity"));

        for (int cycle = 0; cycle < 3; ++cycle) {
            QVERIFY(history.undo());
            const auto* restored = dynamic_cast<const DeadlineActivity*>(manager.findActivityById(id));
            QVERIFY(restored != nullptr);
            QCOMPARE(restored->context(), QStringLiteral("University exam"));
            QVERIFY(history.redo());
            const auto* reapplied = dynamic_cast<const DeadlineActivity*>(manager.findActivityById(id));
            QVERIFY(reapplied != nullptr);
            QCOMPARE(reapplied->context(), QStringLiteral("Updated context"));
        }
    }

    {
        ActivityManager manager;
        CommandHistory history;
        auto reminder = makeReminder(QStringLiteral("toggle-command"));
        reminder->setCompleted(false);
        const QString id = reminder->id();
        QVERIFY(manager.addActivity(std::move(reminder)));

        QVERIFY(history.executeCommand(
            std::make_unique<ToggleCompletionCommand>(&manager, id)));
        QVERIFY(manager.findActivityById(id)->isCompleted());
        QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Mark completed"));

        for (int cycle = 0; cycle < 4; ++cycle) {
            QVERIFY(history.undo());
            QVERIFY(!manager.findActivityById(id)->isCompleted());
            QVERIFY(history.redo());
            QVERIFY(manager.findActivityById(id)->isCompleted());
        }
    }

    {
        ActivityManager manager;
        CommandHistory history;
        auto checklist = makeChecklist(QStringLiteral("delete-command"));
        const QString id = checklist->id();
        QVERIFY(manager.addActivity(std::move(checklist)));

        QVERIFY(history.executeCommand(
            std::make_unique<RemoveActivityCommand>(&manager, id)));
        QVERIFY(manager.findActivityById(id) == nullptr);
        QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Delete activity"));
        QVERIFY(history.undo());
        QVERIFY(manager.findActivityById(id) != nullptr);
        QVERIFY(history.redo());
        QVERIFY(manager.findActivityById(id) == nullptr);
    }

    {
        ActivityManager manager;
        CommandHistory history;
        QVERIFY(history.executeCommand(
            std::make_unique<AddActivityCommand>(&manager,
                makeEvent(QStringLiteral("old-history-branch")))));
        QVERIFY(history.undo());
        QVERIFY(history.canRedo());

        QVERIFY(history.executeCommand(
            std::make_unique<AddActivityCommand>(&manager,
                makeReminder(QStringLiteral("new-history-branch")))));
        QVERIFY(!history.canRedo());
        QCOMPARE(history.redoCount(), 0);
    }
}

void AgendaTests::filteringSortingAndSearchingRemainStable()
{
    ActivityManager manager;
    QVERIFY(manager.addActivity(makeEvent(QStringLiteral("filter-event"))));
    QVERIFY(manager.addActivity(makeDeadline(QStringLiteral("filter-deadline"))));
    QVERIFY(manager.addActivity(makeReminder(QStringLiteral("filter-reminder"))));
    QVERIFY(manager.addActivity(makeChecklist(QStringLiteral("filter-checklist"))));

    ActivityFilter::Criteria criteria;
    criteria.category = QStringLiteral("study");
    criteria.sortKey = ActivityFilter::SortKey::Title;
    criteria.sortOrder = ActivityFilter::SortOrder::Ascending;

    std::vector<const Activity*> filtered =
        ActivityFilter::apply(manager.activities(), criteria, dateTime(2026, 7, 27, 12, 0));
    QCOMPARE(static_cast<int>(filtered.size()), 3);
    QCOMPARE(filtered.at(0)->title(), QStringLiteral("Object-Oriented Programming lecture"));
    QCOMPARE(filtered.at(1)->title(), QStringLiteral("Prepare study session"));
    QCOMPARE(filtered.at(2)->title(), QStringLiteral("Register for OOP exam"));

    criteria.priority = Priority::Critical;
    filtered = ActivityFilter::apply(manager.activities(), criteria, dateTime(2026, 7, 27, 12, 0));
    QCOMPARE(static_cast<int>(filtered.size()), 1);
    QCOMPARE(filtered.front()->title(), QStringLiteral("Register for OOP exam"));

    const SearchEngine::SearchResponse titleSearch =
        SearchEngine::search(manager.activities(), QStringLiteral("programming lecture"));
    QVERIFY(titleSearch.hasResults());
    QCOMPARE(titleSearch.results.front().activity->id(), QStringLiteral("filter-event"));

    const SearchEngine::SearchResponse descriptionSearch =
        SearchEngine::search(manager.activities(), QStringLiteral("appointment confirmation"));
    QVERIFY(descriptionSearch.hasResults());
    QCOMPARE(descriptionSearch.results.front().activity->id(), QStringLiteral("filter-reminder"));

    const SearchEngine::SearchResponse noResult =
        SearchEngine::search(manager.activities(), QStringLiteral("interplanetary launch"));
    QVERIFY(!noResult.hasResults());
}

void AgendaTests::mainWorkflowsStayInsideMainWindowAndMouseNavigationWorks()
{
    ActivityManager activities;
    ActivityTemplateManager templates;
    CategoryManager categories;
    QVERIFY(activities.addActivity(makeEvent(QStringLiteral("gui-event"))));
    QVERIFY(activities.addActivity(makeDeadline(QStringLiteral("gui-deadline"))));

    MainWindow window(&activities, &templates, &categories);
    window.resize(1100, 760);
    window.show();
    QTest::qWait(100);

    QStackedWidget* workspace =
        window.findChild<QStackedWidget*>(QStringLiteral("workspaceStack"));
    QListWidget* list = window.findChild<QListWidget*>(QStringLiteral("activityList"));
    QVERIFY(workspace != nullptr);
    QVERIFY(list != nullptr);
    QCOMPARE(workspace->currentWidget()->objectName(), QStringLiteral("activityDetailPage"));
    QTRY_COMPARE(list->count(), 2);

    QPushButton* addButton = buttonWithText(&window, QStringLiteral("Add activity"));
    QVERIFY(addButton != nullptr);
    QTest::mouseClick(addButton, Qt::LeftButton);
    QTRY_COMPARE(workspace->currentWidget()->objectName(), QStringLiteral("activityCreationPage"));

    for (QWidget* topLevel : QApplication::topLevelWidgets()) {
        if (topLevel != &window && topLevel->isVisible()) {
            QVERIFY2(qobject_cast<QDialog*>(topLevel) == nullptr,
                     "Creation unexpectedly opened a separate dialog");
        }
    }

    QDialogButtonBox* creationButtons =
        workspace->currentWidget()->findChild<QDialogButtonBox*>();
    QVERIFY(creationButtons != nullptr);
    QVERIFY(creationButtons->button(QDialogButtonBox::Cancel) != nullptr);
    creationButtons->button(QDialogButtonBox::Cancel)->click();
    QTRY_COMPARE(workspace->currentWidget()->objectName(), QStringLiteral("activityDetailPage"));

    list->setFocus();
    QListWidgetItem* firstItem = list->item(0);
    QListWidgetItem* secondItem = list->item(1);
    QVERIFY(firstItem != nullptr);
    QVERIFY(secondItem != nullptr);
    QTRY_VERIFY(list->itemWidget(firstItem) != nullptr);
    QTRY_VERIFY(list->itemWidget(secondItem) != nullptr);

    QTest::mouseClick(list->viewport(),
                      Qt::LeftButton,
                      Qt::NoModifier,
                      list->visualItemRect(firstItem).center());
    QTRY_COMPARE(list->currentRow(), 0);
    QCOMPARE(workspace->currentWidget()->objectName(), QStringLiteral("activityDetailPage"));

    QTest::mouseDClick(list->viewport(),
                       Qt::LeftButton,
                       Qt::NoModifier,
                       list->visualItemRect(firstItem).center());
    QTRY_COMPARE(workspace->currentWidget()->objectName(), QStringLiteral("activityEditPage"));

    for (QWidget* topLevel : QApplication::topLevelWidgets()) {
        if (topLevel != &window && topLevel->isVisible()) {
            QVERIFY2(qobject_cast<QDialog*>(topLevel) == nullptr,
                     "Editing unexpectedly opened a separate dialog");
        }
    }

    QTest::mouseClick(list->viewport(),
                      Qt::LeftButton,
                      Qt::NoModifier,
                      list->visualItemRect(secondItem).center());
    QTRY_COMPARE(workspace->currentWidget()->objectName(), QStringLiteral("activityDetailPage"));
    QTRY_COMPARE(list->currentRow(), 1);

    bool contextActionTriggered = false;
    QTimer::singleShot(100, qApp, [&contextActionTriggered]() {
        QMenu* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
        if (!menu) {
            return;
        }

        for (QAction* action : menu->actions()) {
            if (action && action->text() == QStringLiteral("Mark completed")) {
                contextActionTriggered = true;
                action->trigger();
                return;
            }
        }
        menu->close();
    });

    QTest::mouseClick(list->viewport(),
                      Qt::RightButton,
                      Qt::NoModifier,
                      list->visualItemRect(secondItem).center());
    QVERIFY2(contextActionTriggered, "The contextual menu did not expose Mark completed");
    QTRY_VERIFY(activities.findActivityById(QStringLiteral("gui-deadline"))->isCompleted());

    list->setMinimumHeight(360);
    window.resize(1100, 900);
    QCoreApplication::processEvents();
    const QRect lastItemRect = list->visualItemRect(list->item(list->count() - 1));
    const QPoint emptyPoint(10, qMin(list->viewport()->height() - 4, lastItemRect.bottom() + 20));
    QVERIFY(list->itemAt(emptyPoint) == nullptr);
    QTest::mouseClick(list->viewport(), Qt::RightButton, Qt::NoModifier, emptyPoint);
    QTRY_VERIFY(list->currentItem() == nullptr);
    QVERIFY(QApplication::activePopupWidget() == nullptr);
}

QTEST_MAIN(AgendaTests)
#include "AgendaTests.moc"
