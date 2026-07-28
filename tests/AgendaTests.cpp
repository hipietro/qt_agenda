#include <QtTest>

#include "commands/AddActivityCommand.h"
#include "commands/CommandHistory.h"
#include "commands/RemoveActivityCommand.h"
#include "commands/ToggleCompletionCommand.h"
#include "commands/UpdateActivityCommand.h"
#include "gui/ActivityDetailVisitor.h"
#include "gui/ActivityListItemVisitor.h"
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

#include <QDateTime>
#include <QJsonObject>
#include <QLabel>
#include <QTemporaryDir>
#include <QTimeZone>

#include <memory>
#include <vector>

namespace {

QDateTime fixedDate(int day, int hour = 12, int minute = 0)
{
    return QDateTime(QDate(2026, 7, day),
                     QTime(hour, minute),
                     QTimeZone::utc());
}

std::unique_ptr<EventActivity> makeEvent(const QString& id = QStringLiteral("event-id"))
{
    auto event = std::make_unique<EventActivity>(
        QStringLiteral("Object-Oriented Programming lecture"),
        fixedDate(27, 14, 30),
        fixedDate(27, 16, 30),
        QStringLiteral("Room A"),
        QStringList{QStringLiteral("Pietro"), QStringLiteral("Alice")},
        QStringLiteral("Visitor pattern lecture"),
        QStringLiteral("Study"),
        Priority::High,
        false,
        id,
        fixedDate(20, 10),
        fixedDate(21, 11));

    event->setRecurrenceRule(RecurrenceRule(
        RecurrenceRule::Frequency::Weekly,
        2,
        RecurrenceRule::EndMode::AfterOccurrences,
        QDateTime(),
        5));
    return event;
}

std::unique_ptr<DeadlineActivity> makeDeadline(const QString& id = QStringLiteral("deadline-id"))
{
    return std::make_unique<DeadlineActivity>(
        QStringLiteral("Register for OOP exam"),
        QDateTime(QDate(2026, 8, 2), QTime(14, 30), QTimeZone::utc()),
        QStringLiteral("University exam"),
        true,
        QStringLiteral("Submit exam registration"),
        QStringLiteral("Study"),
        Priority::Critical,
        false,
        id,
        fixedDate(20, 10),
        fixedDate(22, 12));
}

std::unique_ptr<ReminderActivity> makeReminder(const QString& id = QStringLiteral("reminder-id"))
{
    return std::make_unique<ReminderActivity>(
        QStringLiteral("Call the doctor"),
        fixedDate(28, 16, 45),
        15,
        QStringLiteral("Ask about appointment confirmation"),
        QStringLiteral("Personal reminder"),
        QStringLiteral("Health"),
        Priority::Medium,
        true,
        id,
        fixedDate(20, 10),
        fixedDate(23, 13));
}

std::unique_ptr<ChecklistActivity> makeChecklist(const QString& id = QStringLiteral("checklist-id"))
{
    return std::make_unique<ChecklistActivity>(
        QStringLiteral("Prepare study session"),
        fixedDate(29, 14, 30),
        QVector<ChecklistItem>{
            {QStringLiteral("Read chapter"), true},
            {QStringLiteral("Solve exercises"), false},
            {QStringLiteral("Review notes"), false}},
        QStringLiteral("Prepare all study material"),
        QStringLiteral("Study"),
        Priority::Medium,
        false,
        id,
        fixedDate(20, 10),
        fixedDate(24, 14));
}

std::vector<std::unique_ptr<Activity>> allActivities()
{
    std::vector<std::unique_ptr<Activity>> result;
    result.push_back(makeEvent());
    result.push_back(makeDeadline());
    result.push_back(makeReminder());
    result.push_back(makeChecklist());
    return result;
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

void compareActivity(const Activity& expected, const Activity& actual)
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

    if (const auto* source = dynamic_cast<const EventActivity*>(&expected)) {
        const auto* restored = dynamic_cast<const EventActivity*>(&actual);
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->startDateTime(), source->startDateTime());
        QCOMPARE(restored->endDateTime(), source->endDateTime());
        QCOMPARE(restored->location(), source->location());
        QCOMPARE(restored->participants(), source->participants());
        return;
    }

