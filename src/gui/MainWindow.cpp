#include "MainWindow.h"

#include "model/ActivityFilter.h"
#include "model/ActivityManager.h"
#include "model/SearchEngine.h"

#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

MainWindow::MainWindow(ActivityManager* activityManager, QWidget* parent)
    : QMainWindow(parent),
      m_activityManager(activityManager)
{
    setupUi();
    connectSignals();
    refreshActivityList();
}

void MainWindow::setupUi()
{
    setWindowTitle("Agenda Qt");
    resize(1200, 750);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QLabel* titleLabel = new QLabel("Agenda Qt", centralWidget);
    titleLabel->setObjectName("appTitleLabel");

    QLabel* subtitleLabel = new QLabel("Manage activities with search, filters and basic actions", centralWidget);
    subtitleLabel->setObjectName("appSubtitleLabel");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);

    QSplitter* splitter = new QSplitter(centralWidget);

    QWidget* leftPanel = new QWidget(splitter);
    leftPanel->setMinimumWidth(360);

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    QLabel* searchLabel = new QLabel("Search", leftPanel);
    m_searchEdit = new QLineEdit(leftPanel);
    m_searchEdit->setPlaceholderText("Search by title, category, description or summary...");

    QLabel* typeLabel = new QLabel("Activity type", leftPanel);
    m_typeCombo = new QComboBox(leftPanel);
    m_typeCombo->addItem("All types", -1);
    m_typeCombo->addItem("Events", static_cast<int>(ActivityKind::Event));
    m_typeCombo->addItem("Deadlines", static_cast<int>(ActivityKind::Deadline));
    m_typeCombo->addItem("Reminders", static_cast<int>(ActivityKind::Reminder));
    m_typeCombo->addItem("Checklists", static_cast<int>(ActivityKind::Checklist));

    m_resultCountLabel = new QLabel(leftPanel);
    m_resultCountLabel->setObjectName("resultCountLabel");

    m_activityList = new QListWidget(leftPanel);
    m_activityList->setSpacing(4);

    m_toggleCompletedButton = new QPushButton("Mark completed", leftPanel);
    m_toggleCompletedButton->setObjectName("primaryButton");

    m_deleteButton = new QPushButton("Delete activity", leftPanel);
    m_deleteButton->setObjectName("dangerButton");

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addWidget(m_toggleCompletedButton);
    actionLayout->addWidget(m_deleteButton);

    leftLayout->addWidget(searchLabel);
    leftLayout->addWidget(m_searchEdit);
    leftLayout->addWidget(typeLabel);
    leftLayout->addWidget(m_typeCombo);
    leftLayout->addWidget(m_resultCountLabel);
    leftLayout->addWidget(m_activityList);
    leftLayout->addLayout(actionLayout);

    QWidget* rightPanel = new QWidget(splitter);
    rightPanel->setMinimumWidth(620);

    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    QLabel* detailLabel = new QLabel("Activity details", rightPanel);

    m_detailView = new QTextEdit(rightPanel);
    m_detailView->setReadOnly(true);

    rightLayout->addWidget(detailLabel);
    rightLayout->addWidget(m_detailView);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);

    setCentralWidget(centralWidget);
}

void MainWindow::connectSignals()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this]() {
        refreshActivityList();
    });

    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_activityList, &QListWidget::currentRowChanged, this, [this](int currentRow) {
        if (currentRow < 0) {
            showActivityDetails(nullptr);
            updateActionButtons();
            return;
        }

        QListWidgetItem* item = m_activityList->item(currentRow);

        if (!item) {
            showActivityDetails(nullptr);
            updateActionButtons();
            return;
        }

        const QString activityId = item->data(Qt::UserRole).toString();
        showActivityDetails(findActivityById(activityId));
        updateActionButtons();
    });

    connect(m_toggleCompletedButton, &QPushButton::clicked, this, [this]() {
        toggleSelectedActivityCompletion();
    });

    connect(m_deleteButton, &QPushButton::clicked, this, [this]() {
        deleteSelectedActivity();
    });
}

void MainWindow::refreshActivityList()
{
    const QString previousSelectedId = selectedActivityId();

    m_activityList->clear();

    const std::vector<const Activity*> visibleActivities = collectVisibleActivities();

    int rowToSelect = -1;

    for (const Activity* activity : visibleActivities) {
        if (!activity) {
            continue;
        }

        const QString itemText = QString("%1\n%2 | %3 | %4")
                .arg(activity->title())
                .arg(activityKindToString(activity->kind()))
                .arg(activity->primaryDate().toString("yyyy-MM-dd HH:mm"))
                .arg(statusText(activity));

        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, activity->id());
        item->setToolTip(activity->summary());

        const int newRow = m_activityList->count();
        m_activityList->addItem(item);

        if (activity->id() == previousSelectedId) {
            rowToSelect = newRow;
        }
    }

    const QString query = m_searchEdit->text().trimmed();
    const QString typeFilter = m_typeCombo->currentText();

    if (query.isEmpty()) {
        m_resultCountLabel->setText(QString("Activities shown: %1 | Filter: %2")
                                    .arg(visibleActivities.size())
                                    .arg(typeFilter));
    } else {
        m_resultCountLabel->setText(QString("Search results: %1 | Query: \"%2\" | Filter: %3")
                                    .arg(visibleActivities.size())
                                    .arg(query)
                                    .arg(typeFilter));
    }

    if (m_activityList->count() > 0) {
        if (rowToSelect < 0) {
            rowToSelect = 0;
        }

        m_activityList->setCurrentRow(rowToSelect);
    } else {
        if (query.isEmpty()) {
            m_detailView->setPlainText(
                "No activities are available.\n\n"
                "When activity creation is implemented, new activities will appear here."
            );
        } else {
            m_detailView->setPlainText(
                QString("No activities match \"%1\".\n\n"
                        "Try changing the search text or the activity type filter.")
                        .arg(query)
            );
        }
    }

    updateActionButtons();
}

