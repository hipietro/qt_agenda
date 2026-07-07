// Activity creation dialog implementation. It builds the right concrete Activity type from user input.

#include "ActivityCreationDialog.h"

#include "model/Category.h"
#include "model/CategoryManager.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStringList>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>

ActivityCreationDialog::ActivityCreationDialog(const CategoryManager* categoryManager,
                                                   QWidget* parent)
    : QDialog(parent),
      m_categoryManager(categoryManager)
{
    setupUi();
    connectSignals();
    updateTypePage();
}

std::unique_ptr<Activity> ActivityCreationDialog::takeCreatedActivity()
{
    return std::move(m_createdActivity);
}

void ActivityCreationDialog::accept()
{
    if (!validateForm()) {
        return;
    }

    m_createdActivity = createActivityFromForm();

    if (!m_createdActivity) {
        QMessageBox::warning(this, "Invalid activity", "The activity could not be created.");
        return;
    }

    QDialog::accept();
}

void ActivityCreationDialog::setupUi()
{
    setWindowTitle("Create activity");
    resize(720, 620);
    setMinimumSize(680, 560);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);

    QGroupBox* commonGroup = new QGroupBox("Common fields", this);
    QFormLayout* commonLayout = new QFormLayout(commonGroup);

    /* Ho scelto di forzare allineamenti e spaziature del form
       perché il layout automatico era troppo stretto e disallineato. */
    commonLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    commonLayout->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
    commonLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    commonLayout->setHorizontalSpacing(12);
    commonLayout->setVerticalSpacing(9);

    m_typeCombo = new QComboBox(commonGroup);
    m_typeCombo->addItem("Event", static_cast<int>(ActivityKind::Event));
    m_typeCombo->addItem("Deadline", static_cast<int>(ActivityKind::Deadline));
    m_typeCombo->addItem("Reminder", static_cast<int>(ActivityKind::Reminder));
    m_typeCombo->addItem("Checklist", static_cast<int>(ActivityKind::Checklist));
    m_typeCombo->setMinimumWidth(180);
    m_typeCombo->setMinimumContentsLength(12);
    m_typeCombo->view()->setMinimumWidth(180);

    m_titleEdit = new QLineEdit(commonGroup);
    m_titleEdit->setPlaceholderText("Activity title");
    m_titleEdit->setMinimumWidth(300);

    m_descriptionEdit = new QTextEdit(commonGroup);
    m_descriptionEdit->setPlaceholderText("Optional description");
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

    populateCategoryCombo();

    m_priorityCombo = new QComboBox(commonGroup);
    m_priorityCombo->addItem("Low", static_cast<int>(Priority::Low));
    m_priorityCombo->addItem("Medium", static_cast<int>(Priority::Medium));
    m_priorityCombo->addItem("High", static_cast<int>(Priority::High));
    m_priorityCombo->addItem("Critical", static_cast<int>(Priority::Critical));
    m_priorityCombo->setCurrentIndex(1);
    m_priorityCombo->setMinimumWidth(180);
    m_priorityCombo->view()->setMinimumWidth(180);

    commonLayout->addRow("Type", m_typeCombo);
    commonLayout->addRow("Title", m_titleEdit);
    commonLayout->addRow("Description", m_descriptionEdit);
    commonLayout->addRow("Category", m_categoryCombo);
    commonLayout->addRow("Priority", m_priorityCombo);

    QGroupBox* specificGroup = new QGroupBox("Type-specific fields", this);
    QVBoxLayout* specificLayout = new QVBoxLayout(specificGroup);
    specificLayout->setContentsMargins(12, 12, 12, 12);

    m_typeStack = new QStackedWidget(this);
    m_typeStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    const QDateTime now = QDateTime::currentDateTime();

    QWidget* eventPage = new QWidget(m_typeStack);
    QFormLayout* eventLayout = new QFormLayout(eventPage);
    eventLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    eventLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    eventLayout->setHorizontalSpacing(12);
    eventLayout->setVerticalSpacing(9);

    m_eventStartEdit = new QDateTimeEdit(now.addDays(1), eventPage);
    m_eventStartEdit->setCalendarPopup(true);
    m_eventStartEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_eventStartEdit->setMinimumWidth(180);

    m_eventEndEdit = new QDateTimeEdit(now.addDays(1).addSecs(3600), eventPage);
    m_eventEndEdit->setCalendarPopup(true);
    m_eventEndEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_eventEndEdit->setMinimumWidth(180);

    m_eventLocationEdit = new QLineEdit(eventPage);
    m_eventLocationEdit->setPlaceholderText("Optional location");
    m_eventLocationEdit->setMinimumWidth(300);

    m_eventParticipantsEdit = new QLineEdit(eventPage);
    m_eventParticipantsEdit->setPlaceholderText("Comma-separated participants");
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

    m_deadlineDueEdit = new QDateTimeEdit(now.addDays(7), deadlinePage);
    m_deadlineDueEdit->setCalendarPopup(true);
    m_deadlineDueEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_deadlineDueEdit->setMinimumWidth(180);

    m_deadlineContextEdit = new QLineEdit(deadlinePage);
    m_deadlineContextEdit->setPlaceholderText("Optional context");
    m_deadlineContextEdit->setMinimumWidth(300);

    m_deadlineHardCheck = new QCheckBox("Hard deadline", deadlinePage);
    m_deadlineHardCheck->setChecked(true);

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
    m_reminderDateEdit->setMinimumWidth(180);

    m_reminderAdvanceSpin = new QSpinBox(reminderPage);
    m_reminderAdvanceSpin->setRange(0, 10080);
    m_reminderAdvanceSpin->setSuffix(" min");
    m_reminderAdvanceSpin->setMinimumWidth(130);

    m_reminderNoteEdit = new QLineEdit(reminderPage);
    m_reminderNoteEdit->setPlaceholderText("Optional reminder note");
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

    m_checklistDueEdit = new QDateTimeEdit(now.addDays(3), checklistPage);
    m_checklistDueEdit->setCalendarPopup(true);
    m_checklistDueEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_checklistDueEdit->setMinimumWidth(180);

    m_checklistItemsEdit = new QTextEdit(checklistPage);
    m_checklistItemsEdit->setPlaceholderText("One checklist item per line");
    m_checklistItemsEdit->setMinimumWidth(340);
    m_checklistItemsEdit->setFixedHeight(115);

    checklistLayout->addRow("Target date", m_checklistDueEdit);
    checklistLayout->addRow("Items", m_checklistItemsEdit);

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
    m_recurrenceGroup = new QGroupBox("Recurrence", this);
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
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Create");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancel");

    mainLayout->addWidget(commonGroup);
    mainLayout->addWidget(specificGroup);
    mainLayout->addWidget(m_recurrenceGroup);
    mainLayout->addWidget(m_buttonBox);

    updateRecurrenceControls();
}

