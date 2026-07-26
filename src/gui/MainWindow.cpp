// MainWindow implementation: menus, list refresh, details panel and user actions.

#include "MainWindow.h"

#include "ActivityCreationPage.h"
#include "ActivityEditPage.h"
#include "CategoryManagementDialog.h"
#include "commands/AddActivityCommand.h"
#include "commands/RemoveActivityCommand.h"
#include "commands/ToggleCompletionCommand.h"
#include "commands/UpdateActivityCommand.h"
#include "model/ActivityFilter.h"
#include "model/ActivityManager.h"
#include "model/ActivityTemplate.h"
#include "model/ActivityTemplateManager.h"
#include "model/Category.h"
#include "model/CategoryManager.h"
#include "model/SearchEngine.h"
#include "persistence/AgendaJsonStorage.h"

#include <QAction>
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSize>
#include <QSplitter>
#include <QStatusBar>
#include <QStackedWidget>
#include <QStringList>
#include <QTextEdit>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

MainWindow::MainWindow(ActivityManager* activityManager,
                       ActivityTemplateManager* templateManager,
                       CategoryManager* categoryManager,
                       QWidget* parent)
    : QMainWindow(parent),
      m_activityManager(activityManager),
      m_templateManager(templateManager),
      m_categoryManager(categoryManager)
{
    setupUi();
    setupMenuBar();
    connectSignals();
    synchronizeCategoryManagerFromActivities();
    refreshActivityList();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (confirmDiscardUnsavedChanges()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    updateResponsiveWorkspace();
}

void MainWindow::setupUi()
{
    setWindowTitle("Agenda Qt");
    resize(1020, 630);
    setMinimumSize(780, 480);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_pageStack = new QStackedWidget(centralWidget);
    m_pageStack->setObjectName("mainPageStack");

    m_agendaPage = new QWidget(m_pageStack);
    m_agendaPage->setObjectName("agendaPage");

    QVBoxLayout* mainLayout = new QVBoxLayout(m_agendaPage);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    m_mainSplitter = new QSplitter(m_agendaPage);
    m_mainSplitter->setChildrenCollapsible(false);

    m_leftScrollArea = new QScrollArea(m_mainSplitter);
    m_leftScrollArea->setWidgetResizable(true);
    m_leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_leftScrollArea->setFrameShape(QFrame::NoFrame);
    m_leftScrollArea->setMinimumWidth(320);

    QWidget* leftPanel = new QWidget(m_leftScrollArea);
    leftPanel->setMinimumWidth(340);

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 6, 0);
    leftLayout->setSpacing(8);

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

    QPushButton* filterToggleButton = new QPushButton("Show filters and sorting", leftPanel);
    filterToggleButton->setCheckable(true);
    filterToggleButton->setObjectName("filterToggleButton");

    m_priorityCombo = new QComboBox(leftPanel);
    m_priorityCombo->addItem("All priorities", -1);
    m_priorityCombo->addItem("Low", static_cast<int>(Priority::Low));
    m_priorityCombo->addItem("Medium", static_cast<int>(Priority::Medium));
    m_priorityCombo->addItem("High", static_cast<int>(Priority::High));
    m_priorityCombo->addItem("Critical", static_cast<int>(Priority::Critical));

    m_categoryCombo = new QComboBox(leftPanel);
    m_categoryCombo->addItem("All categories", QString());

    m_completionCombo = new QComboBox(leftPanel);
    m_completionCombo->addItem("All statuses", 0);
    m_completionCombo->addItem("Active only", 1);
    m_completionCombo->addItem("Completed only", 2);

    m_recurrenceCombo = new QComboBox(leftPanel);
    m_recurrenceCombo->addItem("All recurrence", 0);
    m_recurrenceCombo->addItem("Recurring only", 1);
    m_recurrenceCombo->addItem("Non-recurring only", 2);

    m_overdueCombo = new QComboBox(leftPanel);
    m_overdueCombo->addItem("All due states", 0);
    m_overdueCombo->addItem("Overdue only", 1);
    m_overdueCombo->addItem("Not overdue only", 2);

    m_sortCombo = new QComboBox(leftPanel);
    m_sortCombo->addItem("Date ascending", "date_asc");
    m_sortCombo->addItem("Date descending", "date_desc");
    m_sortCombo->addItem("Title A-Z", "title_asc");
    m_sortCombo->addItem("Title Z-A", "title_desc");
    m_sortCombo->addItem("Priority high first", "priority_desc");
    m_sortCombo->addItem("Priority low first", "priority_asc");
    m_sortCombo->addItem("Active first", "completion_asc");
    m_sortCombo->addItem("Completed first", "completion_desc");
    m_sortCombo->addItem("Recently created", "created_desc");
    m_sortCombo->addItem("Recently updated", "updated_desc");

    QGroupBox* filterPanel = new QGroupBox("Filters and sorting", leftPanel);
    filterPanel->setObjectName("filterPanel");
    filterPanel->setVisible(false);

    QGridLayout* filterGridLayout = new QGridLayout(filterPanel);
    filterGridLayout->setContentsMargins(10, 10, 10, 10);
    filterGridLayout->setHorizontalSpacing(10);
    filterGridLayout->setVerticalSpacing(8);

    auto createFilterCell = [leftPanel](const QString& labelText, QComboBox* comboBox) {
        QWidget* cell = new QWidget(leftPanel);
        QVBoxLayout* cellLayout = new QVBoxLayout(cell);
        cellLayout->setContentsMargins(0, 0, 0, 0);
        cellLayout->setSpacing(3);

        QLabel* label = new QLabel(labelText, cell);
        label->setObjectName("filterFieldLabel");

        cellLayout->addWidget(label);
        cellLayout->addWidget(comboBox);
        return cell;
    };

    filterGridLayout->addWidget(createFilterCell("Priority", m_priorityCombo), 0, 0);
    filterGridLayout->addWidget(createFilterCell("Category", m_categoryCombo), 0, 1);
    filterGridLayout->addWidget(createFilterCell("Status", m_completionCombo), 1, 0);
    filterGridLayout->addWidget(createFilterCell("Recurrence", m_recurrenceCombo), 1, 1);
    filterGridLayout->addWidget(createFilterCell("Due state", m_overdueCombo), 2, 0);
    filterGridLayout->addWidget(createFilterCell("Sort by", m_sortCombo), 2, 1);
    filterGridLayout->setColumnStretch(0, 1);
    filterGridLayout->setColumnStretch(1, 1);

    connect(filterToggleButton, &QPushButton::toggled, this, [filterToggleButton, filterPanel](bool checked) {
        filterPanel->setVisible(checked);
        filterToggleButton->setText(checked ? "Hide filters and sorting" : "Show filters and sorting");
    });

    m_resultCountLabel = new QLabel(leftPanel);
    m_resultCountLabel->setObjectName("resultCountLabel");

    m_activityList = new QListWidget(leftPanel);
    m_activityList->setObjectName("activityList");
    m_activityList->setSpacing(4);
    m_activityList->setMinimumHeight(150);

    m_addButton = new QPushButton("Add activity", leftPanel);
    m_addButton->setObjectName("primaryButton");

    m_editButton = new QPushButton("Edit activity", leftPanel);
    m_editButton->setObjectName("primaryButton");

    m_templateButton = new QPushButton("From template", leftPanel);
    m_templateButton->setObjectName("primaryButton");

    m_toggleCompletedButton = new QPushButton("Mark completed", leftPanel);
    m_toggleCompletedButton->setObjectName("primaryButton");

    m_deleteButton = new QPushButton("Delete activity", leftPanel);
    m_deleteButton->setObjectName("dangerButton");

    m_undoButton = new QPushButton("Undo", leftPanel);
    m_undoButton->setObjectName("primaryButton");

    m_redoButton = new QPushButton("Redo", leftPanel);
    m_redoButton->setObjectName("primaryButton");

    QVBoxLayout* actionLayout = new QVBoxLayout();
    actionLayout->setSpacing(10);
    actionLayout->setContentsMargins(0, 6, 0, 0);

    m_addButton->setObjectName("accentButton");
    actionLayout->addWidget(m_addButton);
    actionLayout->addSpacing(6);

    QGridLayout* actionGridLayout = new QGridLayout();
    actionGridLayout->setContentsMargins(0, 0, 0, 0);
    actionGridLayout->setHorizontalSpacing(10);
    actionGridLayout->setVerticalSpacing(10);
    actionGridLayout->addWidget(m_editButton, 0, 0);
    actionGridLayout->addWidget(m_templateButton, 0, 1);
    actionGridLayout->addWidget(m_toggleCompletedButton, 1, 0);
    actionGridLayout->addWidget(m_deleteButton, 1, 1);
    actionGridLayout->addWidget(m_undoButton, 2, 0);
    actionGridLayout->addWidget(m_redoButton, 2, 1);
    actionGridLayout->setColumnStretch(0, 1);
    actionGridLayout->setColumnStretch(1, 1);
    actionLayout->addLayout(actionGridLayout);

    leftLayout->addWidget(searchLabel);
    leftLayout->addWidget(m_searchEdit);
    leftLayout->addWidget(typeLabel);
    leftLayout->addWidget(m_typeCombo);
    leftLayout->addWidget(filterToggleButton);
    leftLayout->addWidget(filterPanel);
    leftLayout->addWidget(m_resultCountLabel);
    leftLayout->addWidget(m_activityList, 1);
    leftLayout->addLayout(actionLayout);

    m_workspaceStack = new QStackedWidget(m_mainSplitter);
    m_workspaceStack->setObjectName("workspaceStack");
    m_workspaceStack->setMinimumWidth(280);

    m_detailPage = new QWidget(m_workspaceStack);
    m_detailPage->setObjectName("activityDetailPage");

    QVBoxLayout* rightLayout = new QVBoxLayout(m_detailPage);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    QLabel* detailLabel = new QLabel("Activity details", m_detailPage);
    detailLabel->setObjectName("sectionLabel");

    m_detailView = new QTextEdit(m_detailPage);
    m_detailView->setObjectName("activityDetailView");
    m_detailView->setReadOnly(true);

    rightLayout->addWidget(detailLabel);
    rightLayout->addWidget(m_detailView, 1);

    m_workspaceStack->addWidget(m_detailPage);

    m_leftScrollArea->setWidget(leftPanel);

    m_mainSplitter->addWidget(m_leftScrollArea);
    m_mainSplitter->addWidget(m_workspaceStack);
    m_mainSplitter->setStretchFactor(0, 12);
    m_mainSplitter->setStretchFactor(1, 8);
    m_mainSplitter->setSizes({590, 360});

    mainLayout->addWidget(m_mainSplitter, 1);

    m_pageStack->addWidget(m_agendaPage);

    m_creationPage = new ActivityCreationPage(m_categoryManager, m_workspaceStack);
    m_creationPage->setCreatedHandler([this](std::unique_ptr<Activity> activity) {
        addCreatedActivity(std::move(activity));
    });
    m_creationPage->setCancelHandler([this]() {
        openAgendaPage();
    });
    m_workspaceStack->addWidget(m_creationPage);

    m_editingPage = new ActivityEditPage(m_categoryManager, m_workspaceStack);
    m_editingPage->setSavedHandler(
        [this](const QString& activityId, std::unique_ptr<Activity> activity) {
            return applyEditedActivity(activityId, std::move(activity));
        });
    m_editingPage->setCancelHandler([this]() {
        openAgendaPage();
    });
    m_workspaceStack->addWidget(m_editingPage);

    rootLayout->addWidget(m_pageStack, 1);
    setCentralWidget(centralWidget);
    openAgendaPage();
}

