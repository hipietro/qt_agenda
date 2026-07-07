#include "ActivityEditDialog.h"

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"
#include "model/Category.h"
#include "model/CategoryManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStringList>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGridLayout>


#include <algorithm>

ActivityEditDialog::ActivityEditDialog(const Activity& activity,
                                       const CategoryManager* categoryManager,
                                       QWidget* parent)
    : QDialog(parent),
      m_categoryManager(categoryManager),
      m_activityKind(activity.kind()),
      m_originalId(activity.id()),
      m_originalCreatedAt(activity.createdAt()),
      m_originalCompleted(activity.isCompleted())
{
    setupUi();
    populateFromActivity(activity);
}

std::unique_ptr<Activity> ActivityEditDialog::takeUpdatedActivity()
{
    return std::move(m_updatedActivity);
}

void ActivityEditDialog::accept()
{
    if (!validateForm()) {
        return;
    }

    m_updatedActivity = createUpdatedActivityFromForm();

    if (!m_updatedActivity) {
        QMessageBox::warning(this, "Invalid activity", "The activity could not be updated.");
        return;
    }

    QDialog::accept();
}

void ActivityEditDialog::setupUi()
{
    setWindowTitle("Edit activity");
    resize(760, 640);
    setMinimumSize(680, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);

    /*
     * Tengo i pulsanti finali sempre visibili e rendo scrollabile il contenuto.
     * In questo modo le checklist lunghe non comprimono i campi e non si
     * sovrappongono ai pulsanti di aggiunta/rimozione.
     */
    QScrollArea* contentScrollArea = new QScrollArea(this);
    contentScrollArea->setWidgetResizable(true);
    contentScrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* contentWidget = new QWidget(contentScrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);

    QGroupBox* commonGroup = new QGroupBox("Common fields", contentWidget);
    QFormLayout* commonLayout = new QFormLayout(commonGroup);
    commonLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    commonLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    commonLayout->setHorizontalSpacing(12);
    commonLayout->setVerticalSpacing(9);

    /*
     * In modifica mostro il tipo ma non lo rendo modificabile.
     * Ho scelto così perché cambiare tipo richiederebbe convertire campi diversi
     * e rischierebbe di perdere dati specifici dell'attività originale.
     */
    m_typeLabel = new QLabel(commonGroup);

    m_titleEdit = new QLineEdit(commonGroup);
    m_titleEdit->setMinimumWidth(300);

    m_descriptionEdit = new QTextEdit(commonGroup);
    m_descriptionEdit->setMinimumWidth(340);
    m_descriptionEdit->setFixedHeight(82);

    m_categoryCombo = new QComboBox(commonGroup);
    m_categoryCombo->setEditable(true);
    m_categoryCombo->setInsertPolicy(QComboBox::NoInsert);
    m_categoryCombo->setMinimumWidth(300);
    m_categoryCombo->setMinimumContentsLength(18);

    if (m_categoryCombo->lineEdit()) {
        m_categoryCombo->lineEdit()->setPlaceholderText("Optional category");
    }

    m_priorityCombo = new QComboBox(commonGroup);
    m_priorityCombo->addItem("Low", static_cast<int>(Priority::Low));
    m_priorityCombo->addItem("Medium", static_cast<int>(Priority::Medium));
    m_priorityCombo->addItem("High", static_cast<int>(Priority::High));
    m_priorityCombo->addItem("Critical", static_cast<int>(Priority::Critical));
    m_priorityCombo->setMinimumWidth(180);

    commonLayout->addRow("Type", m_typeLabel);
    commonLayout->addRow("Title", m_titleEdit);
    commonLayout->addRow("Description", m_descriptionEdit);
    commonLayout->addRow("Category", m_categoryCombo);
    commonLayout->addRow("Priority", m_priorityCombo);

    QGroupBox* specificGroup = new QGroupBox("Type-specific fields", contentWidget);
    QVBoxLayout* specificLayout = new QVBoxLayout(specificGroup);
    specificLayout->setContentsMargins(12, 12, 12, 12);

    m_typeStack = new QStackedWidget(this);

    const QDateTime now = QDateTime::currentDateTime();

    QWidget* eventPage = new QWidget(m_typeStack);
    QFormLayout* eventLayout = new QFormLayout(eventPage);
    eventLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    eventLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    eventLayout->setHorizontalSpacing(12);
    eventLayout->setVerticalSpacing(9);

    m_eventStartEdit = new QDateTimeEdit(now, eventPage);
    m_eventStartEdit->setCalendarPopup(true);
    m_eventStartEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_eventStartEdit->setMinimumWidth(220);

    m_eventEndEdit = new QDateTimeEdit(now.addSecs(3600), eventPage);
    m_eventEndEdit->setCalendarPopup(true);
    m_eventEndEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_eventEndEdit->setMinimumWidth(220);

    m_eventLocationEdit = new QLineEdit(eventPage);
    m_eventLocationEdit->setMinimumWidth(300);

    m_eventParticipantsEdit = new QLineEdit(eventPage);
    m_eventParticipantsEdit->setMinimumWidth(300);

    eventLayout->addRow("Start", m_eventStartEdit);
    eventLayout->addRow("End", m_eventEndEdit);
    eventLayout->addRow("Location", m_eventLocationEdit);
    eventLayout->addRow("Participants", m_eventParticipantsEdit);

    QWidget* deadlinePage = new QWidget(m_typeStack);
    QFormLayout* deadlineLayout = new QFormLayout(deadlinePage);
    deadlineLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    deadlineLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    deadlineLayout->setHorizontalSpacing(12);
    deadlineLayout->setVerticalSpacing(9);

    m_deadlineDueEdit = new QDateTimeEdit(now.addDays(1), deadlinePage);
    m_deadlineDueEdit->setCalendarPopup(true);
    m_deadlineDueEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_deadlineDueEdit->setMinimumWidth(220);

    m_deadlineContextEdit = new QLineEdit(deadlinePage);
    m_deadlineContextEdit->setMinimumWidth(300);

    m_deadlineHardCheck = new QCheckBox("Hard deadline", deadlinePage);

    deadlineLayout->addRow("Due date", m_deadlineDueEdit);
    deadlineLayout->addRow("Context", m_deadlineContextEdit);
    deadlineLayout->addRow("", m_deadlineHardCheck);

    QWidget* reminderPage = new QWidget(m_typeStack);
    QFormLayout* reminderLayout = new QFormLayout(reminderPage);
    reminderLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    reminderLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    reminderLayout->setHorizontalSpacing(12);
    reminderLayout->setVerticalSpacing(9);

    m_reminderDateEdit = new QDateTimeEdit(now.addDays(1), reminderPage);
    m_reminderDateEdit->setCalendarPopup(true);
    m_reminderDateEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_reminderDateEdit->setMinimumWidth(220);

    m_reminderAdvanceSpin = new QSpinBox(reminderPage);
    m_reminderAdvanceSpin->setRange(0, 10080);
    m_reminderAdvanceSpin->setSuffix(" min");
    m_reminderAdvanceSpin->setMinimumWidth(130);

    m_reminderNoteEdit = new QLineEdit(reminderPage);
    m_reminderNoteEdit->setMinimumWidth(300);

    reminderLayout->addRow("Reminder time", m_reminderDateEdit);
    reminderLayout->addRow("Advance", m_reminderAdvanceSpin);
    reminderLayout->addRow("Note", m_reminderNoteEdit);

    QWidget* checklistPage = new QWidget(m_typeStack);
    QFormLayout* checklistLayout = new QFormLayout(checklistPage);
    checklistLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    checklistLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    checklistLayout->setHorizontalSpacing(12);
    checklistLayout->setVerticalSpacing(9);

    m_checklistDueEdit = new QDateTimeEdit(now.addDays(1), checklistPage);
    m_checklistDueEdit->setCalendarPopup(true);
    m_checklistDueEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_checklistDueEdit->setMinimumWidth(220);

    /*
     * Per la modifica della checklist uso una QListWidget con checkbox.
     * Ho scelto questa soluzione perché è più sicura del formato testuale [x]/[ ],
     * evita errori di scrittura e rende immediato lo stato completato/non completato.
     */
    QWidget* checklistItemsWidget = new QWidget(checklistPage);
    QVBoxLayout* checklistItemsLayout = new QVBoxLayout(checklistItemsWidget);
    checklistItemsLayout->setContentsMargins(0, 0, 0, 0);
    checklistItemsLayout->setSpacing(8);

    m_checklistItemsList = new QListWidget(checklistItemsWidget);
    m_checklistItemsList->setMinimumWidth(340);
    m_checklistItemsList->setMinimumHeight(170);
    m_checklistItemsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_checklistItemsList->setEditTriggers(QAbstractItemView::DoubleClicked |
                                        QAbstractItemView::EditKeyPressed);

    m_checklistNewItemEdit = new QLineEdit(checklistItemsWidget);
    m_checklistNewItemEdit->setPlaceholderText("New checklist item");

    m_addChecklistItemButton = new QPushButton("Add item", checklistItemsWidget);
    m_addChecklistItemButton->setObjectName("primaryButton");

    m_removeChecklistItemButton = new QPushButton("Remove selected", checklistItemsWidget);
    m_removeChecklistItemButton->setObjectName("dangerButton");

    QHBoxLayout* checklistButtonLayout = new QHBoxLayout();
    checklistButtonLayout->setSpacing(8);
    checklistButtonLayout->addStretch(1);
    checklistButtonLayout->addWidget(m_addChecklistItemButton);
    checklistButtonLayout->addWidget(m_removeChecklistItemButton);

    checklistItemsLayout->addWidget(m_checklistItemsList);
    checklistItemsLayout->addWidget(m_checklistNewItemEdit);
    checklistItemsLayout->addLayout(checklistButtonLayout);

    checklistLayout->addRow("Target date", m_checklistDueEdit);
    checklistLayout->addRow("Items", checklistItemsWidget);

    m_typeStack->addWidget(eventPage);
    m_typeStack->addWidget(deadlinePage);
    m_typeStack->addWidget(reminderPage);
    m_typeStack->addWidget(checklistPage);

    specificLayout->addWidget(m_typeStack);

    /*
    * La ricorrenza resta compatta e mostra solo i campi necessari.
    * Ho scelto questa struttura perché "Repeat every 2 week(s)" è più chiaro
    * di un campo generico chiamato "Interval".
    */
    m_recurrenceGroup = new QGroupBox("Recurrence", contentWidget);
    QVBoxLayout* recurrenceLayout = new QVBoxLayout(m_recurrenceGroup);
    recurrenceLayout->setContentsMargins(12, 12, 12, 12);
    recurrenceLayout->setSpacing(8);

    m_repeatsCheck = new QCheckBox("Repeats", m_recurrenceGroup);
    recurrenceLayout->addWidget(m_repeatsCheck);

    m_recurrenceOptionsWidget = new QWidget(m_recurrenceGroup);
    QGridLayout* recurrenceGrid = new QGridLayout(m_recurrenceOptionsWidget);
    recurrenceGrid->setContentsMargins(0, 0, 0, 0);
    recurrenceGrid->setHorizontalSpacing(10);
    recurrenceGrid->setVerticalSpacing(8);

    QLabel* repeatEveryLabel = new QLabel("Repeat every", m_recurrenceOptionsWidget);

    QWidget* repeatEveryWidget = new QWidget(m_recurrenceOptionsWidget);
    QHBoxLayout* repeatEveryLayout = new QHBoxLayout(repeatEveryWidget);
    repeatEveryLayout->setContentsMargins(0, 0, 0, 0);
    repeatEveryLayout->setSpacing(8);

    m_recurrenceIntervalSpin = new QSpinBox(repeatEveryWidget);
    m_recurrenceIntervalSpin->setRange(1, 365);
    m_recurrenceIntervalSpin->setValue(1);
    m_recurrenceIntervalSpin->setMinimumWidth(80);

    m_recurrenceFrequencyCombo = new QComboBox(repeatEveryWidget);
    m_recurrenceFrequencyCombo->addItem("day(s)", static_cast<int>(RecurrenceRule::Frequency::Daily));
    m_recurrenceFrequencyCombo->addItem("week(s)", static_cast<int>(RecurrenceRule::Frequency::Weekly));
    m_recurrenceFrequencyCombo->addItem("month(s)", static_cast<int>(RecurrenceRule::Frequency::Monthly));
    m_recurrenceFrequencyCombo->addItem("year(s)", static_cast<int>(RecurrenceRule::Frequency::Yearly));
    m_recurrenceFrequencyCombo->setMinimumWidth(140);

    repeatEveryLayout->addWidget(m_recurrenceIntervalSpin);
    repeatEveryLayout->addWidget(m_recurrenceFrequencyCombo, 1);

    QLabel* endConditionLabel = new QLabel("End condition", m_recurrenceOptionsWidget);

    m_recurrenceEndModeCombo = new QComboBox(m_recurrenceOptionsWidget);
    m_recurrenceEndModeCombo->addItem("Never", static_cast<int>(RecurrenceRule::EndMode::Never));
    m_recurrenceEndModeCombo->addItem("Until date", static_cast<int>(RecurrenceRule::EndMode::UntilDate));
    m_recurrenceEndModeCombo->addItem("After occurrences", static_cast<int>(RecurrenceRule::EndMode::AfterOccurrences));
    m_recurrenceEndModeCombo->setMinimumWidth(180);

    m_recurrenceEndDetailsWidget = new QWidget(m_recurrenceOptionsWidget);
    QHBoxLayout* endDetailsLayout = new QHBoxLayout(m_recurrenceEndDetailsWidget);
    endDetailsLayout->setContentsMargins(0, 0, 0, 0);
    endDetailsLayout->setSpacing(8);

    m_recurrenceEndDetailsLabel = new QLabel(m_recurrenceEndDetailsWidget);

    m_recurrenceUntilEdit = new QDateTimeEdit(QDateTime::currentDateTime().addMonths(1),
                                            m_recurrenceEndDetailsWidget);
    m_recurrenceUntilEdit->setCalendarPopup(true);
    m_recurrenceUntilEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_recurrenceUntilEdit->setMinimumWidth(200);

    m_recurrenceOccurrencesSpin = new QSpinBox(m_recurrenceEndDetailsWidget);
    m_recurrenceOccurrencesSpin->setRange(1, 999);
    m_recurrenceOccurrencesSpin->setValue(5);
    m_recurrenceOccurrencesSpin->setMinimumWidth(100);

    endDetailsLayout->addWidget(m_recurrenceEndDetailsLabel);
    endDetailsLayout->addWidget(m_recurrenceUntilEdit, 1);
    endDetailsLayout->addWidget(m_recurrenceOccurrencesSpin, 1);

    recurrenceGrid->addWidget(repeatEveryLabel, 0, 0);
    recurrenceGrid->addWidget(repeatEveryWidget, 0, 1);
    recurrenceGrid->addWidget(endConditionLabel, 1, 0);
    recurrenceGrid->addWidget(m_recurrenceEndModeCombo, 1, 1);
    recurrenceGrid->addWidget(m_recurrenceEndDetailsWidget, 2, 0, 1, 2);

    recurrenceGrid->setColumnStretch(1, 1);

    recurrenceLayout->addWidget(m_recurrenceOptionsWidget);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Save changes");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancel");

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        accept();
    });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        reject();
    });

    connect(m_addChecklistItemButton, &QPushButton::clicked, this, [this]() {
        const QString text = m_checklistNewItemEdit->text().trimmed();

        if (text.isEmpty()) {
            return;
        }

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
        item->setCheckState(Qt::Unchecked);

        m_checklistItemsList->addItem(item);
        m_checklistNewItemEdit->clear();
    });

    connect(m_removeChecklistItemButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = m_checklistItemsList->currentItem();

        if (!item) {
            QMessageBox::information(
                this,
                "No item selected",
                "Select a checklist item before removing it."
            );
            return;
        }

        if (m_checklistItemsList->count() <= 1) {
            QMessageBox::warning(
                this,
                "Invalid checklist",
                "A checklist must contain at least one item."
            );
            return;
        }

        delete m_checklistItemsList->takeItem(m_checklistItemsList->row(item));
    });

    connect(m_repeatsCheck, &QCheckBox::toggled, this, [this]() {
        updateRecurrenceControls();
    });

    connect(m_recurrenceEndModeCombo, &QComboBox::currentIndexChanged, this, [this]() {
        updateRecurrenceControls();
    });

    contentLayout->addWidget(commonGroup);
    contentLayout->addWidget(specificGroup);
    contentLayout->addWidget(m_recurrenceGroup);
    contentLayout->addStretch(1);

    contentScrollArea->setWidget(contentWidget);

    mainLayout->addWidget(contentScrollArea, 1);
    mainLayout->addWidget(m_buttonBox);

    updateRecurrenceControls();
}

