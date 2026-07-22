#include <QtTest>

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"
#include "model/RecurrenceRule.h"
#include "persistence/ActivityJsonSerializer.h"

#include <QJsonArray>
#include <QJsonObject>

class JsonSerializationVisitorTest : public QObject
{
    Q_OBJECT

private slots:
    void serializesEventFields();
    void serializesDeadlineFields();
    void serializesReminderFields();
    void serializesChecklistFields();
    void preservesCommonFieldsAndRecurrence();
};

void JsonSerializationVisitorTest::serializesEventFields()
{
    const QDateTime start = QDateTime::fromString("2026-08-01T10:00:00", Qt::ISODate);
    const QDateTime end = QDateTime::fromString("2026-08-01T12:00:00", Qt::ISODate);
    EventActivity activity("Lecture", start, end, "Room A", {"Pietro", "Anna"});

    const QJsonObject json = ActivityJsonSerializer::toJson(activity);

    QCOMPARE(json["type"].toString(), QString("event"));
    QCOMPARE(json["startDateTime"].toString(), start.toString(Qt::ISODate));
    QCOMPARE(json["endDateTime"].toString(), end.toString(Qt::ISODate));
    QCOMPARE(json["location"].toString(), QString("Room A"));
    QCOMPARE(json["participants"].toArray().size(), 2);
}

void JsonSerializationVisitorTest::serializesDeadlineFields()
{
    const QDateTime due = QDateTime::fromString("2026-08-10T18:30:00", Qt::ISODate);
    DeadlineActivity activity("Submit report", due, "University", true);

    const QJsonObject json = ActivityJsonSerializer::toJson(activity);

    QCOMPARE(json["type"].toString(), QString("deadline"));
    QCOMPARE(json["dueDate"].toString(), due.toString(Qt::ISODate));
    QCOMPARE(json["context"].toString(), QString("University"));
    QCOMPARE(json["hardDeadline"].toBool(), true);
}

void JsonSerializationVisitorTest::serializesReminderFields()
{
    const QDateTime reminderDate = QDateTime::fromString("2026-08-04T09:15:00", Qt::ISODate);
    ReminderActivity activity("Call doctor", reminderDate, 20, "Bring documents");

    const QJsonObject json = ActivityJsonSerializer::toJson(activity);

    QCOMPARE(json["type"].toString(), QString("reminder"));
    QCOMPARE(json["reminderDateTime"].toString(), reminderDate.toString(Qt::ISODate));
    QCOMPARE(json["advanceMinutes"].toInt(), 20);
    QCOMPARE(json["reminderNote"].toString(), QString("Bring documents"));
}

void JsonSerializationVisitorTest::serializesChecklistFields()
{
    const QDateTime due = QDateTime::fromString("2026-08-06T20:00:00", Qt::ISODate);
    const QVector<ChecklistItem> items = {
        {"Review theory", true},
        {"Solve exercises", false}
    };
    ChecklistActivity activity("Study", due, items);

    const QJsonObject json = ActivityJsonSerializer::toJson(activity);
    const QJsonArray itemsJson = json["items"].toArray();

    QCOMPARE(json["type"].toString(), QString("checklist"));
    QCOMPARE(json["targetDate"].toString(), due.toString(Qt::ISODate));
    QCOMPARE(itemsJson.size(), 2);
    QCOMPARE(itemsJson[0].toObject()["text"].toString(), QString("Review theory"));
    QCOMPARE(itemsJson[0].toObject()["completed"].toBool(), true);
    QCOMPARE(itemsJson[1].toObject()["completed"].toBool(), false);
}

void JsonSerializationVisitorTest::preservesCommonFieldsAndRecurrence()
{
    const QDateTime created = QDateTime::fromString("2026-07-01T08:00:00", Qt::ISODate);
    const QDateTime updated = QDateTime::fromString("2026-07-02T09:30:00", Qt::ISODate);
    const QDateTime due = QDateTime::fromString("2026-08-12T12:00:00", Qt::ISODate);
    const QDateTime until = QDateTime::fromString("2026-09-01T12:00:00", Qt::ISODate);

    DeadlineActivity activity(
        "Recurring deadline",
        due,
        "Work",
        false,
        "Description",
        "Projects",
        Priority::Critical,
        true,
        "deadline-id",
        created,
        updated);
    activity.setRecurrenceRule(RecurrenceRule(
        RecurrenceRule::Frequency::Weekly,
        2,
        RecurrenceRule::EndMode::UntilDate,
        until,
        1));

    const QJsonObject json = ActivityJsonSerializer::toJson(activity);
    const QJsonObject recurrence = json["recurrence"].toObject();

    QCOMPARE(json["id"].toString(), QString("deadline-id"));
    QCOMPARE(json["title"].toString(), QString("Recurring deadline"));
    QCOMPARE(json["description"].toString(), QString("Description"));
    QCOMPARE(json["category"].toString(), QString("Projects"));
    QCOMPARE(json["priority"].toString(), QString("critical"));
    QCOMPARE(json["completed"].toBool(), true);
    QCOMPARE(json["createdAt"].toString(), created.toString(Qt::ISODate));
    QCOMPARE(json["updatedAt"].toString(), activity.updatedAt().toString(Qt::ISODate));
    QCOMPARE(recurrence["frequency"].toString(), QString("weekly"));
    QCOMPARE(recurrence["interval"].toInt(), 2);
    QCOMPARE(recurrence["endMode"].toString(), QString("until_date"));
    QCOMPARE(recurrence["untilDate"].toString(), until.toString(Qt::ISODate));
}

QTEST_APPLESS_MAIN(JsonSerializationVisitorTest)

#include "tst_jsonserializationvisitor.moc"
