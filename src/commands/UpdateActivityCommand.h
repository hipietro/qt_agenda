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
 * Conserva due snapshot polimorfi completi: lo stato precedente e quello
 * aggiornato. Undo e redo sostituiscono quindi l'intero oggetto, compresi
 * gli attributi specifici del tipo concreto.
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
    QString undoDescription() const override;
    QString redoDescription() const override;

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
