// Undoable command for removing an activity.

#ifndef REMOVEACTIVITYCOMMAND_H
#define REMOVEACTIVITYCOMMAND_H

#include "Command.h"

#include <QString>

#include <memory>

class Activity;
class ActivityManager;

/*
 * Command concreta per rimuovere un'attività.
 *
 * Ho scelto di salvare una copia dell'attività rimossa perché l'undo deve
 * poterla reinserire nel manager mantenendo i suoi dati originali.
 */
class RemoveActivityCommand : public Command
{
public:
    RemoveActivityCommand(ActivityManager* activityManager,
                          const QString& activityId);

    bool execute() override;
    bool undo() override;
    QString description() const override;

    QString activityId() const;

private:
    ActivityManager* m_activityManager = nullptr;

    QString m_activityId;
    QString m_activityTitle;

    std::unique_ptr<Activity> m_removedActivityPrototype;

    bool m_hasBeenExecuted = false;
};

#endif