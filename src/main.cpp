#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QTextEdit>
#include <QDateTime>

#include <memory>
#include <vector>

#include "model/ActivityManager.h"
#include "model/SearchEngine.h"
#include "model/ActivityFilter.h"
#include "model/EventActivity.h"
#include "model/DeadlineActivity.h"
#include "model/ReminderActivity.h"
#include "model/ChecklistActivity.h"

static QString activityStatusToString(const Activity* activity, const QDateTime& now)
{
    if (!activity) {
        return "Invalid";
    }

    if (activity->isCompleted()) {
        return "Completed";
    }

    if (activity->isOverdue(now)) {
        return "Overdue";
    }

    return "Active";
}

static QString matchedFieldToString(const QString& matchedField)
{
    if (matchedField == "title") {
        return "title";
    }

    if (matchedField == "category") {
        return "category";
    }

    if (matchedField == "description") {
        return "description";
    }

    if (matchedField == "summary") {
        return "summary";
    }

    if (matchedField == "all") {
        return "all fields";
    }

    return "unknown field";
}

static QString priorityToDisplayString(Priority priority)
{
    switch (priority) {
    case Priority::Low:
        return "Low";
    case Priority::Medium:
        return "Medium";
    case Priority::High:
        return "High";
    case Priority::Critical:
        return "Critical";
    }

    return "Medium";
}

static void appendActivityList(QString& output,
                               const std::vector<const Activity*>& activities,
                               const QDateTime& now)
{
    for (const Activity* activity : activities) {
        output += QString("- %1 | Type: %2 | Category: %3 | Priority: %4 | Status: %5 | Main date: %6\n")
                .arg(activity->title())
                .arg(activityKindToString(activity->kind()))
                .arg(activity->category())
                .arg(priorityToDisplayString(activity->priority()))
                .arg(activityStatusToString(activity, now))
                .arg(activity->primaryDate().toString("yyyy-MM-dd HH:mm"));
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    const QDateTime now = QDateTime::currentDateTime();

    ActivityManager manager;

    manager.addActivity(std::make_unique<EventActivity>(
        "Object-Oriented Programming lecture",
        now.addDays(1),
        now.addDays(1).addSecs(7200),
        "Room A",
        QStringList{"Professor", "Students"},
        "Qt project discussion",
        "University",
        Priority::High
    ));

    manager.addActivity(std::make_unique<DeadlineActivity>(
        "Submit Qt project",
        now.addDays(14),
        "Programming exam",
        true,
        "Final delivery on Moodle",
        "University",
        Priority::Critical
    ));

    manager.addActivity(std::make_unique<ReminderActivity>(
        "Call the doctor",
        now.addSecs(3600),
        15,
        "Ask about appointment confirmation",
        "Personal reminder",
        "Health",
        Priority::Medium
    ));

    auto checklist = std::make_unique<ChecklistActivity>(
        "Prepare study session",
        now.addDays(3),
        QVector<ChecklistItem>{},
        "Prepare the material before studying",
        "University",
        Priority::Medium
    );

    checklist->addItem("Review theory");
    checklist->addItem("Solve exercises");
    checklist->addItem("Write summary notes");

    manager.addActivity(std::move(checklist));

    QString output;
    output += "Agenda Qt - Search, filters and sorting initialized\n\n";
    output += QString("Stored activities: %1\n\n").arg(manager.size());

    output += "All activities:\n\n";
    appendActivityList(output, manager.activities(), now);

    output += "\n----------------------------------------\n\n";

    const QString normalQuery = "qt";
    const SearchEngine::SearchResponse normalSearch =
        SearchEngine::search(manager.activities(), normalQuery);

    output += QString("Search query: \"%1\"\n").arg(normalQuery);
    output += QString("Direct results: %1\n\n").arg(static_cast<int>(normalSearch.results.size()));

    for (const SearchEngine::SearchResult& result : normalSearch.results) {
        output += QString("- %1 [matched field: %2, score: %3]\n")
                .arg(result.activity->title())
                .arg(matchedFieldToString(result.matchedField))
                .arg(result.score);
    }

    output += "\n----------------------------------------\n\n";

    const QString typoQuery = "projet";
    const SearchEngine::SearchResponse typoSearch =
        SearchEngine::search(manager.activities(), typoQuery);

    output += QString("Search query with typo: \"%1\"\n").arg(typoQuery);
    output += QString("Direct results: %1\n").arg(static_cast<int>(typoSearch.results.size()));

    if (!typoSearch.hasResults() && typoSearch.suggestion.isValid()) {
        output += QString("No direct result. Did you mean: %1? ")
                .arg(typoSearch.suggestion.suggestedText);
        output += QString("(matched text: %1, distance: %2)\n")
                .arg(typoSearch.suggestion.matchedText)
                .arg(typoSearch.suggestion.distance);
    } else if (!typoSearch.hasResults()) {
        output += "No direct result and no reliable suggestion found.\n";
    }

    output += "\n----------------------------------------\n\n";

    ActivityFilter::Criteria universityCriteria;
    universityCriteria.category = "University";
    universityCriteria.completion = ActivityFilter::CompletionFilter::ActiveOnly;
    universityCriteria.fromDate = now;
    universityCriteria.toDate = now.addDays(15);
    universityCriteria.sortKey = ActivityFilter::SortKey::PrimaryDate;
    universityCriteria.sortOrder = ActivityFilter::SortOrder::Ascending;

    const std::vector<const Activity*> universityActivities =
        ActivityFilter::apply(manager.activities(), universityCriteria, now);

    output += "Filter: University category, active activities, next 15 days, sorted by date\n";
    output += QString("Filtered results: %1\n\n").arg(static_cast<int>(universityActivities.size()));
    appendActivityList(output, universityActivities, now);

    output += "\n----------------------------------------\n\n";

    ActivityFilter::Criteria deadlineCriteria;
    deadlineCriteria.kind = ActivityKind::Deadline;
    deadlineCriteria.sortKey = ActivityFilter::SortKey::Priority;
    deadlineCriteria.sortOrder = ActivityFilter::SortOrder::Descending;

    const std::vector<const Activity*> deadlineActivities =
        ActivityFilter::apply(manager.activities(), deadlineCriteria, now);

    output += "Filter: deadlines only, sorted by descending priority\n";
    output += QString("Filtered results: %1\n\n").arg(static_cast<int>(deadlineActivities.size()));
    appendActivityList(output, deadlineActivities, now);

    QMainWindow window;
    window.setWindowTitle("Agenda Qt");
    window.resize(1000, 700);

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setText(output);

    window.setCentralWidget(textEdit);
    window.show();

    return app.exec();
}