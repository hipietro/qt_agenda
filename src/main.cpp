#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QTextEdit>
#include <QDateTime>

#include <memory>

#include "model/ActivityManager.h"
#include "model/SearchEngine.h"
#include "model/EventActivity.h"
#include "model/DeadlineActivity.h"
#include "model/ReminderActivity.h"
#include "model/ChecklistActivity.h"

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
        "Simple personal reminder",
        "Health",
        Priority::Medium
    ));

    auto checklist = std::make_unique<ChecklistActivity>(
        "Prepare study session",
        now.addDays(3),
        QVector<ChecklistItem>{},
        "Prepare material before studying",
        "University",
        Priority::Medium
    );

    checklist->addItem("Review theory");
    checklist->addItem("Solve exercises");
    checklist->addItem("Write summary notes");

    manager.addActivity(std::move(checklist));

    QString output;
    output += "Agenda Qt - Advanced search initialized\n\n";
    output += QString("Stored activities: %1\n\n").arg(manager.size());

    output += "All activities:\n\n";

    for (const Activity* activity : manager.activities()) {
        output += activity->summary();
        output += activity->isOverdue(now) ? " | OVERDUE\n" : " | OK\n";
    }

    output += "\n----------------------------------------\n\n";

    const QString normalQuery = "qt";
    const SearchEngine::SearchResponse normalSearch =
        SearchEngine::search(manager.activities(), normalQuery);

    output += QString("Search query: \"%1\"\n").arg(normalQuery);
    output += QString("Direct results: %1\n\n").arg(static_cast<int>(normalSearch.results.size()));

    for (const SearchEngine::SearchResult& result : normalSearch.results) {
        output += QString("- %1 [matched field: %2, score: %3]\n")
                .arg(result.activity->title())
                .arg(result.matchedField)
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

    QMainWindow window;
    window.setWindowTitle("Agenda Qt");
    window.resize(900, 600);

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setText(output);

    window.setCentralWidget(textEdit);
    window.show();

    return app.exec();
}