void ActivityEditDialog::populateFromActivity(const Activity& activity)
{
    m_typeLabel->setText(activityKindToString(activity.kind()));
    m_titleEdit->setText(activity.title());
    m_descriptionEdit->setPlainText(activity.description());
    populateCategoryCombo(activity.category());

    const int priorityIndex = m_priorityCombo->findData(static_cast<int>(activity.priority()));
    m_priorityCombo->setCurrentIndex(priorityIndex >= 0 ? priorityIndex : 1);

    switch (activity.kind()) {
    case ActivityKind::Event: {
        m_typeStack->setCurrentIndex(0);

        const EventActivity& event = static_cast<const EventActivity&>(activity);
        m_eventStartEdit->setDateTime(event.startDateTime());
        m_eventEndEdit->setDateTime(event.endDateTime());
        m_eventLocationEdit->setText(event.location());
        m_eventParticipantsEdit->setText(event.participants().join(", "));
        break;
    }

    case ActivityKind::Deadline: {
        m_typeStack->setCurrentIndex(1);

        const DeadlineActivity& deadline = static_cast<const DeadlineActivity&>(activity);
        m_deadlineDueEdit->setDateTime(deadline.dueDate());
        m_deadlineContextEdit->setText(deadline.context());
        m_deadlineHardCheck->setChecked(deadline.isHardDeadline());
        break;
    }

    case ActivityKind::Reminder: {
        m_typeStack->setCurrentIndex(2);

        const ReminderActivity& reminder = static_cast<const ReminderActivity&>(activity);
        m_reminderDateEdit->setDateTime(reminder.reminderDateTime());
        m_reminderAdvanceSpin->setValue(reminder.advanceMinutes());
        m_reminderNoteEdit->setText(reminder.reminderNote());
        break;
    }

    case ActivityKind::Checklist:
        m_typeStack->setCurrentIndex(3);
        m_checklistDueEdit->setDateTime(activity.primaryDate());
        populateChecklistItems(activity);
        break;
    }
    populateRecurrence(activity);
}

