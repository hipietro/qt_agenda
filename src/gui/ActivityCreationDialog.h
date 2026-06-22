#ifndef ACTIVITYCREATIONDIALOG_H
#define ACTIVITYCREATIONDIALOG_H

#include <QDialog>

#include <memory>

#include "model/Activity.h"
#include "model/ActivityKind.h"
#include "model/Priority.h"

class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QDialogButtonBox;
class QLineEdit;
class QSpinBox;
class QStackedWidget;
class QTextEdit;

/*
 * Dialog usato per creare una nuova attività.
 *
 * Ho scelto di separare questa classe dalla MainWindow perché la finestra
 * principale deve coordinare la GUI generale, mentre questo dialog gestisce
 * solo la raccolta e la validazione dei dati necessari alla creazione.
 */
class ActivityCreationDialog : public QDialog
{
public:
    explicit ActivityCreationDialog(QWidget* parent = nullptr);

    /*
     * Restituisce l'attività creata trasferendone la proprietà al chiamante.
     * La MainWindow potrà poi inserirla dentro ActivityManager.
     */
    std::unique_ptr<Activity> takeCreatedActivity();

protected:
    /*
     * Sovrascrivo accept() per validare i campi prima di chiudere il dialog.
     * In questo modo evito di creare attività incomplete o incoerenti.
     */
    void accept() override;

private:
    void setupUi();
    void connectSignals();
    void updateTypePage();

    bool validateForm() const;
    std::unique_ptr<Activity> createActivityFromForm() const;

    Priority selectedPriority() const;
    ActivityKind selectedActivityKind() const;

    QComboBox* m_typeCombo = nullptr;
    QLineEdit* m_titleEdit = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
    QComboBox* m_priorityCombo = nullptr;

    /*
     * Uso uno QStackedWidget per mostrare campi diversi in base al tipo
     * concreto scelto: Event, Deadline, Reminder o Checklist.
     */
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

    std::unique_ptr<Activity> m_createdActivity;
};

#endif