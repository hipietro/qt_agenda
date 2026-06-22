#ifndef ACTIVITYEDITDIALOG_H
#define ACTIVITYEDITDIALOG_H

#include <QDialog>

#include <memory>

#include "model/Activity.h"
#include "model/ActivityKind.h"
#include "model/Priority.h"
#include "model/ChecklistActivity.h"

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

/*
 * Dialog usato per modificare un'attività esistente.
 *
 * Ho scelto di creare un dialog separato da quello di creazione perché qui
 * devo preservare id, timestamp e tipo concreto dell'attività originale.
 */
class ActivityEditDialog : public QDialog
{
public:
    explicit ActivityEditDialog(const Activity& activity, QWidget* parent = nullptr);

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

    void populateChecklistItems(const Activity& activity);
    QVector<ChecklistItem> checklistItemsFromList() const;

    ActivityKind m_activityKind;
    QString m_originalId;
    QDateTime m_originalCreatedAt;
    bool m_originalCompleted = false;

    QLabel* m_typeLabel = nullptr;
    QLineEdit* m_titleEdit = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
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

    std::unique_ptr<Activity> m_updatedActivity;
};

#endif