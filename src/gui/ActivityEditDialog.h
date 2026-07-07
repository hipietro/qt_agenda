#ifndef ACTIVITYEDITDIALOG_H
#define ACTIVITYEDITDIALOG_H

#include <QDialog>

#include <memory>
#include <optional>

#include "model/Activity.h"
#include "model/ActivityKind.h"
#include "model/Priority.h"
#include "model/ChecklistActivity.h"
#include "model/RecurrenceRule.h"

class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTextEdit;
class QGroupBox;
class QWidget;
class CategoryManager;

/*
 * Dialog usato per modificare un'attività esistente.
 *
 * Ho scelto di creare un dialog separato da quello di creazione perché qui
 * devo preservare id, timestamp e tipo concreto dell'attività originale.
 */
class ActivityEditDialog : public QDialog
{
public:
    explicit ActivityEditDialog(const Activity& activity,
                                const CategoryManager* categoryManager = nullptr,
                                QWidget* parent = nullptr);

    /*
     * Restituisce l'attività aggiornata trasferendone la proprietà al chiamante.
     * La MainWindow la userà per sostituire l'oggetto dentro ActivityManager.
     */
    std::unique_ptr<Activity> takeUpdatedActivity();

protected:
    void accept() override;

private:
    void setupUi();
    void populateFromActivity(const Activity& activity);

    bool validateForm() const;
    std::unique_ptr<Activity> createUpdatedActivityFromForm() const;

    Priority selectedPriority() const;
    QString selectedCategoryText() const;
    void populateCategoryCombo(const QString& currentCategory);

    void populateChecklistItems(const Activity& activity);
    QVector<ChecklistItem> checklistItemsFromList() const;

    void populateRecurrence(const Activity& activity);
    void updateRecurrenceControls();

    std::optional<RecurrenceRule> recurrenceRuleFromForm() const;
    RecurrenceRule::Frequency selectedRecurrenceFrequency() const;
    RecurrenceRule::EndMode selectedRecurrenceEndMode() const;

    const CategoryManager* m_categoryManager = nullptr;

    ActivityKind m_activityKind;
    QString m_originalId;
    QDateTime m_originalCreatedAt;
    bool m_originalCompleted = false;

    QLabel* m_typeLabel = nullptr;
    QLineEdit* m_titleEdit = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QComboBox* m_categoryCombo = nullptr;
    QComboBox* m_priorityCombo = nullptr;

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
    QListWidget* m_checklistItemsList = nullptr;
    QLineEdit* m_checklistNewItemEdit = nullptr;
    QPushButton* m_addChecklistItemButton = nullptr;
    QPushButton* m_removeChecklistItemButton = nullptr;

    QDialogButtonBox* m_buttonBox = nullptr;

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

    std::unique_ptr<Activity> m_updatedActivity;
};

#endif