bool ActivityEditDialog::validateForm() const
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Invalid activity", "The title cannot be empty.");
        return false;
    }

    if (m_activityKind == ActivityKind::Event &&
        m_eventEndEdit->dateTime() <= m_eventStartEdit->dateTime()) {
        QMessageBox::warning(nullptr, "Invalid event", "The event end time must be after the start time.");
        return false;
    }

    if (m_activityKind == ActivityKind::Checklist &&
        checklistItemsFromList().isEmpty()) {
        QMessageBox::warning(
            nullptr,
            "Invalid checklist",
            "A checklist must contain at least one item."
        );

if (m_repeatsCheck->isChecked() &&
    selectedRecurrenceEndMode() == RecurrenceRule::EndMode::UntilDate) {
    QDateTime primaryDate;

    switch (m_activityKind) {
    case ActivityKind::Event:
        primaryDate = m_eventStartEdit->dateTime();
        break;

    case ActivityKind::Deadline:
        primaryDate = m_deadlineDueEdit->dateTime();
        break;

    case ActivityKind::Reminder:
        primaryDate = m_reminderDateEdit->dateTime();
        break;

    case ActivityKind::Checklist:
        primaryDate = m_checklistDueEdit->dateTime();
        break;
    }

    if (m_recurrenceUntilEdit->dateTime() <= primaryDate) {
        QMessageBox::warning(
            nullptr,
            "Invalid recurrence",
            "The recurrence end date must be after the activity date."
        );
        return false;
    }
    }
}

    return true;
}

