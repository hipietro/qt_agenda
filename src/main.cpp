#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QTextEdit>
#include <QDateTime>

#include <memory>

#include "model/ActivityManager.h"
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
    output += "Agenda Qt - ActivityManager initialized\n\n";
    output += QString("Stored activities: %1\n\n").arg(manager.size());

    for (const Activity* activity : manager.activities()) {
        output += activity->summary();
        output += activity->isOverdue(now) ? " | OVERDUE\n" : " | OK\n";
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