    if (const auto* source = dynamic_cast<const DeadlineActivity*>(&expected)) {
        const auto* restored = dynamic_cast<const DeadlineActivity*>(&actual);
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->dueDate(), source->dueDate());
        QCOMPARE(restored->context(), source->context());
        QCOMPARE(restored->isHardDeadline(), source->isHardDeadline());
        return;
    }

    if (const auto* source = dynamic_cast<const ReminderActivity*>(&expected)) {
        const auto* restored = dynamic_cast<const ReminderActivity*>(&actual);
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->reminderDateTime(), source->reminderDateTime());
        QCOMPARE(restored->advanceMinutes(), source->advanceMinutes());
        QCOMPARE(restored->reminderNote(), source->reminderNote());
        return;
    }

    const auto* source = dynamic_cast<const ChecklistActivity*>(&expected);
    const auto* restored = dynamic_cast<const ChecklistActivity*>(&actual);
    QVERIFY(source != nullptr);
    QVERIFY(restored != nullptr);
    QCOMPARE(restored->dueDate(), source->dueDate());
    QCOMPARE(restored->items().size(), source->items().size());
    for (int index = 0; index < source->items().size(); ++index) {
        QCOMPARE(restored->items().at(index).text, source->items().at(index).text);
        QCOMPARE(restored->items().at(index).completed, source->items().at(index).completed);
    }
}

QString allLabelText(QWidget* widget)
{
    QStringList text;
    for (const QLabel* label : widget->findChildren<QLabel*>()) {
        text.append(label->text());
    }
    return text.join(QLatin1Char('\n'));
}

class RecordingVisitor final : public ActivityVisitor
{
public:
    void visit(const EventActivity&) override { visits.append(QStringLiteral("event")); }
    void visit(const DeadlineActivity&) override { visits.append(QStringLiteral("deadline")); }
    void visit(const ReminderActivity&) override { visits.append(QStringLiteral("reminder")); }
    void visit(const ChecklistActivity&) override { visits.append(QStringLiteral("checklist")); }

    QStringList visits;
};

} // namespace

class AgendaTests final : public QObject
{
    Q_OBJECT

private slots:
    void visitorDoubleDispatch();
    void visitorRenderers();
    void activityJsonRoundTrip();
    void malformedJson();
    void agendaStorageRoundTrip();
    void commandHistoryRegression();
    void filterAndSearchRegression();
};

void AgendaTests::visitorDoubleDispatch()
{
    RecordingVisitor visitor;
    const auto activities = allActivities();
    for (const auto& activity : activities) {
        const Activity& base = *activity;
        base.accept(visitor);
    }

    QCOMPARE(visitor.visits,
             QStringList({QStringLiteral("event"),
                          QStringLiteral("deadline"),
                          QStringLiteral("reminder"),
                          QStringLiteral("checklist")}));
}

void AgendaTests::visitorRenderers()
{
    const auto activities = allActivities();
    const QStringList badges{QStringLiteral("EVENT"), QStringLiteral("DEADLINE"),
                             QStringLiteral("REMINDER"), QStringLiteral("CHECKLIST")};
    const QStringList sections{QStringLiteral("Event schedule"), QStringLiteral("Deadline status"),
                               QStringLiteral("Reminder settings"), QStringLiteral("Checklist progress")};

    for (int index = 0; index < static_cast<int>(activities.size()); ++index) {
        ActivityListItemVisitor listVisitor;
        activities.at(index)->accept(listVisitor);
        std::unique_ptr<QWidget> card(listVisitor.takeWidget());
        QVERIFY(card != nullptr);
        QVERIFY(allLabelText(card.get()).contains(badges.at(index)));

        ActivityDetailVisitor detailVisitor;
        activities.at(index)->accept(detailVisitor);
        std::unique_ptr<QWidget> page(detailVisitor.takeWidget());
        QVERIFY(page != nullptr);
        QVERIFY(allLabelText(page.get()).contains(sections.at(index)));
    }
}

void AgendaTests::activityJsonRoundTrip()
{
    const auto activities = allActivities();
    for (const auto& original : activities) {
        QString error;
        const QJsonObject json = ActivityJsonSerializer::toJson(*original);
        std::unique_ptr<Activity> restored = ActivityJsonSerializer::fromJson(json, &error);
        QVERIFY2(restored != nullptr, qPrintable(error));
        QVERIFY(error.isEmpty());
        compareActivity(*original, *restored);
    }
}