std::unique_ptr<Activity> ActivityEditDialog::createUpdatedActivityFromForm() const
{
    const QString title = m_titleEdit->text().trimmed();
    const QString description = m_descriptionEdit->toPlainText().trimmed();
    const QString category = selectedCategoryText();
    const Priority priority = selectedPriority();

    const QDateTime updatedAt = QDateTime::currentDateTime();

    std::unique_ptr<Activity> activity;

    /*
     * Ricostruisco una nuova istanza concreta mantenendo id e createdAt originali.
     * Ho scelto questa strada per evitare setter specifici su ogni sottoclasse
     * e per usare ActivityManager::replaceActivity(...) in modo pulito.
     */
    switch (m_activityKind) {
    case ActivityKind::Event: {
        QStringList participants;

        const QStringList rawParticipants =
            m_eventParticipantsEdit->text().split(",", Qt::SkipEmptyParts);

        for (const QString& participant : rawParticipants) {
            const QString trimmedParticipant = participant.trimmed();

            if (!trimmedParticipant.isEmpty()) {
                participants.append(trimmedParticipant);
            }
        }

        activity = std::make_unique<EventActivity>(
            title,
            m_eventStartEdit->dateTime(),
            m_eventEndEdit->dateTime(),
            m_eventLocationEdit->text().trimmed(),
            participants,
            description,
            category,
            priority,
            m_originalCompleted,
            m_originalId,
            m_originalCreatedAt,
            updatedAt
        );

        break;
    }

    case ActivityKind::Deadline:
        activity = std::make_unique<DeadlineActivity>(
            title,
            m_deadlineDueEdit->dateTime(),
            m_deadlineContextEdit->text().trimmed(),
            m_deadlineHardCheck->isChecked(),
            description,
            category,
            priority,
            m_originalCompleted,
            m_originalId,
            m_originalCreatedAt,
            updatedAt
        );

        break;

    case ActivityKind::Reminder:
        activity = std::make_unique<ReminderActivity>(
            title,
            m_reminderDateEdit->dateTime(),
            m_reminderAdvanceSpin->value(),
            m_reminderNoteEdit->text().trimmed(),
            description,
            category,
            priority,
            m_originalCompleted,
            m_originalId,
            m_originalCreatedAt,
            updatedAt
        );

        break;

    case ActivityKind::Checklist:
        activity = std::make_unique<ChecklistActivity>(
            title,
            m_checklistDueEdit->dateTime(),
            checklistItemsFromList(),
            description,
            category,
            priority,
            m_originalCompleted,
            m_originalId,
            m_originalCreatedAt,
            updatedAt
        );

        break;
    }

    if (!activity) {
        return nullptr;
    }

    if (const std::optional<RecurrenceRule> recurrence = recurrenceRuleFromForm();
        recurrence.has_value()) {
        activity->setRecurrenceRule(recurrence.value());
    }

    return activity;
}