void MainWindow::openAgendaPage()
{
    showPage(m_detailPage);
}

void MainWindow::openCreationPage()
{
    if (!m_creationPage) {
        return;
    }

    synchronizeCategoryManagerFromActivities();
    m_creationPage->resetForm();
    showPage(m_creationPage);
}

void MainWindow::openEditingPage()
{
    showPage(m_editingPage);
}

void MainWindow::showPage(QWidget* page)
{
    if (!m_workspaceStack || !page || m_workspaceStack->indexOf(page) < 0) {
        return;
    }

    m_workspaceStack->setCurrentWidget(page);
    updateResponsiveWorkspace();
    updateActionButtons();
}

void MainWindow::updateResponsiveWorkspace()
{
    if (!m_mainSplitter || !m_leftScrollArea || !m_workspaceStack || !m_detailPage) {
        return;
    }

    const bool workflowActive = m_workspaceStack->currentWidget() != m_detailPage;
    const bool compactWindow = width() < 1050;
    const bool showAgendaPanel = !(workflowActive && compactWindow);

    m_leftScrollArea->setVisible(showAgendaPanel);

    const int totalWidth = std::max(1, m_mainSplitter->width());

    if (!showAgendaPanel) {
        m_mainSplitter->setSizes({0, totalWidth});
        return;
    }

    int leftWidth = workflowActive
        ? totalWidth * 32 / 100
        : totalWidth * 60 / 100;

    if (workflowActive) {
        leftWidth = std::max(320, std::min(leftWidth, 420));
    } else {
        leftWidth = std::max(360, std::min(leftWidth, 650));
    }

    m_mainSplitter->setSizes({leftWidth, std::max(1, totalWidth - leftWidth)});
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* loadAction = fileMenu->addAction("Load...");
    QAction* saveAction = fileMenu->addAction("Save");
    QAction* saveAsAction = fileMenu->addAction("Save As...");

    loadAction->setShortcut(QKeySequence("Ctrl+O"));
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));

    connect(loadAction, &QAction::triggered, this, [this]() {
        loadAgenda();
    });

    connect(saveAction, &QAction::triggered, this, [this]() {
        saveAgenda();
    });

    connect(saveAsAction, &QAction::triggered, this, [this]() {
        saveAgendaAs();
    });

    QMenu* templatesMenu = menuBar()->addMenu("Templates");

    QAction* createFromTemplateAction = templatesMenu->addAction("Create from template...");
    QAction* saveAsTemplateAction = templatesMenu->addAction("Save selected as template...");

    createFromTemplateAction->setShortcut(QKeySequence("Ctrl+T"));
    saveAsTemplateAction->setShortcut(QKeySequence("Ctrl+Shift+T"));

    connect(createFromTemplateAction, &QAction::triggered, this, [this]() {
        createActivityFromTemplate();
    });

    connect(saveAsTemplateAction, &QAction::triggered, this, [this]() {
        saveSelectedActivityAsTemplate();
    });

    QMenu* categoriesMenu = menuBar()->addMenu("Categories");

    QAction* manageCategoriesAction = categoriesMenu->addAction("Manage categories...");
    manageCategoriesAction->setShortcut(QKeySequence("Ctrl+Shift+C"));

    connect(manageCategoriesAction, &QAction::triggered, this, [this]() {
        manageCategories();
    });

    QMenu* editMenu = menuBar()->addMenu("Edit");

    m_undoAction = editMenu->addAction("Undo");
    m_redoAction = editMenu->addAction("Redo");

    m_undoAction->setShortcut(QKeySequence::Undo);
    m_redoAction->setShortcut(QKeySequence::Redo);

    connect(m_undoAction, &QAction::triggered, this, [this]() {
        undoLastCommand();
    });

    connect(m_redoAction, &QAction::triggered, this, [this]() {
        redoLastCommand();
    });
}

