#include "ActivityEditDialog.h"

#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>

ActivityEditDialog::ActivityEditDialog(const Activity& activity, QWidget* parent)
    : QDialog(parent),
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

    if (const std::optional<RecurrenceRule> recurrence = m_updatedActivity->recurrenceRule();
        recurrence.has_value()) {
        m_updatedActivity->setRecurrenceRule(recurrence.value());
    }

    QDialog::accept();
}

void ActivityEditDialog::setupUi()
{
    setWindowTitle("Edit activity");
    resize(720, 620);
    setMinimumSize(680, 560);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);

    QGroupBox* commonGroup = new QGroupBox("Common fields", this);
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

    m_categoryEdit = new QLineEdit(commonGroup);
    m_categoryEdit->setMinimumWidth(300);

    m_priorityCombo = new QComboBox(commonGroup);
    m_priorityCombo->addItem("Low", static_cast<int>(Priority::Low));
    m_priorityCombo->addItem("Medium", static_cast<int>(Priority::Medium));
    m_priorityCombo->addItem("High", static_cast<int>(Priority::High));
    m_priorityCombo->addItem("Critical", static_cast<int>(Priority::Critical));
    m_priorityCombo->setMinimumWidth(180);

    commonLayout->addRow("Type", m_typeLabel);
    commonLayout->addRow("Title", m_titleEdit);
    commonLayout->addRow("Description", m_descriptionEdit);
    commonLayout->addRow("Category", m_categoryEdit);
    commonLayout->addRow("Priority", m_priorityCombo);

    QGroupBox* specificGroup = new QGroupBox("Type-specific fields", this);
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
    m_checklistItemsList->setMinimumHeight(130);
    m_checklistItemsList->setEditTriggers(QAbstractItemView::DoubleClicked |
                                        QAbstractItemView::EditKeyPressed);

    m_checklistNewItemEdit = new QLineEdit(checklistItemsWidget);
    m_checklistNewItemEdit->setPlaceholderText("New checklist item");

    m_addChecklistItemButton = new QPushButton("Add item", checklistItemsWidget);
    m_addChecklistItemButton->setObjectName("primaryButton");

    m_removeChecklistItemButton = new QPushButton("Remove selected", checklistItemsWidget);
    m_removeChecklistItemButton->setObjectName("dangerButton");

    QHBoxLayout* checklistButtonLayout = new QHBoxLayout();
    checklistButtonLayout->addWidget(m_checklistNewItemEdit);
    checklistButtonLayout->addWidget(m_addChecklistItemButton);
    checklistButtonLayout->addWidget(m_removeChecklistItemButton);

    checklistItemsLayout->addWidget(m_checklistItemsList);
    checklistItemsLayout->addLayout(checklistButtonLayout);

    checklistLayout->addRow("Target date", m_checklistDueEdit);
    checklistLayout->addRow("Items", checklistItemsWidget);

    m_typeStack->addWidget(eventPage);
    m_typeStack->addWidget(deadlinePage);
    m_typeStack->addWidget(reminderPage);
    m_typeStack->addWidget(checklistPage);

    specificLayout->addWidget(m_typeStack);

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
            return;
        }

        delete m_checklistItemsList->takeItem(m_checklistItemsList->row(item));
    });

    mainLayout->addWidget(commonGroup);
    mainLayout->addWidget(specificGroup);
    mainLayout->addWidget(m_buttonBox);
}

void ActivityEditDialog::populateFromActivity(const Activity& activity)
{
    m_typeLabel->setText(activityKindToString(activity.kind()));
    m_titleEdit->setText(activity.title());
    m_descriptionEdit->setPlainText(activity.description());
    m_categoryEdit->setText(activity.category());

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

    return true;
}

std::unique_ptr<Activity> ActivityEditDialog::createUpdatedActivityFromForm() const
{
    const QString title = m_titleEdit->text().trimmed();
    const QString description = m_descriptionEdit->toPlainText().trimmed();
    const QString category = m_categoryEdit->text().trimmed();
    const Priority priority = selectedPriority();

    const QDateTime updatedAt = QDateTime::currentDateTime();

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

        return std::make_unique<EventActivity>(
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
    }

    case ActivityKind::Deadline:
        return std::make_unique<DeadlineActivity>(
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

    case ActivityKind::Reminder:
        return std::make_unique<ReminderActivity>(
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

    case ActivityKind::Checklist:
        return std::make_unique<ChecklistActivity>(
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
    }

    return nullptr;
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