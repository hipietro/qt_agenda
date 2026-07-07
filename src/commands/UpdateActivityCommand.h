// Undoable command for replacing an existing activity with an edited version.

#ifndef UPDATEACTIVITYCOMMAND_H
#define UPDATEACTIVITYCOMMAND_H

#include "Command.h"

#include <QString>

#include <memory>

class Activity;
class ActivityManager;

/*
 * Command concreta per aggiornare un'attività esistente.
 *
 * Ho scelto di conservare sia lo stato precedente sia quello aggiornato
 * perché undo e redo devono poter sostituire l'attività in entrambe le direzioni.
 */
class UpdateActivityCommand : public Command
{
public:
    UpdateActivityCommand(ActivityManager* activityManager,
                          const QString& activityId,
                          std::unique_ptr<Activity> updatedActivity);

    bool execute() override;
    bool undo() override;
    QString description() const override;

    QString activityId() const;

private:
    ActivityManager* m_activityManager = nullptr;

    QString m_activityId;
    QString m_activityTitle;

    std::unique_ptr<Activity> m_previousActivityPrototype;
    std::unique_ptr<Activity> m_updatedActivityPrototype;

    bool m_hasBeenExecuted = false;
};

#endif