void AgendaTests::malformedJson()
{
    QString error;
    QVERIFY(ActivityJsonSerializer::fromJson(QJsonObject(), &error) == nullptr);
    QVERIFY(!error.isEmpty());

    QJsonObject unknown{{QStringLiteral("type"), QStringLiteral("meeting")},
                        {QStringLiteral("title"), QStringLiteral("Unknown")}};
    error.clear();
    QVERIFY(ActivityJsonSerializer::fromJson(unknown, &error) == nullptr);
    QVERIFY(error.contains(QStringLiteral("unknown"), Qt::CaseInsensitive));

    auto event = makeEvent();
    QJsonObject invalidEvent = ActivityJsonSerializer::toJson(*event);
    invalidEvent[QStringLiteral("endDateTime")] = fixedDate(27, 12).toString(Qt::ISODateWithMs);
    error.clear();
    QVERIFY(ActivityJsonSerializer::fromJson(invalidEvent, &error) == nullptr);

    QJsonObject invalidRecurrence = ActivityJsonSerializer::toJson(*event);
    QJsonObject recurrence = invalidRecurrence.value(QStringLiteral("recurrence")).toObject();
    recurrence[QStringLiteral("interval")] = 0;
    invalidRecurrence[QStringLiteral("recurrence")] = recurrence;
    error.clear();
    QVERIFY(ActivityJsonSerializer::fromJson(invalidRecurrence, &error) == nullptr);
    QVERIFY(error.contains(QStringLiteral("interval"), Qt::CaseInsensitive));
}

void AgendaTests::agendaStorageRoundTrip()
{
    ActivityManager sourceActivities;
    const auto originals = allActivities();
    for (const auto& activity : originals) {
        QVERIFY(sourceActivities.addActivity(activity->clone()));
    }

    ActivityTemplateManager sourceTemplates;
    QVERIFY(sourceTemplates.addTemplate(QStringLiteral("Lecture template"),
                                        makeEvent(QStringLiteral("template-event"))));

    CategoryManager sourceCategories;
    QVERIFY(sourceCategories.addCategory(QStringLiteral("Study"), QStringLiteral("#3F51B5")));
    QVERIFY(sourceCategories.addCategory(QStringLiteral("Health"), QStringLiteral("#8E44AD")));

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("agenda.json"));
    QString error;
    QVERIFY2(AgendaJsonStorage::saveToFile(sourceActivities, sourceTemplates,
                                           sourceCategories, path, &error), qPrintable(error));

    ActivityManager restoredActivities;
    ActivityTemplateManager restoredTemplates;
    CategoryManager restoredCategories;
    error.clear();
    QVERIFY2(AgendaJsonStorage::loadFromFile(restoredActivities, restoredTemplates,
                                             restoredCategories, path, &error), qPrintable(error));

    QCOMPARE(restoredActivities.size(), sourceActivities.size());
    for (const Activity* expected : sourceActivities.activities()) {
        const Activity* actual = restoredActivities.findActivityById(expected->id());
        QVERIFY(actual != nullptr);
        compareActivity(*expected, *actual);
    }

    const ActivityTemplate* sourceTemplate = sourceTemplates.findTemplateByName(QStringLiteral("Lecture template"));
    const ActivityTemplate* restoredTemplate = restoredTemplates.findTemplateByName(QStringLiteral("Lecture template"));
    QVERIFY(sourceTemplate != nullptr);
    QVERIFY(restoredTemplate != nullptr);
    compareActivity(*sourceTemplate->prototype(), *restoredTemplate->prototype());

    QCOMPARE(restoredCategories.size(), 2);
    QCOMPARE(restoredCategories.findCategoryByName(QStringLiteral("Study"))->colorHex(),
             QStringLiteral("#3F51B5"));
    QCOMPARE(restoredCategories.findCategoryByName(QStringLiteral("Health"))->colorHex(),
             QStringLiteral("#8E44AD"));
}

