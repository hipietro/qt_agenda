#include <QtTest>

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/RecurrenceRule.h"
#include "model/ReminderActivity.h"
#include "persistence/ActivityFactoryRegistry.h"
#include "persistence/ActivityJsonSerializer.h"

#include <QJsonArray>
#include <QJsonObject>

#include <memory>

class JsonFactoryRegistryTest : public QObject
{
    Q_OBJECT

private slots:
    void reconstructsEvent();
    void reconstructsDeadline();
    void reconstructsReminder();
    void reconstructsChecklist();
    void rejectsMissingType();
    void rejectsUnknownType();
    void rejectsMalformedRequiredField();
    void preservesRoundTripData();
    void supportsCustomFactoryRegistration();
};

void JsonFactoryRegistryTest::reconstructsEvent()
{
    const QDateTime start = QDateTime::fromString("2026-08-01T10:00:00", Qt::ISODate);
    const QDateTime end = QDateTime::fromString("2026-08-01T12:00:00", Qt::ISODate);
    EventActivity original("Lecture", start, end, "Room A", {"Pietro", "Anna"});

    QString error;
    std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(ActivityJsonSerializer::toJson(original), &error);

    QVERIFY2(activity != nullptr, qPrintable(error));
    EventActivity* event = dynamic_cast<EventActivity*>(activity.get());
    QVERIFY(event != nullptr);
    QCOMPARE(event->startDateTime(), start);
    QCOMPARE(event->endDateTime(), end);
    QCOMPARE(event->location(), QString("Room A"));
    QCOMPARE(event->participants(), QStringList({"Pietro", "Anna"}));
}

void JsonFactoryRegistryTest::reconstructsDeadline()
{
    const QDateTime due = QDateTime::fromString("2026-08-10T18:30:00", Qt::ISODate);
    DeadlineActivity original("Submit report", due, "University", true);

    QString error;
    std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(ActivityJsonSerializer::toJson(original), &error);

    QVERIFY2(activity != nullptr, qPrintable(error));
    DeadlineActivity* deadline = dynamic_cast<DeadlineActivity*>(activity.get());
    QVERIFY(deadline != nullptr);
    QCOMPARE(deadline->dueDate(), due);
    QCOMPARE(deadline->context(), QString("University"));
    QCOMPARE(deadline->isHardDeadline(), true);
}

void JsonFactoryRegistryTest::reconstructsReminder()
{
    const QDateTime reminderDate =
        QDateTime::fromString("2026-08-04T09:15:00", Qt::ISODate);
    ReminderActivity original("Call doctor", reminderDate, 20, "Bring documents");

    QString error;
    std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(ActivityJsonSerializer::toJson(original), &error);

    QVERIFY2(activity != nullptr, qPrintable(error));
    ReminderActivity* reminder = dynamic_cast<ReminderActivity*>(activity.get());
    QVERIFY(reminder != nullptr);
    QCOMPARE(reminder->reminderDateTime(), reminderDate);
    QCOMPARE(reminder->advanceMinutes(), 20);
    QCOMPARE(reminder->reminderNote(), QString("Bring documents"));
}

void JsonFactoryRegistryTest::reconstructsChecklist()
{
    const QDateTime targetDate =
        QDateTime::fromString("2026-08-06T20:00:00", Qt::ISODate);
    const QVector<ChecklistItem> items = {
        {"Review theory", true},
        {"Solve exercises", false}
    };
    ChecklistActivity original("Study", targetDate, items);

    QString error;
    std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(ActivityJsonSerializer::toJson(original), &error);

    QVERIFY2(activity != nullptr, qPrintable(error));
    ChecklistActivity* checklist = dynamic_cast<ChecklistActivity*>(activity.get());
    QVERIFY(checklist != nullptr);
    QCOMPARE(checklist->dueDate(), targetDate);
    QCOMPARE(checklist->items().size(), 2);
    QCOMPARE(checklist->items()[0].text, QString("Review theory"));
    QCOMPARE(checklist->items()[0].completed, true);
    QCOMPARE(checklist->items()[1].text, QString("Solve exercises"));
    QCOMPARE(checklist->items()[1].completed, false);
}

void JsonFactoryRegistryTest::rejectsMissingType()
{
    QJsonObject json;
    json["title"] = "Missing type";

    QString error;
    const std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(json, &error);

    QVERIFY(activity == nullptr);
    QVERIFY(error.contains("type"));
}

void JsonFactoryRegistryTest::rejectsUnknownType()
{
    QJsonObject json;
    json["type"] = "appointment";
    json["title"] = "Unknown type";

    QString error;
    const std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(json, &error);

    QVERIFY(activity == nullptr);
    QVERIFY(error.contains("Unknown activity type"));
}

void JsonFactoryRegistryTest::rejectsMalformedRequiredField()
{
    QJsonObject json;
    json["type"] = "event";
    json["title"] = "Invalid event";
    json["endDateTime"] = "2026-08-01T12:00:00";

    QString error;
    const std::unique_ptr<Activity> activity =
        ActivityJsonSerializer::fromJson(json, &error);

    QVERIFY(activity == nullptr);
    QVERIFY(error.contains("startDateTime"));
}

void JsonFactoryRegistryTest::preservesRoundTripData()
{
    const QDateTime created =
        QDateTime::fromString("2026-07-01T08:00:00", Qt::ISODate);
    const QDateTime updated =
        QDateTime::fromString("2026-07-02T09:30:00", Qt::ISODate);
    const QDateTime reminderDate =
        QDateTime::fromString("2026-08-04T09:15:00", Qt::ISODate);

    ReminderActivity original(
        "Recurring reminder",
        reminderDate,
        30,
        "Prepare documents",
        "Description",
        "Personal",
        Priority::High,
        false,
        "reminder-id",
        created,
        updated
    );
    original.setRecurrenceRule(RecurrenceRule(
        RecurrenceRule::Frequency::Weekly,
        2,
        RecurrenceRule::EndMode::AfterOccurrences,
        QDateTime(),
        5
    ));

    const QJsonObject originalJson = ActivityJsonSerializer::toJson(original);

    QString error;
    std::unique_ptr<Activity> loaded =
        ActivityJsonSerializer::fromJson(originalJson, &error);

    QVERIFY2(loaded != nullptr, qPrintable(error));
    QCOMPARE(loaded->updatedAt().toString(Qt::ISODate),
             originalJson["updatedAt"].toString());
    QVERIFY(loaded->hasRecurrence());
    QCOMPARE(loaded->recurrenceRule()->interval(), 2);
    QCOMPARE(loaded->recurrenceRule()->maxOccurrences(), 5);
    QVERIFY(ActivityJsonSerializer::toJson(*loaded) == originalJson);
}

void JsonFactoryRegistryTest::supportsCustomFactoryRegistration()
{
    ActivityFactoryRegistry registry;
    registry.registerFactory(
        "custom",
        [](const QJsonObject&, QString*) -> std::unique_ptr<Activity> {
            return std::make_unique<DeadlineActivity>(
                "Custom activity",
                QDateTime::fromString("2026-09-01T12:00:00", Qt::ISODate)
            );
        }
    );

    QJsonObject json;
    json["type"] = "custom";

    QString error;
    std::unique_ptr<Activity> activity = registry.create(json, &error);

    QVERIFY2(activity != nullptr, qPrintable(error));
    QVERIFY(dynamic_cast<DeadlineActivity*>(activity.get()) != nullptr);
    QCOMPARE(activity->title(), QString("Custom activity"));
}

QTEST_APPLESS_MAIN(JsonFactoryRegistryTest)

#include "tst_jsonfactoryregistry.moc"
