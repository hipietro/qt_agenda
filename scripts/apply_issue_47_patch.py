#!/usr/bin/env python3
"""Move activity creation from a dialog into MainWindow for issue 47.

The helper performs guarded source transformations and removes itself after a
successful run so only the real implementation remains in the feature branch.
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def write(relative: str, content: str) -> None:
    path = ROOT / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def regex_once(text: str, pattern: str, replacement: str, label: str) -> str:
    updated, count = re.subn(pattern, replacement, text, count=1, flags=re.S)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return updated


def build_creation_page_header() -> str:
    return r'''// In-window page used to collect data for a new activity.

#ifndef ACTIVITYCREATIONPAGE_H
#define ACTIVITYCREATIONPAGE_H

#include <QWidget>

#include <functional>
#include <memory>
#include <optional>

#include "model/Activity.h"
#include "model/ActivityKind.h"
#include "model/ChecklistActivity.h"
#include "model/Priority.h"
#include "model/RecurrenceRule.h"

class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QDialogButtonBox;
class QLineEdit;
class QSpinBox;
class QStackedWidget;
class QTextEdit;
class QGroupBox;
class QLabel;
class QWidget;
class CategoryManager;

/*
 * Reusable activity-creation page hosted directly inside MainWindow.
 *
 * The page owns only form state and validation. MainWindow remains responsible
 * for executing AddActivityCommand and navigating back to the agenda.
 */
class ActivityCreationPage : public QWidget
{
public:
    using CreatedHandler = std::function<void(std::unique_ptr<Activity>)>;
    using CancelHandler = std::function<void()>;

    explicit ActivityCreationPage(const CategoryManager* categoryManager = nullptr,
                                  QWidget* parent = nullptr);

    void setCreatedHandler(CreatedHandler handler);
    void setCancelHandler(CancelHandler handler);
    void resetForm();

private:
    void setupUi();
    void connectSignals();
    void updateTypePage();
    void submit();

    bool validateForm() const;
    std::unique_ptr<Activity> createActivityFromForm() const;

    Priority selectedPriority() const;
    ActivityKind selectedActivityKind() const;
    QString selectedCategoryText() const;
    void populateCategoryCombo();

    QVector<ChecklistItem> checklistItemsFromText() const;

    void updateRecurrenceControls();
    std::optional<RecurrenceRule> recurrenceRuleFromForm() const;
    RecurrenceRule::Frequency selectedRecurrenceFrequency() const;
    RecurrenceRule::EndMode selectedRecurrenceEndMode() const;

    const CategoryManager* m_categoryManager = nullptr;

    QComboBox* m_typeCombo = nullptr;
    QLineEdit* m_titleEdit = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QComboBox* m_categoryCombo = nullptr;
    QComboBox* m_priorityCombo = nullptr;

    QGroupBox* m_recurrenceGroup = nullptr;
    QCheckBox* m_repeatsCheck = nullptr;
    QWidget* m_recurrenceOptionsWidget = nullptr;
    QComboBox* m_recurrenceFrequencyCombo = nullptr;
    QSpinBox* m_recurrenceIntervalSpin = nullptr;
    QComboBox* m_recurrenceEndModeCombo = nullptr;
    QWidget* m_recurrenceEndDetailsWidget = nullptr;
    QLabel* m_recurrenceEndDetailsLabel = nullptr;
    QDateTimeEdit* m_recurrenceUntilEdit = nullptr;
    QSpinBox* m_recurrenceOccurrencesSpin = nullptr;

    QStackedWidget* m_typeStack = nullptr;

    QDateTimeEdit* m_eventStartEdit = nullptr;
    QDateTimeEdit* m_eventEndEdit = nullptr;
    QLineEdit* m_eventLocationEdit = nullptr;
    QLineEdit* m_eventParticipantsEdit = nullptr;

    QDateTimeEdit* m_deadlineDueEdit = nullptr;
    QLineEdit* m_deadlineContextEdit = nullptr;
    QCheckBox* m_deadlineHardCheck = nullptr;

    QDateTimeEdit* m_reminderDateEdit = nullptr;
    QSpinBox* m_reminderAdvanceSpin = nullptr;
    QLineEdit* m_reminderNoteEdit = nullptr;

    QDateTimeEdit* m_checklistDueEdit = nullptr;
    QTextEdit* m_checklistItemsEdit = nullptr;

    QDialogButtonBox* m_buttonBox = nullptr;

    CreatedHandler m_createdHandler;
    CancelHandler m_cancelHandler;
};

#endif
'''


def patch_creation_page() -> None:
    write("src/gui/ActivityCreationPage.h", build_creation_page_header())

    source = read("src/gui/ActivityCreationDialog.cpp")
    source = source.replace("ActivityCreationDialog", "ActivityCreationPage")
    source = replace_once(
        source,
        '#include "ActivityCreationPage.h"\n',
        '#include "ActivityCreationPage.h"\n',
        "creation page include",
    )
    source = source.replace(
        "// Activity creation dialog implementation. It builds the right concrete Activity type from user input.",
        "// In-window activity creation page. It builds the requested concrete Activity from user input.",
    )
    source = replace_once(
        source,
        "#include <QFormLayout>\n",
        "#include <QFormLayout>\n#include <QFrame>\n",
        "QFrame include",
    )
    source = replace_once(
        source,
        "#include <QPushButton>\n",
        "#include <QPushButton>\n#include <QScrollArea>\n",
        "QScrollArea include",
    )
    source = replace_once(
        source,
        "    : QDialog(parent),\n",
        "    : QWidget(parent),\n",
        "creation page base initializer",
    )
    source = replace_once(
        source,
        "    setupUi();\n    connectSignals();\n    updateTypePage();\n",
        "    setupUi();\n    connectSignals();\n    resetForm();\n",
        "creation page constructor",
    )

    lifecycle_methods = r'''void ActivityCreationPage::setCreatedHandler(CreatedHandler handler)
{
    m_createdHandler = std::move(handler);
}

void ActivityCreationPage::setCancelHandler(CancelHandler handler)
{
    m_cancelHandler = std::move(handler);
}

void ActivityCreationPage::resetForm()
{
    if (!m_typeCombo) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();

    m_typeCombo->setCurrentIndex(0);
    m_titleEdit->clear();
    m_descriptionEdit->clear();
    populateCategoryCombo();
    m_priorityCombo->setCurrentIndex(1);

    m_eventStartEdit->setDateTime(now.addDays(1));
    m_eventEndEdit->setDateTime(now.addDays(1).addSecs(3600));
    m_eventLocationEdit->clear();
    m_eventParticipantsEdit->clear();

    m_deadlineDueEdit->setDateTime(now.addDays(7));
    m_deadlineContextEdit->clear();
    m_deadlineHardCheck->setChecked(true);

    m_reminderDateEdit->setDateTime(now.addDays(1));
    m_reminderAdvanceSpin->setValue(0);
    m_reminderNoteEdit->clear();

    m_checklistDueEdit->setDateTime(now.addDays(3));
    m_checklistItemsEdit->clear();

    m_repeatsCheck->setChecked(false);
    m_recurrenceIntervalSpin->setValue(1);
    m_recurrenceFrequencyCombo->setCurrentIndex(0);
    m_recurrenceEndModeCombo->setCurrentIndex(0);
    m_recurrenceUntilEdit->setDateTime(now.addMonths(1));
    m_recurrenceOccurrencesSpin->setValue(5);

    updateTypePage();
    updateRecurrenceControls();
    m_titleEdit->setFocus(Qt::OtherFocusReason);
}

void ActivityCreationPage::submit()
{
    if (!validateForm()) {
        return;
    }

    std::unique_ptr<Activity> activity = createActivityFromForm();

    if (!activity) {
        QMessageBox::warning(this, "Invalid activity", "The activity could not be created.");
        return;
    }

    if (!m_createdHandler) {
        QMessageBox::warning(this, "Create activity failed", "The creation action is not available.");
        return;
    }

    m_createdHandler(std::move(activity));
}

'''
    source = regex_once(
        source,
        r"std::unique_ptr<Activity> ActivityCreationPage::takeCreatedActivity\(\)\n\{.*?\n\}\n\nvoid ActivityCreationPage::accept\(\)\n\{.*?\n\}\n\n",
        lifecycle_methods,
        "replace dialog lifecycle methods",
    )

    source = replace_once(
        source,
        '''    setWindowTitle("Create activity");
    resize(720, 620);
    setMinimumSize(680, 560);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);
''',
        '''    setObjectName("activityCreationPage");

    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(10);

    QLabel* pageTitle = new QLabel("Create activity", contentWidget);
    pageTitle->setObjectName("pageTitle");

    QLabel* pageDescription = new QLabel(
        "Choose an activity type, complete its fields, then create it without leaving the main window.",
        contentWidget);
    pageDescription->setWordWrap(true);

    mainLayout->addWidget(pageTitle);
    mainLayout->addWidget(pageDescription);
''',
        "creation page scroll layout",
    )

    source = source.replace(
        'QGroupBox* commonGroup = new QGroupBox("Common fields", this);',
        'QGroupBox* commonGroup = new QGroupBox("Common fields", contentWidget);',
    )
    source = source.replace(
        'QGroupBox* specificGroup = new QGroupBox("Type-specific fields", this);',
        'QGroupBox* specificGroup = new QGroupBox("Type-specific fields", contentWidget);',
    )
    source = source.replace(
        'm_typeStack = new QStackedWidget(this);',
        'm_typeStack = new QStackedWidget(specificGroup);',
    )
    source = source.replace(
        'm_recurrenceGroup = new QGroupBox("Recurrence", this);',
        'm_recurrenceGroup = new QGroupBox("Recurrence", contentWidget);',
    )
    source = source.replace(
        'm_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);',
        'm_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, contentWidget);',
    )

    source = replace_once(
        source,
        '''    mainLayout->addWidget(commonGroup);
    mainLayout->addWidget(specificGroup);
    mainLayout->addWidget(m_recurrenceGroup);
    mainLayout->addWidget(m_buttonBox);

    updateRecurrenceControls();
''',
        '''    mainLayout->addWidget(commonGroup);
    mainLayout->addWidget(specificGroup);
    mainLayout->addWidget(m_recurrenceGroup);
    mainLayout->addWidget(m_buttonBox);
    mainLayout->addStretch(1);

    scrollArea->setWidget(contentWidget);
    pageLayout->addWidget(scrollArea);

    updateRecurrenceControls();
''',
        "finish creation page layout",
    )

    source = replace_once(
        source,
        '''    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        accept();
    });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        reject();
    });
''',
        '''    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        submit();
    });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        if (m_cancelHandler) {
            m_cancelHandler();
        }
    });
''',
        "creation page button handlers",
    )

    source = source.replace("        adjustSize();\n", "")
    source = source.replace("    adjustSize();\n", "")

    write("src/gui/ActivityCreationPage.cpp", source)

    (ROOT / "src/gui/ActivityCreationDialog.h").unlink()
    (ROOT / "src/gui/ActivityCreationDialog.cpp").unlink()


def patch_main_window() -> None:
    header_path = "src/gui/MainWindow.h"
    header = read(header_path)
    header = replace_once(
        header,
        "#include <vector>\n",
        "#include <memory>\n#include <vector>\n",
        "MainWindow memory include",
    )
    header = replace_once(
        header,
        "class ActivityManager;\n",
        "class ActivityCreationPage;\nclass ActivityManager;\n",
        "MainWindow creation page forward declaration",
    )
    header = replace_once(
        header,
        "    void createActivity();\n    void editSelectedActivity();\n",
        "    void createActivity();\n    void addCreatedActivity(std::unique_ptr<Activity> activity);\n    void editSelectedActivity();\n",
        "MainWindow creation handler declaration",
    )
    header = replace_once(
        header,
        "    QWidget* m_creationPage = nullptr;\n",
        "    ActivityCreationPage* m_creationPage = nullptr;\n",
        "MainWindow creation page member",
    )
    write(header_path, header)

    source_path = "src/gui/MainWindow.cpp"
    source = read(source_path)
    source = replace_once(
        source,
        '#include "ActivityCreationDialog.h"\n',
        '#include "ActivityCreationPage.h"\n',
        "MainWindow creation page include",
    )

    source = replace_once(
        source,
        '''    m_creationPage = createWorkflowPlaceholderPage(
        "Create activity",
        "The activity creation form will be hosted on this page. "
        "The dedicated form migration is implemented separately so the existing agenda remains stable."
    );
    m_creationPage->setObjectName("activityCreationPage");
    m_pageStack->addWidget(m_creationPage);
''',
        '''    m_creationPage = new ActivityCreationPage(m_categoryManager, m_pageStack);
    m_creationPage->setCreatedHandler([this](std::unique_ptr<Activity> activity) {
        addCreatedActivity(std::move(activity));
    });
    m_creationPage->setCancelHandler([this]() {
        openAgendaPage();
    });
    m_pageStack->addWidget(m_creationPage);
''',
        "MainWindow creation page construction",
    )

    source = replace_once(
        source,
        '''void MainWindow::openCreationPage()
{
    showPage(m_creationPage);
}
''',
        '''void MainWindow::openCreationPage()
{
    if (!m_creationPage) {
        return;
    }

    synchronizeCategoryManagerFromActivities();
    m_creationPage->resetForm();
    showPage(m_creationPage);
}
''',
        "MainWindow open creation page",
    )

    creation_flow = r'''void MainWindow::createActivity()
{
    if (!m_activityManager || !m_creationPage) {
        return;
    }

    openCreationPage();
}

void MainWindow::addCreatedActivity(std::unique_ptr<Activity> activity)
{
    if (!m_activityManager || !activity) {
        return;
    }

    const QString createdActivityId = activity->id();

    auto command = std::make_unique<AddActivityCommand>(
        m_activityManager,
        std::move(activity)
    );

    if (!m_commandHistory.executeCommand(std::move(command))) {
        QMessageBox::warning(this, "Create activity failed", "The activity could not be added.");
        return;
    }

    setUnsavedChanges(true);
    synchronizeCategoryManagerFromActivities();
    openAgendaPage();
    refreshActivityList();

    for (int row = 0; row < m_activityList->count(); ++row) {
        QListWidgetItem* item = m_activityList->item(row);

        if (item && item->data(Qt::UserRole).toString() == createdActivityId) {
            m_activityList->setCurrentRow(row);
            break;
        }
    }

    updateActionButtons();
    statusBar()->showMessage("Activity created", 3000);
}

void MainWindow::editSelectedActivity'''
    source = regex_once(
        source,
        r"void MainWindow::createActivity\(\)\n\{.*?\n\}\n\nvoid MainWindow::editSelectedActivity",
        creation_flow,
        "replace MainWindow creation dialog flow",
    )

    write(source_path, source)


def patch_project_file() -> None:
    path = "agenda_qt.pro"
    text = read(path)
    text = replace_once(
        text,
        "    src/gui/ActivityCreationDialog.cpp \\\n",
        "    src/gui/ActivityCreationPage.cpp \\\n",
        "qmake creation source",
    )
    text = replace_once(
        text,
        "    src/gui/ActivityCreationDialog.h \\\n",
        "    src/gui/ActivityCreationPage.h \\\n",
        "qmake creation header",
    )
    write(path, text)


def audit_removed_dialog() -> None:
    candidates = [ROOT / "agenda_qt.pro"]
    candidates.extend((ROOT / "src").rglob("*.h"))
    candidates.extend((ROOT / "src").rglob("*.cpp"))

    remaining = []
    for path in candidates:
        if "ActivityCreationDialog" in path.read_text(encoding="utf-8"):
            remaining.append(str(path.relative_to(ROOT)))

    if remaining:
        raise RuntimeError(
            "obsolete ActivityCreationDialog references remain in: " + ", ".join(remaining)
        )


def main() -> None:
    patch_creation_page()
    patch_main_window()
    patch_project_file()
    audit_removed_dialog()

    Path(__file__).resolve().unlink()
    print("Issue 47 in-window creation patch applied successfully.")


if __name__ == "__main__":
    main()