void AgendaTests::commandHistoryRegression()
{
    ActivityManager manager;
    CommandHistory history;

    auto event = makeEvent(QStringLiteral("command-event"));
    const QString eventId = event->id();
    QVERIFY(history.executeCommand(std::make_unique<AddActivityCommand>(&manager, std::move(event))));
    QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Add activity"));
    for (int cycle = 0; cycle < 3; ++cycle) {
        QVERIFY(history.undo());
        QVERIFY(manager.findActivityById(eventId) == nullptr);
        QVERIFY(history.redo());
        QVERIFY(manager.findActivityById(eventId) != nullptr);
    }

    auto edited = std::make_unique<EventActivity>(
        QStringLiteral("Edited lecture"), fixedDate(30, 9), fixedDate(30, 11),
        QStringLiteral("Room B"), QStringList{}, QStringLiteral("Edited"),
        QStringLiteral("Study"), Priority::Critical, false, eventId,
        fixedDate(20, 10), fixedDate(25, 15));
    QVERIFY(history.executeCommand(std::make_unique<UpdateActivityCommand>(
        &manager, eventId, std::move(edited))));
    QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Edit activity"));
    QVERIFY(history.undo());
    QCOMPARE(dynamic_cast<const EventActivity*>(manager.findActivityById(eventId))->location(),
             QStringLiteral("Room A"));
    QVERIFY(history.redo());
    QCOMPARE(dynamic_cast<const EventActivity*>(manager.findActivityById(eventId))->location(),
             QStringLiteral("Room B"));

    QVERIFY(history.executeCommand(std::make_unique<ToggleCompletionCommand>(&manager, eventId)));
    for (int cycle = 0; cycle < 4; ++cycle) {
        QVERIFY(history.undo());
        QVERIFY(!manager.findActivityById(eventId)->isCompleted());
        QVERIFY(history.redo());
        QVERIFY(manager.findActivityById(eventId)->isCompleted());
    }

    QVERIFY(history.executeCommand(std::make_unique<RemoveActivityCommand>(&manager, eventId)));
    QCOMPARE(history.undoDescription(), QStringLiteral("Undo: Delete activity"));
    QVERIFY(history.undo());
    QVERIFY(manager.findActivityById(eventId) != nullptr);

    QVERIFY(history.undo());
    QVERIFY(history.canRedo());
    QVERIFY(history.executeCommand(std::make_unique<AddActivityCommand>(
        &manager, makeReminder(QStringLiteral("new-branch")))));
    QVERIFY(!history.canRedo());
}

void AgendaTests::filterAndSearchRegression()
{
    ActivityManager manager;
    QVERIFY(manager.addActivity(makeEvent(QStringLiteral("filter-event"))));
    QVERIFY(manager.addActivity(makeDeadline(QStringLiteral("filter-deadline"))));
    QVERIFY(manager.addActivity(makeReminder(QStringLiteral("filter-reminder"))));
    QVERIFY(manager.addActivity(makeChecklist(QStringLiteral("filter-checklist"))));

    ActivityFilter::Criteria criteria;
    criteria.category = QStringLiteral("study");
    criteria.sortKey = ActivityFilter::SortKey::Title;
    std::vector<const Activity*> filtered =
        ActivityFilter::apply(manager.activities(), criteria, fixedDate(27));
    QCOMPARE(static_cast<int>(filtered.size()), 3);
    QCOMPARE(filtered.at(0)->title(), QStringLiteral("Object-Oriented Programming lecture"));

    criteria.priority = Priority::Critical;
    filtered = ActivityFilter::apply(manager.activities(), criteria, fixedDate(27));
    QCOMPARE(static_cast<int>(filtered.size()), 1);
    QCOMPARE(filtered.front()->id(), QStringLiteral("filter-deadline"));

    const SearchEngine::SearchResponse titleSearch =
        SearchEngine::search(manager.activities(), QStringLiteral("programming lecture"));
    QVERIFY(titleSearch.hasResults());
    QCOMPARE(titleSearch.results.front().activity->id(), QStringLiteral("filter-event"));

    const SearchEngine::SearchResponse summarySearch =
        SearchEngine::search(manager.activities(), QStringLiteral("appointment confirmation"));
    QVERIFY(summarySearch.hasResults());
    QCOMPARE(summarySearch.results.front().activity->id(), QStringLiteral("filter-reminder"));

    QVERIFY(!SearchEngine::search(manager.activities(), QStringLiteral("interplanetary launch")).hasResults());
}

QTEST_MAIN(AgendaTests)
#include "AgendaTests.moc"