void MainWindow::connectSignals()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this]() {
        refreshActivityList();
    });

    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_priorityCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_categoryCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_completionCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_recurrenceCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_overdueCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_sortCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_activityList, &QListWidget::currentRowChanged, this, [this](int currentRow) {
        if (currentRow < 0) {
            updateActionButtons();
            return;
        }

        QListWidgetItem* item = m_activityList->item(currentRow);

        if (!item) {
            updateActionButtons();
            return;
        }

        updateActionButtons();
    });

    connect(m_addButton, &QPushButton::clicked, this, [this]() {
        createActivity();
    });

    connect(m_templateButton, &QPushButton::clicked, this, [this]() {
        createActivityFromTemplate();
    });

    connect(m_toggleCompletedButton, &QPushButton::clicked, this, [this]() {
        toggleSelectedActivityCompletion();
    });

    connect(m_deleteButton, &QPushButton::clicked, this, [this]() {
        deleteSelectedActivity();
    });

    connect(m_editButton, &QPushButton::clicked, this, [this]() {
        editSelectedActivity();
    });

    connect(m_undoButton, &QPushButton::clicked, this, [this]() {
        undoLastCommand();
    });

    connect(m_redoButton, &QPushButton::clicked, this, [this]() {
        redoLastCommand();
    });
}