QString ActivityEditDialog::selectedCategoryText() const
{
    if (!m_categoryCombo) {
        return QString();
    }

    return m_categoryCombo->currentText().trimmed();
}

void ActivityEditDialog::populateCategoryCombo(const QString& currentCategory)
{
    if (!m_categoryCombo) {
        return;
    }

    m_categoryCombo->clear();

    QStringList categories;

    if (m_categoryManager) {
        for (const Category& category : m_categoryManager->categories()) {
            const QString name = category.name().trimmed();

            if (!name.isEmpty() && !categories.contains(name, Qt::CaseInsensitive)) {
                categories.append(name);
            }
        }
    }

    const QString trimmedCurrent = currentCategory.trimmed();

    if (!trimmedCurrent.isEmpty() && !categories.contains(trimmedCurrent, Qt::CaseInsensitive)) {
        categories.append(trimmedCurrent);
    }

    std::sort(categories.begin(), categories.end(), [](const QString& first, const QString& second) {
        return QString::localeAwareCompare(first, second) < 0;
    });

    for (const QString& category : categories) {
        m_categoryCombo->addItem(category);
    }

    m_categoryCombo->setCurrentText(trimmedCurrent);
}

Priority ActivityEditDialog::selectedPriority() const
{
    return static_cast<Priority>(m_priorityCombo->currentData().toInt());
}

