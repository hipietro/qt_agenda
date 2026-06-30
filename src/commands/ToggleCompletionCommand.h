#ifndef TOGGLECOMPLETIONCOMMAND_H
#define TOGGLECOMPLETIONCOMMAND_H

#include "Command.h"

#include <QString>

class ActivityManager;

/*
 * Command concreta per cambiare lo stato completato/attivo di un'attività.
 *
 * Ho scelto di salvare lo stato precedente e quello nuovo perché l'undo deve
 * ripristinare esattamente la situazione iniziale, non limitarsi a fare toggle.
 */
class ToggleCompletionCommand : public Command
{
public:
    ToggleCompletionCommand(ActivityManager* activityManager,
                            const QString& activityId);

    bool execute() override;
    bool undo() override;
    QString description() const override;

    QString activityId() const;

private:
    ActivityManager* m_activityManager = nullptr;

    QString m_activityId;
    QString m_activityTitle;

    bool m_previousCompleted = false;
    bool m_newCompleted = false;
    bool m_hasBeenExecuted = false;
};

#endif