void MainWindow::refreshActivityList()
{
    const QString previousSelectedId = selectedActivityId();

    updateCategoryFilterOptions();

    m_activityList->clear();

    const std::vector<const Activity*> visibleActivities = collectVisibleActivities();

    int rowToSelect = -1;

    for (const Activity* activity : visibleActivities) {
        if (!activity) {
            continue;
        }

        // The presentation controller renders the concrete card through ActivityVisitor.
        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, activity->id());
        item->setToolTip(activity->summary());

        const int newRow = m_activityList->count();
        m_activityList->addItem(item);

        if (activity->id() == previousSelectedId) {
            rowToSelect = newRow;
        }
    }

    const QString query = m_searchEdit->text().trimmed();
    const QString sortText = m_sortCombo ? m_sortCombo->currentText() : "Default";

    if (query.isEmpty()) {
        m_resultCountLabel->setText(QString("Activities shown: %1 | Sort: %2")
                                    .arg(visibleActivities.size())
                                    .arg(sortText));
    } else {
        m_resultCountLabel->setText(QString("Search results: %1 | Query: \"%2\" | Sort: %3")
                                    .arg(visibleActivities.size())
                                    .arg(query)
                                    .arg(sortText));
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
                "Use Add activity or From template to create a new activity."
            );
        } else {
            m_detailView->setPlainText(
                QString("No activities match \"%1\".\n\n"
                        "Try changing the search text or the active filters.")
                        .arg(query)
            );
        }
    }

    updateActionButtons();
}