void ActivityCreationDialog::connectSignals()
{
    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        updateTypePage();
    });

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        accept();
    });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        reject();
    });

    connect(m_repeatsCheck, &QCheckBox::toggled, this, [this]() {
        updateRecurrenceControls();
    });

    connect(m_recurrenceEndModeCombo, &QComboBox::currentIndexChanged, this, [this]() {
        updateRecurrenceControls();
    });
}

void ActivityCreationDialog::updateTypePage()
{
    if (!m_typeStack || !m_typeCombo) {
        return;
    }

    /*
     * L'indice del combo coincide con l'indice della pagina nello stack.
     * Ho scelto questa soluzione perché rende immediata la gestione della GUI.
     */
    m_typeStack->setCurrentIndex(m_typeCombo->currentIndex());
}

bool ActivityCreationDialog::validateForm() const
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Invalid activity", "The title cannot be empty.");
        return false;
    }

    // Eventi senza durata o con fine prima dell'inizio creerebbero dati poco credibili.
    if (selectedActivityKind() == ActivityKind::Event &&
        m_eventEndEdit->dateTime() <= m_eventStartEdit->dateTime()) {
        QMessageBox::warning(nullptr, "Invalid event", "The event end time must be after the start time.");
        return false;
    }

    if (selectedActivityKind() == ActivityKind::Checklist &&
        checklistItemsFromText().isEmpty()) {
        QMessageBox::warning(
            nullptr,
            "Invalid checklist",
            "A checklist must contain at least one item."
        );
        return false;
    }

    if (m_repeatsCheck->isChecked() &&
        selectedRecurrenceEndMode() == RecurrenceRule::EndMode::UntilDate) {
        QDateTime primaryDate;

        switch (selectedActivityKind()) {
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

    return true;
}

std::unique_ptr<Activity> ActivityCreationDialog::createActivityFromForm() const
{
    const QString title = m_titleEdit->text().trimmed();
    const QString description = m_descriptionEdit->toPlainText().trimmed();
    const QString category = selectedCategoryText();
    const Priority priority = selectedPriority();

    std::unique_ptr<Activity> activity;

    switch (selectedActivityKind()) {
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
            priority
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
            priority
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
            priority
        );

        break;

    case ActivityKind::Checklist:
        activity = std::make_unique<ChecklistActivity>(
            title,
            m_checklistDueEdit->dateTime(),
            checklistItemsFromText(),
            description,
            category,
            priority
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

QString ActivityCreationDialog::selectedCategoryText() const
{
    if (!m_categoryCombo) {
        return QString();
    }

    return m_categoryCombo->currentText().trimmed();
}

void ActivityCreationDialog::populateCategoryCombo()
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

    std::sort(categories.begin(), categories.end(), [](const QString& first, const QString& second) {
        return QString::localeAwareCompare(first, second) < 0;
    });

    for (const QString& category : categories) {
        m_categoryCombo->addItem(category);
    }

    m_categoryCombo->setCurrentText(QString());
}

Priority ActivityCreationDialog::selectedPriority() const
{
    return static_cast<Priority>(m_priorityCombo->currentData().toInt());
}

ActivityKind ActivityCreationDialog::selectedActivityKind() const
{
    return static_cast<ActivityKind>(m_typeCombo->currentData().toInt());
}

void ActivityCreationDialog::updateRecurrenceControls()
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

std::optional<RecurrenceRule> ActivityCreationDialog::recurrenceRuleFromForm() const
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

QVector<ChecklistItem> ActivityCreationDialog::checklistItemsFromText() const
{
    QVector<ChecklistItem> items;

    const QStringList lines =
        m_checklistItemsEdit->toPlainText().split("\n", Qt::SkipEmptyParts);

    for (QString line : lines) {
        line = line.trimmed();

        if (line.isEmpty()) {
            continue;
        }

        ChecklistItem item;

        if (line.startsWith("[x]", Qt::CaseInsensitive)) {
            item.completed = true;
            item.text = line.mid(3).trimmed();
        } else if (line.startsWith("[ ]")) {
            item.completed = false;
            item.text = line.mid(3).trimmed();
        } else {
            item.completed = false;
            item.text = line;
        }

        if (!item.text.isEmpty()) {
            items.append(item);
        }
    }

    return items;
}

RecurrenceRule::Frequency ActivityCreationDialog::selectedRecurrenceFrequency() const
{
    return static_cast<RecurrenceRule::Frequency>(
        m_recurrenceFrequencyCombo->currentData().toInt()
    );
}

RecurrenceRule::EndMode ActivityCreationDialog::selectedRecurrenceEndMode() const
{
    return static_cast<RecurrenceRule::EndMode>(
        m_recurrenceEndModeCombo->currentData().toInt()
    );
}
