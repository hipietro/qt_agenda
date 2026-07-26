// In-window page used to edit an existing activity without changing its concrete type.

#ifndef ACTIVITYEDITPAGE_H
#define ACTIVITYEDITPAGE_H

#include <QWidget>

#include <functional>
#include <memory>
#include <optional>

#include "model/Activity.h"
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
class ActivityEditFormVisitor;

/*
 * Reusable edit page hosted directly inside MainWindow.
 *
 * The page owns form state and validation. MainWindow remains responsible for
 * executing UpdateActivityCommand, refreshing the agenda, and navigation.
 */
class ActivityEditPage : public QWidget
{
    friend class ActivityEditFormVisitor;
public:
    using SavedHandler = std::function<bool(const QString&, std::unique_ptr<Activity>)>;
    using CancelHandler = std::function<void()>;

    explicit ActivityEditPage(const CategoryManager* categoryManager = nullptr,
                              QWidget* parent = nullptr);

    void setSavedHandler(SavedHandler handler);
    void setCancelHandler(CancelHandler handler);
    void editActivity(const Activity& activity);
    void clearActivity();

private:
    void setupUi();
    void populateFromActivity(const Activity& activity);
    void submit();

    bool validateForm() const;
    std::unique_ptr<Activity> createUpdatedActivityFromForm() const;

    Priority selectedPriority() const;
    QString selectedCategoryText() const;
    void populateCategoryCombo(const QString& currentCategory);

    void populateChecklistItems(const ChecklistActivity& activity);
    QVector<ChecklistItem> checklistItemsFromList() const;

    void populateRecurrence(const Activity& activity);
    void updateRecurrenceControls();

    std::optional<RecurrenceRule> recurrenceRuleFromForm() const;
    RecurrenceRule::Frequency selectedRecurrenceFrequency() const;
    RecurrenceRule::EndMode selectedRecurrenceEndMode() const;

    const CategoryManager* m_categoryManager = nullptr;

    const Activity* m_originalActivity = nullptr;
    QString m_originalId;
    QDateTime m_originalCreatedAt;
    bool m_originalCompleted = false;

    QLabel* m_contextLabel = nullptr;
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

    SavedHandler m_savedHandler;
    CancelHandler m_cancelHandler;
};

#endif