void MainWindow::updateActionButtons()
{
    const bool workflowActive = m_workspaceStack &&
        m_detailPage &&
        m_workspaceStack->currentWidget() != m_detailPage;

    if (m_addButton) {
        m_addButton->setEnabled(m_activityManager != nullptr && !workflowActive);
    }

    if (m_templateButton) {
        m_templateButton->setEnabled(m_activityManager != nullptr &&
                                     m_templateManager != nullptr &&
                                     !m_templateManager->isEmpty() &&
                                     !workflowActive);
    }

    const QString activityId = selectedActivityId();
    const Activity* activity = activityId.isEmpty() ? nullptr : findActivityById(activityId);
    const bool hasSelection = activity != nullptr && !workflowActive;

    if (m_toggleCompletedButton) {
        m_toggleCompletedButton->setEnabled(hasSelection);
        m_toggleCompletedButton->setText(
            activity && activity->isCompleted() ? "Mark active" : "Mark completed"
        );
    }

    if (m_editButton) {
        m_editButton->setEnabled(hasSelection);
    }

    if (m_deleteButton) {
        m_deleteButton->setEnabled(hasSelection);
    }

    if (m_undoButton) {
        m_undoButton->setEnabled(!workflowActive && m_commandHistory.canUndo());
        m_undoButton->setToolTip(m_commandHistory.undoDescription());
    }

    if (m_redoButton) {
        m_redoButton->setEnabled(!workflowActive && m_commandHistory.canRedo());
        m_redoButton->setToolTip(m_commandHistory.redoDescription());
    }

    if (m_undoAction) {
        m_undoAction->setEnabled(!workflowActive && m_commandHistory.canUndo());
        m_undoAction->setText(m_commandHistory.undoDescription());
    }

    if (m_redoAction) {
        m_redoAction->setEnabled(!workflowActive && m_commandHistory.canRedo());
        m_redoAction->setText(m_commandHistory.redoDescription());
    }
}
void MainWindow::updateCategoryFilterOptions()
{
    if (!m_categoryCombo) {
        return;
    }

    const QString previousCategory = m_categoryCombo->currentData().toString();

    QStringList categories;

    if (m_categoryManager) {
        for (const Category& category : m_categoryManager->categories()) {
            const QString categoryName = category.name().trimmed();

            if (!categoryName.isEmpty() && !categories.contains(categoryName, Qt::CaseInsensitive)) {
                categories.append(categoryName);
            }
        }
    }

    if (m_activityManager) {
        for (const Activity* activity : m_activityManager->activities()) {
            if (!activity) {
                continue;
            }

            const QString category = activity->category().trimmed();

            if (!category.isEmpty() && !categories.contains(category, Qt::CaseInsensitive)) {
                categories.append(category);
            }
        }
    }

    std::sort(categories.begin(), categories.end(), [](const QString& first, const QString& second) {
        return QString::localeAwareCompare(first, second) < 0;
    });

    m_categoryCombo->blockSignals(true);
    m_categoryCombo->clear();
    m_categoryCombo->addItem("All categories", QString());

    for (const QString& category : categories) {
        m_categoryCombo->addItem(category, category);
    }

    const int previousIndex = m_categoryCombo->findData(previousCategory);

    if (previousIndex >= 0) {
        m_categoryCombo->setCurrentIndex(previousIndex);
    } else {
        m_categoryCombo->setCurrentIndex(0);
    }

    m_categoryCombo->blockSignals(false);
}

void MainWindow::synchronizeCategoryManagerFromActivities()
{
    if (!m_categoryManager || !m_activityManager) {
        return;
    }

    for (const Activity* activity : m_activityManager->activities()) {
        if (!activity) {
            continue;
        }

        const QString category = activity->category().trimmed();

        if (!category.isEmpty() && !m_categoryManager->containsName(category)) {
            m_categoryManager->addCategory(category);
        }
    }
}