void ActivityEditDialog::populateChecklistItems(const Activity& activity)
{
    const ChecklistActivity& checklist = static_cast<const ChecklistActivity&>(activity);

    m_checklistItemsList->clear();

    /*
     * Ogni ChecklistItem del modello diventa una riga selezionabile.
     * Lo stato completed viene rappresentato direttamente dalla checkbox.
     */
    for (const ChecklistItem& checklistItem : checklist.items()) {
        QListWidgetItem* item = new QListWidgetItem(checklistItem.text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
        item->setCheckState(checklistItem.completed ? Qt::Checked : Qt::Unchecked);

        m_checklistItemsList->addItem(item);
    }
}

QVector<ChecklistItem> ActivityEditDialog::checklistItemsFromList() const
{
    QVector<ChecklistItem> items;

    /*
     * Converto le righe della QListWidget nel formato del modello.
     * In questo modo la GUI resta pratica, mentre ChecklistActivity continua
     * a conservare solo dati semplici: testo e stato completato.
     */
    for (int row = 0; row < m_checklistItemsList->count(); ++row) {
        const QListWidgetItem* listItem = m_checklistItemsList->item(row);

        if (!listItem) {
            continue;
        }

        const QString text = listItem->text().trimmed();

        if (text.isEmpty()) {
            continue;
        }

        ChecklistItem checklistItem;
        checklistItem.text = text;
        checklistItem.completed = listItem->checkState() == Qt::Checked;

        items.append(checklistItem);
    }

    return items;
}

void ActivityEditDialog::populateRecurrence(const Activity& activity)
{
    const std::optional<RecurrenceRule> recurrence = activity.recurrenceRule();

    if (!recurrence.has_value()) {
        m_repeatsCheck->setChecked(false);
        updateRecurrenceControls();
        return;
    }

    m_repeatsCheck->setChecked(true);

    const int frequencyIndex =
        m_recurrenceFrequencyCombo->findData(static_cast<int>(recurrence->frequency()));

    m_recurrenceFrequencyCombo->setCurrentIndex(frequencyIndex >= 0 ? frequencyIndex : 1);

    m_recurrenceIntervalSpin->setValue(recurrence->interval());

    const int endModeIndex =
        m_recurrenceEndModeCombo->findData(static_cast<int>(recurrence->endMode()));

    m_recurrenceEndModeCombo->setCurrentIndex(endModeIndex >= 0 ? endModeIndex : 2);

    if (recurrence->untilDate().isValid()) {
        m_recurrenceUntilEdit->setDateTime(recurrence->untilDate());
    }

    if (recurrence->maxOccurrences() > 0) {
        m_recurrenceOccurrencesSpin->setValue(recurrence->maxOccurrences());
    }

    updateRecurrenceControls();
}

void ActivityEditDialog::updateRecurrenceControls()
{
    const bool recurrenceEnabled = m_repeatsCheck->isChecked();

    m_recurrenceOptionsWidget->setVisible(recurrenceEnabled);

    if (!recurrenceEnabled) {
        m_recurrenceEndDetailsWidget->setVisible(false);
        adjustSize();
        return;
    }

    const RecurrenceRule::EndMode endMode = selectedRecurrenceEndMode();

    const bool usesUntilDate = endMode == RecurrenceRule::EndMode::UntilDate;
    const bool usesOccurrences = endMode == RecurrenceRule::EndMode::AfterOccurrences;

    m_recurrenceEndDetailsWidget->setVisible(usesUntilDate || usesOccurrences);

    m_recurrenceEndDetailsLabel->setText(usesUntilDate ? "Until" : "Occurrences");
    m_recurrenceUntilEdit->setVisible(usesUntilDate);
    m_recurrenceOccurrencesSpin->setVisible(usesOccurrences);

    adjustSize();
}

std::optional<RecurrenceRule> ActivityEditDialog::recurrenceRuleFromForm() const
{
    if (!m_repeatsCheck->isChecked()) {
        return std::nullopt;
    }

    const RecurrenceRule::EndMode endMode = selectedRecurrenceEndMode();

    const QDateTime untilDate =
        endMode == RecurrenceRule::EndMode::UntilDate
            ? m_recurrenceUntilEdit->dateTime()
            : QDateTime();

    const int maxOccurrences =
        endMode == RecurrenceRule::EndMode::AfterOccurrences
            ? m_recurrenceOccurrencesSpin->value()
            : 1;

    RecurrenceRule recurrenceRule(
        selectedRecurrenceFrequency(),
        m_recurrenceIntervalSpin->value(),
        endMode,
        untilDate,
        maxOccurrences
    );

    if (!recurrenceRule.isValid()) {
        return std::nullopt;
    }

    return recurrenceRule;
}

RecurrenceRule::Frequency ActivityEditDialog::selectedRecurrenceFrequency() const
{
    return static_cast<RecurrenceRule::Frequency>(
        m_recurrenceFrequencyCombo->currentData().toInt()
    );
}

RecurrenceRule::EndMode ActivityEditDialog::selectedRecurrenceEndMode() const
{
    return static_cast<RecurrenceRule::EndMode>(
        m_recurrenceEndModeCombo->currentData().toInt()
    );
}
