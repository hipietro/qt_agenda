// Undoable command for changing an activity completion state.

#ifndef TOGGLECOMPLETIONCOMMAND_H
#define TOGGLECOMPLETIONCOMMAND_H

#include "Command.h"

#include <QString>

class ActivityManager;

/*
 * Command concreta per cambiare lo stato completato/attivo di un'attività.
 *
 * Lo stato iniziale viene catturato una sola volta. Undo ripristina quel
 * valore esatto e redo applica sempre il valore di destinazione originario.
 */
class ToggleCompletionCommand : public Command
{
public:
    ToggleCompletionCommand(ActivityManager* activityManager,
                            const QString& activityId);

    bool execute() override;
    bool undo() override;
    QString description() const override;
    QString undoDescription() const override;
    QString redoDescription() const override;

    QString activityId() const;

private:
    ActivityManager* m_activityManager = nullptr;

    QString m_activityId;
    QString m_activityTitle;

    bool m_previousCompleted = false;
    bool m_newCompleted = false;
    bool m_stateCaptured = false;
    bool m_hasBeenExecuted = false;
};

#endif
