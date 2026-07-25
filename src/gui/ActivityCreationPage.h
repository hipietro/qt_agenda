// In-window page used to collect data for a new activity.

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