std::vector<const Activity*> MainWindow::collectVisibleActivities() const
{
    if (!m_activityManager) {
        return {};
    }

    ActivityFilter::Criteria criteria;

    if (m_typeCombo) {
        const int selectedKindValue = m_typeCombo->currentData().toInt();

        // ActivityKind is retained here only as user-selected filter metadata.
        if (selectedKindValue >= 0) {
            criteria.kind = static_cast<ActivityKind>(selectedKindValue);
        }
    }

    if (m_priorityCombo) {
        const int selectedPriorityValue = m_priorityCombo->currentData().toInt();

        if (selectedPriorityValue >= 0) {
            criteria.priority = static_cast<Priority>(selectedPriorityValue);
        }
    }

    if (m_categoryCombo) {
        const QString selectedCategory = m_categoryCombo->currentData().toString().trimmed();

        if (!selectedCategory.isEmpty()) {
            criteria.category = selectedCategory;
        }
    }

    if (m_completionCombo) {
        const int completionValue = m_completionCombo->currentData().toInt();

        if (completionValue == 1) {
            criteria.completion = ActivityFilter::CompletionFilter::ActiveOnly;
        } else if (completionValue == 2) {
            criteria.completion = ActivityFilter::CompletionFilter::CompletedOnly;
        }
    }

    if (m_recurrenceCombo) {
        const int recurrenceValue = m_recurrenceCombo->currentData().toInt();

        if (recurrenceValue == 1) {
            criteria.recurring = true;
        } else if (recurrenceValue == 2) {
            criteria.recurring = false;
        }
    }

    if (m_overdueCombo) {
        const int overdueValue = m_overdueCombo->currentData().toInt();

        if (overdueValue == 1) {
            criteria.overdue = ActivityFilter::OverdueFilter::OverdueOnly;
        } else if (overdueValue == 2) {
            criteria.overdue = ActivityFilter::OverdueFilter::NotOverdueOnly;
        }
    }

    const QString sortValue = m_sortCombo
            ? m_sortCombo->currentData().toString()
            : "date_asc";

    if (sortValue == "date_desc") {
        criteria.sortKey = ActivityFilter::SortKey::PrimaryDate;
        criteria.sortOrder = ActivityFilter::SortOrder::Descending;
    } else if (sortValue == "title_asc") {
        criteria.sortKey = ActivityFilter::SortKey::Title;
        criteria.sortOrder = ActivityFilter::SortOrder::Ascending;
    } else if (sortValue == "title_desc") {
        criteria.sortKey = ActivityFilter::SortKey::Title;
        criteria.sortOrder = ActivityFilter::SortOrder::Descending;
    } else if (sortValue == "priority_desc") {
        criteria.sortKey = ActivityFilter::SortKey::Priority;
        criteria.sortOrder = ActivityFilter::SortOrder::Descending;
    } else if (sortValue == "priority_asc") {
        criteria.sortKey = ActivityFilter::SortKey::Priority;
        criteria.sortOrder = ActivityFilter::SortOrder::Ascending;
    } else if (sortValue == "completion_asc") {
        criteria.sortKey = ActivityFilter::SortKey::Completion;
        criteria.sortOrder = ActivityFilter::SortOrder::Ascending;
    } else if (sortValue == "completion_desc") {
        criteria.sortKey = ActivityFilter::SortKey::Completion;
        criteria.sortOrder = ActivityFilter::SortOrder::Descending;
    } else if (sortValue == "created_desc") {
        criteria.sortKey = ActivityFilter::SortKey::CreatedAt;
        criteria.sortOrder = ActivityFilter::SortOrder::Descending;
    } else if (sortValue == "updated_desc") {
        criteria.sortKey = ActivityFilter::SortKey::UpdatedAt;
        criteria.sortOrder = ActivityFilter::SortOrder::Descending;
    } else {
        criteria.sortKey = ActivityFilter::SortKey::PrimaryDate;
        criteria.sortOrder = ActivityFilter::SortOrder::Ascending;
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

    auto command = std::make_unique<ToggleCompletionCommand>(
        m_activityManager,
        activityId
    );

    if (!m_commandHistory.executeCommand(std::move(command))) {
        QMessageBox::warning(this, "Update activity failed", "The activity status could not be changed.");
        return;
    }

    setUnsavedChanges(true);
    synchronizeCategoryManagerFromActivities();

    refreshActivityList();

    const Activity* updatedActivity = findActivityById(activityId);
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

    auto command = std::make_unique<RemoveActivityCommand>(
        m_activityManager,
        activityId
    );

    if (!m_commandHistory.executeCommand(std::move(command))) {
        QMessageBox::warning(this, "Delete activity failed", "The activity could not be deleted.");
        return;
    }

    setUnsavedChanges(true);
    synchronizeCategoryManagerFromActivities();

    refreshActivityList();
}

QString MainWindow::fileDisplayName(const QString& filePath) const
{
    const QFileInfo fileInfo(filePath);

    if (!fileInfo.fileName().trimmed().isEmpty()) {
        return fileInfo.fileName();
    }

    return filePath;
}

QString MainWindow::storageSummaryText() const
{
    const int activityCount = m_activityManager ? m_activityManager->size() : 0;
    const int templateCount = m_templateManager ? m_templateManager->size() : 0;
    const int categoryCount = m_categoryManager ? m_categoryManager->size() : 0;

    const QString activityWord = activityCount == 1 ? "activity" : "activities";
    const QString templateWord = templateCount == 1 ? "template" : "templates";
    const QString categoryWord = categoryCount == 1 ? "category" : "categories";

    return QString("%1 %2, %3 %4, %5 %6")
        .arg(activityCount)
        .arg(activityWord)
        .arg(templateCount)
        .arg(templateWord)
        .arg(categoryCount)
        .arg(categoryWord);
}

void MainWindow::manageCategories()
{
    if (!m_activityManager || !m_categoryManager) {
        QMessageBox::warning(
            this,
            "Categories unavailable",
            "Categories cannot be managed because the internal managers are not available."
        );
        return;
    }

    synchronizeCategoryManagerFromActivities();

    CategoryManagementDialog dialog(m_categoryManager, m_activityManager, this);
    dialog.exec();

    if (dialog.activitiesChanged() || dialog.categoriesChanged()) {
        setUnsavedChanges(true);
    }

    updateCategoryFilterOptions();
    refreshActivityList();
    updateActionButtons();

    if (dialog.activitiesChanged() || dialog.categoriesChanged()) {
        statusBar()->showMessage("Categories updated", 3000);
    }
}

bool MainWindow::saveAgenda()
{
    if (!m_activityManager || !m_templateManager || !m_categoryManager) {
        QMessageBox::warning(
            this,
            "Save failed",
            "The agenda cannot be saved because the internal managers are not available."
        );
        statusBar()->showMessage("Save failed: internal data is not available", 4000);
        return false;
    }

    if (m_currentFilePath.trimmed().isEmpty()) {
        return saveAgendaAs();
    }

    QString errorMessage;

    synchronizeCategoryManagerFromActivities();

    const bool saved = AgendaJsonStorage::saveToFile(
        *m_activityManager,
        *m_templateManager,
        *m_categoryManager,
        m_currentFilePath,
        &errorMessage
    );

    const QString displayName = fileDisplayName(m_currentFilePath);

    if (!saved) {
        QMessageBox::warning(
            this,
            "Save failed",
            QString("Could not save the agenda file.\n\nFile: %1\nPath: %2\n\nReason: %3")
                .arg(displayName, m_currentFilePath, errorMessage)
        );
        statusBar()->showMessage(QString("Save failed: %1").arg(displayName), 4000);
        return false;
    }

    setUnsavedChanges(false);
    updateWindowTitle();

    statusBar()->showMessage(
        QString("Saved %1 (%2)").arg(displayName, storageSummaryText()),
        5000
    );

    return true;
}

bool MainWindow::saveAgendaAs()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save agenda",
        QString(),
        "Agenda JSON (*.json);;All files (*)"
    );

    if (filePath.trimmed().isEmpty()) {
        return false;
    }

    const QString previousFilePath = m_currentFilePath;
    m_currentFilePath = filePath;

    if (!saveAgenda()) {
        m_currentFilePath = previousFilePath;
        updateWindowTitle();
        return false;
    }

    return true;
}