void MainWindow::showActivityDetails(const Activity* activity)
{
    if (!activity) {
        m_detailView->setPlainText(
            "No activity selected.\n\n"
            "Select an activity from the list to view its details."
        );
        return;
    }

    QString details;

    details += QString("%1\n").arg(activity->title());
    details += "----------------------------------------\n\n";

    details += QString("Type: %1\n").arg(activityKindToString(activity->kind()));
    details += QString("Category: %1\n").arg(activity->category().isEmpty() ? "No category" : activity->category());
    details += QString("Priority: %1\n").arg(priorityText(activity->priority()));
    details += QString("Status: %1\n").arg(statusText(activity));
    details += QString("Primary date: %1\n").arg(activity->primaryDate().toString("yyyy-MM-dd HH:mm"));
    details += QString("Created at: %1\n").arg(activity->createdAt().toString("yyyy-MM-dd HH:mm"));
    details += QString("Updated at: %1\n").arg(activity->updatedAt().toString("yyyy-MM-dd HH:mm"));
    details += QString("Recurrence: %1\n").arg(recurrenceText(activity));

    const QDateTime nextOccurrence = activity->nextOccurrenceAfter(QDateTime::currentDateTime());

    details += QString("Next occurrence: %1\n")
            .arg(nextOccurrence.isValid()
                 ? nextOccurrence.toString("yyyy-MM-dd HH:mm")
                 : "No future occurrence");

    details += "\nSummary:\n";
    details += activity->summary();

    if (!activity->description().trimmed().isEmpty()) {
        details += "\n\nDescription:\n";
        details += activity->description();
    }

    m_detailView->setPlainText(details);
}

void MainWindow::updateActionButtons()
{
    const QString activityId = selectedActivityId();
    const Activity* activity = activityId.isEmpty() ? nullptr : findActivityById(activityId);

    const bool hasSelection = activity != nullptr;

    if (m_toggleCompletedButton) {
        m_toggleCompletedButton->setEnabled(hasSelection);

        if (activity && activity->isCompleted()) {
            m_toggleCompletedButton->setText("Mark active");
        } else {
            m_toggleCompletedButton->setText("Mark completed");
        }
    }

    if (m_deleteButton) {
        m_deleteButton->setEnabled(hasSelection);
    }
}

std::vector<const Activity*> MainWindow::collectVisibleActivities() const
{
    if (!m_activityManager) {
        return {};
    }

    ActivityFilter::Criteria criteria;
    criteria.sortKey = ActivityFilter::SortKey::PrimaryDate;
    criteria.sortOrder = ActivityFilter::SortOrder::Ascending;

    const int selectedKindValue = m_typeCombo->currentData().toInt();

    if (selectedKindValue >= 0) {
        criteria.kind = static_cast<ActivityKind>(selectedKindValue);
    }

    std::vector<const Activity*> filteredActivities =
            ActivityFilter::apply(m_activityManager->activities(),
                                  criteria,
                                  QDateTime::currentDateTime());

    const QString query = m_searchEdit->text().trimmed();

    if (query.isEmpty()) {
        return filteredActivities;
    }

    const SearchEngine::SearchResponse searchResponse =
            SearchEngine::search(filteredActivities, query);

    std::vector<const Activity*> result;
    result.reserve(searchResponse.results.size());

    for (const SearchEngine::SearchResult& searchResult : searchResponse.results) {
        result.push_back(searchResult.activity);
    }

    return result;
}

const Activity* MainWindow::findActivityById(const QString& id) const
{
    if (!m_activityManager) {
        return nullptr;
    }

    return m_activityManager->findActivityById(id);
}

QString MainWindow::selectedActivityId() const
{
    if (!m_activityList) {
        return QString();
    }

    QListWidgetItem* item = m_activityList->currentItem();

    if (!item) {
        return QString();
    }

    return item->data(Qt::UserRole).toString();
}

QString MainWindow::statusText(const Activity* activity) const
{
    if (!activity) {
        return "Invalid";
    }

    if (activity->isCompleted()) {
        return "Completed";
    }

    if (activity->isOverdue(QDateTime::currentDateTime())) {
        return "Overdue";
    }

    return "Active";
}

QString MainWindow::priorityText(Priority priority) const
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

QString MainWindow::recurrenceText(const Activity* activity) const
{
    if (!activity || !activity->hasRecurrence()) {
        return "No recurrence";
    }

    return activity->recurrenceRule()->toDisplayString();
}

void MainWindow::toggleSelectedActivityCompletion()
{
    if (!m_activityManager) {
        return;
    }

    const QString activityId = selectedActivityId();

    if (activityId.isEmpty()) {
        return;
    }

    Activity* activity = m_activityManager->findActivityById(activityId);

    if (!activity) {
        return;
    }

    activity->setCompleted(!activity->isCompleted());

    refreshActivityList();

    const Activity* updatedActivity = findActivityById(activityId);
    showActivityDetails(updatedActivity);
    updateActionButtons();
}

void MainWindow::deleteSelectedActivity()
{
    if (!m_activityManager) {
        return;
    }

    const QString activityId = selectedActivityId();

    if (activityId.isEmpty()) {
        return;
    }

    const Activity* activity = m_activityManager->findActivityById(activityId);

    if (!activity) {
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Delete activity",
        QString("Do you really want to delete \"%1\"?").arg(activity->title()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    m_activityManager->removeActivity(activityId);

    refreshActivityList();
}