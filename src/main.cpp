#include <QApplication>
#include <QDateTime>
#include <QFile>

#include <memory>

#include "gui/MainWindow.h"

#include "model/ActivityManager.h"
#include "model/ActivityTemplateManager.h"
#include "model/CategoryManager.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"
#include "model/RecurrenceRule.h"

static void populateDemoData(ActivityManager& manager)
{
    const QDateTime now = QDateTime::currentDateTime();

    CategoryManager categoryManager;
    ActivityTemplateManager templateManager;

    categoryManager.addCategory("University", "#3F51B5");
    categoryManager.addCategory("Health", "#4CAF50");
    categoryManager.addCategory("Personal", "#9E9E9E");

    templateManager.addTemplate(
        "Exam deadline",
        std::make_unique<DeadlineActivity>(
            "New exam deadline",
            now.addDays(7),
            "University exam",
            true,
            "Prepare and submit exam-related material",
            "University",
            Priority::High
        )
    );

    auto studyChecklistTemplate = std::make_unique<ChecklistActivity>(
        "New study checklist",
        now.addDays(5),
        QVector<ChecklistItem>{},
        "Reusable checklist for study sessions",
        "University",
        Priority::Medium
    );

    studyChecklistTemplate->addItem("Review theory");
    studyChecklistTemplate->addItem("Practice exercises");
    studyChecklistTemplate->addItem("Write final notes");

    templateManager.addTemplate("Study checklist", std::move(studyChecklistTemplate));

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

    auto reminder = std::make_unique<ReminderActivity>(
        "Call the doctor",
        now.addSecs(3600),
        15,
        "Ask about appointment confirmation",
        "Personal reminder",
        "Health",
        Priority::Medium
    );

    reminder->setRecurrenceRule(RecurrenceRule(
        RecurrenceRule::Frequency::Weekly,
        1,
        RecurrenceRule::EndMode::AfterOccurrences,
        QDateTime(),
        5
    ));

    manager.addActivity(std::move(reminder));

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

    checklist->setRecurrenceRule(RecurrenceRule(
        RecurrenceRule::Frequency::Daily,
        2,
        RecurrenceRule::EndMode::UntilDate,
        now.addDays(10),
        1
    ));

    manager.addActivity(std::move(checklist));

    const Category* universityCategory = categoryManager.findCategoryByName("University");

    if (universityCategory) {
        categoryManager.updateCategory(universityCategory->id(), "Study", universityCategory->colorHex());
        manager.replaceCategory("University", "Study");
    }

    categoryManager.removeCategoryByName("Personal");

    std::unique_ptr<Activity> activityFromTemplate =
        templateManager.createActivityFromTemplateName("Exam deadline");

    if (activityFromTemplate) {
        activityFromTemplate->setTitle("Register for OOP exam");
        activityFromTemplate->setCategory("Study");
        manager.addActivity(std::move(activityFromTemplate));
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");

    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    ActivityManager manager;
    populateDemoData(manager);

    MainWindow window(&manager);
    window.show();

    return app.exec();
}