void MainWindow::loadAgenda()
{
    if (!m_activityManager || !m_templateManager || !m_categoryManager) {
        QMessageBox::warning(
            this,
            "Load failed",
            "The agenda cannot be loaded because the internal managers are not available."
        );
        statusBar()->showMessage("Load failed: internal data is not available", 4000);
        return;
    }

    if (!confirmDiscardUnsavedChanges()) {
        return;
    }

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Load agenda",
        QString(),
        "Agenda JSON (*.json);;All files (*)"
    );

    if (filePath.trimmed().isEmpty()) {
        return;
    }

    QString errorMessage;

    const bool loaded = AgendaJsonStorage::loadFromFile(
        *m_activityManager,
        *m_templateManager,
        *m_categoryManager,
        filePath,
        &errorMessage
    );

    const QString displayName = fileDisplayName(filePath);

    if (!loaded) {
        QMessageBox::warning(
            this,
            "Load failed",
            QString("Could not load the agenda file.\n\nFile: %1\nPath: %2\n\nReason: %3")
                .arg(displayName, filePath, errorMessage)
        );
        statusBar()->showMessage(QString("Load failed: %1").arg(displayName), 4000);
        return;
    }

    m_currentFilePath = filePath;
    setUnsavedChanges(false);
    m_commandHistory.clear();
    synchronizeCategoryManagerFromActivities();

    m_searchEdit->clear();
    m_typeCombo->setCurrentIndex(0);
    m_priorityCombo->setCurrentIndex(0);
    m_categoryCombo->setCurrentIndex(0);
    m_completionCombo->setCurrentIndex(0);
    m_recurrenceCombo->setCurrentIndex(0);
    m_overdueCombo->setCurrentIndex(0);
    m_sortCombo->setCurrentIndex(0);

    refreshActivityList();
    updateWindowTitle();
    updateActionButtons();

    statusBar()->showMessage(
        QString("Loaded %1 (%2)").arg(displayName, storageSummaryText()),
        5000
    );
}

void MainWindow::updateWindowTitle()
{
    const QString dirtyMarker = m_hasUnsavedChanges ? " *" : "";

    if (m_currentFilePath.trimmed().isEmpty()) {
        setWindowTitle(QString("Agenda Qt - Untitled%1").arg(dirtyMarker));
        return;
    }

    setWindowTitle(QString("Agenda Qt - %1%2")
                   .arg(m_currentFilePath)
                   .arg(dirtyMarker));
}

void MainWindow::setUnsavedChanges(bool hasUnsavedChanges)
{
    if (m_hasUnsavedChanges == hasUnsavedChanges) {
        return;
    }

    m_hasUnsavedChanges = hasUnsavedChanges;
    updateWindowTitle();

    if (m_hasUnsavedChanges) {
        statusBar()->showMessage("Unsaved changes", 2000);
    }
}

bool MainWindow::confirmDiscardUnsavedChanges()
{
    if (!m_hasUnsavedChanges) {
        return true;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Unsaved changes",
        "There are unsaved changes. Do you want to discard them?",
        QMessageBox::Yes | QMessageBox::No
    );

    return answer == QMessageBox::Yes;
}

void MainWindow::createActivity()
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

