#include <QtTest>

#include "ActivityVisitor.h"
#include "ChecklistActivity.h"
#include "DeadlineActivity.h"
#include "EventActivity.h"
#include "ReminderActivity.h"

class RecordingVisitor final : public ActivityVisitor
{
public:
    enum class VisitedType
    {
        None,
        Event,
        Deadline,
        Reminder,
        Checklist
    };

    void reset()
    {
        m_lastVisited = VisitedType::None;
        m_lastActivity = nullptr;
    }

    VisitedType lastVisited() const
    {
        return m_lastVisited;
    }

    const Activity* lastActivity() const
    {
        return m_lastActivity;
    }

    void visit(const EventActivity& activity) override
    {
        record(VisitedType::Event, activity);
    }

    void visit(const DeadlineActivity& activity) override
    {
        record(VisitedType::Deadline, activity);
    }

    void visit(const ReminderActivity& activity) override
    {
        record(VisitedType::Reminder, activity);
    }

    void visit(const ChecklistActivity& activity) override
    {
        record(VisitedType::Checklist, activity);
    }

private:
    void record(VisitedType type, const Activity& activity)
    {
        m_lastVisited = type;
        m_lastActivity = &activity;
    }

    VisitedType m_lastVisited = VisitedType::None;
    const Activity* m_lastActivity = nullptr;
};

class ActivityVisitorTest : public QObject
{
    Q_OBJECT

private slots:
    void dispatchesEventActivity();
    void dispatchesDeadlineActivity();
    void dispatchesReminderActivity();
    void dispatchesChecklistActivity();
};

void ActivityVisitorTest::dispatchesEventActivity()
{
    const QDateTime start = QDateTime::currentDateTime();
    EventActivity activity("Meeting", start, start.addSecs(3600));
    const Activity& base = activity;
    RecordingVisitor visitor;

    base.accept(visitor);

    QCOMPARE(static_cast<int>(visitor.lastVisited()),
             static_cast<int>(RecordingVisitor::VisitedType::Event));
    QCOMPARE(visitor.lastActivity(), &base);
}

void ActivityVisitorTest::dispatchesDeadlineActivity()
{
    DeadlineActivity activity("Submit report", QDateTime::currentDateTime());
    const Activity& base = activity;
    RecordingVisitor visitor;

    base.accept(visitor);

    QCOMPARE(static_cast<int>(visitor.lastVisited()),
             static_cast<int>(RecordingVisitor::VisitedType::Deadline));
    QCOMPARE(visitor.lastActivity(), &base);
}

void ActivityVisitorTest::dispatchesReminderActivity()
{
    ReminderActivity activity("Call dentist", QDateTime::currentDateTime());
    const Activity& base = activity;
    RecordingVisitor visitor;

    base.accept(visitor);

    QCOMPARE(static_cast<int>(visitor.lastVisited()),
             static_cast<int>(RecordingVisitor::VisitedType::Reminder));
    QCOMPARE(visitor.lastActivity(), &base);
}

void ActivityVisitorTest::dispatchesChecklistActivity()
{
    ChecklistActivity activity("Trip preparation", QDateTime::currentDateTime());
    const Activity& base = activity;
    RecordingVisitor visitor;

    base.accept(visitor);

    QCOMPARE(static_cast<int>(visitor.lastVisited()),
             static_cast<int>(RecordingVisitor::VisitedType::Checklist));
    QCOMPARE(visitor.lastActivity(), &base);
}

QTEST_APPLESS_MAIN(ActivityVisitorTest)

#include "tst_activityvisitor.moc"
