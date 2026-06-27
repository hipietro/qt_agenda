#include "MainWindow.h"

#include "model/ActivityFilter.h"
#include "model/ActivityManager.h"
#include "model/SearchEngine.h"
#include "model/ChecklistActivity.h"
#include "persistence/AgendaJsonStorage.h"
#include "ActivityCreationDialog.h"
#include "ActivityEditDialog.h"
#include "model/ActivityTemplate.h"
#include "model/ActivityTemplateManager.h"

#include <QComboBox>
#include <QInputDialog>
#include <QAction>
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
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QDialog>

MainWindow::MainWindow(ActivityManager* activityManager,
                       ActivityTemplateManager* templateManager,
                       QWidget* parent)
    : QMainWindow(parent),
      m_activityManager(activityManager),
      m_templateManager(templateManager)
{
    setupUi();
    setupMenuBar();
    connectSignals();
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

    QHBoxLayout* primaryActionLayout = new QHBoxLayout();
    primaryActionLayout->addWidget(m_addButton);
    primaryActionLayout->addWidget(m_editButton);

    QHBoxLayout* templateActionLayout = new QHBoxLayout();
    templateActionLayout->addWidget(m_templateButton);

    QHBoxLayout* secondaryActionLayout = new QHBoxLayout();
    secondaryActionLayout->addWidget(m_toggleCompletedButton);
    secondaryActionLayout->addWidget(m_deleteButton);

    leftLayout->addWidget(searchLabel);
    leftLayout->addWidget(m_searchEdit);
    leftLayout->addWidget(typeLabel);
    leftLayout->addWidget(m_typeCombo);
    leftLayout->addWidget(m_resultCountLabel);
    leftLayout->addWidget(m_activityList);
    leftLayout->addLayout(primaryActionLayout);
    leftLayout->addLayout(templateActionLayout);
    leftLayout->addLayout(secondaryActionLayout);

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

    /* Se l'attività è una checklist mostro anche i singoli item.
       Ho scelto di farlo qui perché la detail view deve essere utile
       anche prima di implementare il dialog di modifica. */
    if (activity->kind() == ActivityKind::Checklist) {
        const ChecklistActivity& checklist =
            static_cast<const ChecklistActivity&>(*activity);

        details += "\n\nChecklist items:\n";

        if (checklist.items().isEmpty()) {
            details += "- No items\n";
        } else {
            for (const ChecklistItem& item : checklist.items()) {
                const QString marker = item.completed ? "[x]" : "[ ]";
                details += QString("%1 %2\n").arg(marker, item.text);
            }
        }
    }

    m_detailView->setPlainText(details);
}

void MainWindow::updateActionButtons()
{
    if (m_addButton) {
        m_addButton->setEnabled(m_activityManager != nullptr);
    }
    if (m_templateButton) {
        m_templateButton->setEnabled(m_activityManager != nullptr &&
                                    m_templateManager != nullptr &&
                                    !m_templateManager->isEmpty());
    }
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

    if (m_editButton) {
    m_editButton->setEnabled(hasSelection);
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
    setUnsavedChanges(true);

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
    setUnsavedChanges(true);

    refreshActivityList();
}

void MainWindow::saveAgenda()
{
    if (m_currentFilePath.trimmed().isEmpty()) {
        saveAgendaAs();
        return;
    }

    QString errorMessage;

    const bool saved = AgendaJsonStorage::saveToFile(
        *m_activityManager,
        m_currentFilePath,
        &errorMessage
    );

    if (!saved) {
        QMessageBox::warning(this, "Save failed", errorMessage);
        statusBar()->showMessage("Save failed", 3000);
        return;
    }

    setUnsavedChanges(false);
    statusBar()->showMessage(QString("Saved to %1").arg(m_currentFilePath), 3000);
    updateWindowTitle();
}

void MainWindow::saveAgendaAs()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save agenda",
        QString(),
        "Agenda JSON (*.json);;All files (*)"
    );

    if (filePath.trimmed().isEmpty()) {
        return;
    }

    m_currentFilePath = filePath;
    saveAgenda();
}

void MainWindow::loadAgenda()
{
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
        filePath,
        &errorMessage
    );

    if (!loaded) {
        QMessageBox::warning(this, "Load failed", errorMessage);
        statusBar()->showMessage("Load failed", 3000);
        return;
    }

    m_currentFilePath = filePath;
    setUnsavedChanges(false);

    m_searchEdit->clear();
    m_typeCombo->setCurrentIndex(0);

    refreshActivityList();
    updateWindowTitle();

    statusBar()->showMessage(QString("Loaded from %1").arg(m_currentFilePath), 3000);
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
    if (!m_activityManager) {
        return;
    }

    /*
     * Il dialog costruisce l'attività concreta e la restituisce come Activity.
     * Ho scelto questo flusso per tenere la logica di creazione fuori dalla MainWindow.
     */
    ActivityCreationDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    std::unique_ptr<Activity> activity = dialog.takeCreatedActivity();

    if (!activity) {
        return;
    }

    const QString createdActivityId = activity->id();

    if (!m_activityManager->addActivity(std::move(activity))) {
        QMessageBox::warning(this, "Create activity failed", "The activity could not be added.");
        return;
    }

    setUnsavedChanges(true);

    refreshActivityList();

    /*
     * Dopo il refresh provo a selezionare subito l'attività appena creata,
     * così l'utente vede immediatamente il risultato dell'operazione.
     */
    for (int row = 0; row < m_activityList->count(); ++row) {
        QListWidgetItem* item = m_activityList->item(row);

        if (item && item->data(Qt::UserRole).toString() == createdActivityId) {
            m_activityList->setCurrentRow(row);
            break;
        }
    }

    showActivityDetails(findActivityById(createdActivityId));
    updateActionButtons();

    statusBar()->showMessage("Activity created", 3000);
}

void MainWindow::editSelectedActivity()
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

    /*
     * Il dialog modifica una copia logica dell'attività.
     * Quando l'utente conferma, sostituisco l'oggetto nel manager mantenendo lo stesso id.
     */
    ActivityEditDialog dialog(*activity, this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    std::unique_ptr<Activity> updatedActivity = dialog.takeUpdatedActivity();

    if (!updatedActivity) {
        return;
    }

    if (!m_activityManager->replaceActivity(activityId, std::move(updatedActivity))) {
        QMessageBox::warning(this, "Edit activity failed", "The activity could not be updated.");
        return;
    }

    setUnsavedChanges(true);

    refreshActivityList();

    for (int row = 0; row < m_activityList->count(); ++row) {
        QListWidgetItem* item = m_activityList->item(row);

        if (item && item->data(Qt::UserRole).toString() == activityId) {
            m_activityList->setCurrentRow(row);
            break;
        }
    }

    showActivityDetails(findActivityById(activityId));
    updateActionButtons();

    statusBar()->showMessage("Activity updated", 3000);
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

    if (!m_activityManager->addActivity(std::move(activity))) {
        QMessageBox::warning(
            this,
            "Create activity failed",
            "The activity could not be added."
        );
        return;
    }

    setUnsavedChanges(true);
    refreshActivityList();

    for (int row = 0; row < m_activityList->count(); ++row) {
        QListWidgetItem* item = m_activityList->item(row);

        if (item && item->data(Qt::UserRole).toString() == createdActivityId) {
            m_activityList->setCurrentRow(row);
            break;
        }
    }

    showActivityDetails(findActivityById(createdActivityId));
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

    updateActionButtons();

    statusBar()->showMessage(
        QString("Template \"%1\" saved for this session").arg(templateName),
        3000
    );
}