void MainWindow::editSelectedActivity()
{
    if (!m_activityManager || !m_editingPage) {
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

    synchronizeCategoryManagerFromActivities();
    m_editingPage->editActivity(*activity);
    openEditingPage();
}

bool MainWindow::applyEditedActivity(const QString& activityId,
                                     std::unique_ptr<Activity> activity)
{
    if (!m_activityManager || activityId.isEmpty() || !activity) {
        return false;
    }

    auto command = std::make_unique<UpdateActivityCommand>(
        m_activityManager,
        activityId,
        std::move(activity)
    );

    if (!m_commandHistory.executeCommand(std::move(command))) {
        QMessageBox::warning(this, "Edit activity failed", "The activity could not be updated.");
        return false;
    }

    setUnsavedChanges(true);
    synchronizeCategoryManagerFromActivities();
    openAgendaPage();
    refreshActivityList();

    for (int row = 0; row < m_activityList->count(); ++row) {
        QListWidgetItem* item = m_activityList->item(row);

        if (item && item->data(Qt::UserRole).toString() == activityId) {
            m_activityList->setCurrentRow(row);
            break;
        }
    }

    updateActionButtons();
    statusBar()->showMessage("Activity updated", 3000);
    return true;
}

void MainWindow::createActivityFromTemplate()
{
    if (!m_activityManager || !m_templateManager) {
        return;
    }

    const std::vector<const ActivityTemplate*> templates = m_templateManager->templates();

    if (templates.empty()) {
        QMessageBox::information(
            this,
            "No templates",
            "No activity templates are available in the current session."
        );
        return;
    }

    QStringList templateNames;

    for (const ActivityTemplate* activityTemplate : templates) {
        if (activityTemplate && activityTemplate->isValid()) {
            templateNames.append(activityTemplate->name());
        }
    }

    if (templateNames.isEmpty()) {
        QMessageBox::information(
            this,
            "No templates",
            "No valid activity templates are available."
        );
        return;
    }

    bool ok = false;

    const QString selectedTemplateName = QInputDialog::getItem(
        this,
        "Create from template",
        "Template",
        templateNames,
        0,
        false,
        &ok
    );

    if (!ok || selectedTemplateName.trimmed().isEmpty()) {
        return;
    }

    /*
     * Il template manager crea una nuova attività clonando il prototipo.
     * Ho scelto di passare da questo metodo per mantenere la logica dei template
     * nel modello e non dentro la GUI.
     */
    std::unique_ptr<Activity> activity =
        m_templateManager->createActivityFromTemplateName(selectedTemplateName);

    if (!activity) {
        QMessageBox::warning(
            this,
            "Template failed",
            "The selected template could not create an activity."
        );
        return;
    }

    const QString createdActivityId = activity->id();

    auto command = std::make_unique<AddActivityCommand>(
        m_activityManager,
        std::move(activity)
    );

    if (!m_commandHistory.executeCommand(std::move(command))) {
        QMessageBox::warning(
            this,
            "Create activity failed",
            "The activity could not be added."
        );
        return;
    }

    setUnsavedChanges(true);
    synchronizeCategoryManagerFromActivities();
    refreshActivityList();

    for (int row = 0; row < m_activityList->count(); ++row) {
        QListWidgetItem* item = m_activityList->item(row);

        if (item && item->data(Qt::UserRole).toString() == createdActivityId) {
            m_activityList->setCurrentRow(row);
            break;
        }
    }

    updateActionButtons();

    statusBar()->showMessage(
        QString("Activity created from template \"%1\"").arg(selectedTemplateName),
        3000
    );
}

void MainWindow::saveSelectedActivityAsTemplate()
{
    if (!m_activityManager || !m_templateManager) {
        return;
    }

    const QString activityId = selectedActivityId();

    if (activityId.isEmpty()) {
        QMessageBox::information(
            this,
            "No activity selected",
            "Select an activity before saving it as a template."
        );
        return;
    }

    const Activity* activity = findActivityById(activityId);

    if (!activity) {
        QMessageBox::warning(
            this,
            "Invalid activity",
            "The selected activity could not be found."
        );
        return;
    }

    bool ok = false;

    const QString defaultTemplateName =
        QString("%1 template").arg(activity->title());

    const QString templateName = QInputDialog::getText(
        this,
        "Save selected as template",
        "Template name",
        QLineEdit::Normal,
        defaultTemplateName,
        &ok
    ).trimmed();

    if (!ok) {
        return;
    }

    if (templateName.isEmpty()) {
        QMessageBox::warning(
            this,
            "Invalid template",
            "The template name cannot be empty."
        );
        return;
    }

    /*
     * Salvo una copia polimorfa dell'attività selezionata come prototipo.
     * Quando il template verrà usato, il modello creerà una nuova attività
     * con una nuova identità.
     */
    if (!m_templateManager->addTemplate(templateName, activity->clone())) {
        QMessageBox::warning(
            this,
            "Template not saved",
            "The template could not be saved. The name may already be used."
        );
        return;
    }

    setUnsavedChanges(true);
    updateActionButtons();

    statusBar()->showMessage(
        QString("Template \"%1\" saved").arg(templateName),
        3000
    );
}

void MainWindow::undoLastCommand()
{
    if (!m_commandHistory.canUndo()) {
        statusBar()->showMessage("Nothing to undo", 2000);
        updateActionButtons();
        return;
    }

    if (!m_commandHistory.undo()) {
        QMessageBox::warning(this, "Undo failed", "The last operation could not be undone.");
        updateActionButtons();
        return;
    }

    setUnsavedChanges(true);
    refreshActivityList();
    updateActionButtons();

    statusBar()->showMessage("Undo completed", 3000);
}

void MainWindow::redoLastCommand()
{
    if (!m_commandHistory.canRedo()) {
        statusBar()->showMessage("Nothing to redo", 2000);
        updateActionButtons();
        return;
    }

    if (!m_commandHistory.redo()) {
        QMessageBox::warning(this, "Redo failed", "The last undone operation could not be repeated.");
        updateActionButtons();
        return;
    }

    setUnsavedChanges(true);
    refreshActivityList();
    updateActionButtons();

    statusBar()->showMessage("Redo completed", 3000);
}