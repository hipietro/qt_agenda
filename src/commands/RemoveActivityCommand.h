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
 * Conserva una copia polimorfa dell'oggetto eliminato, così undo ripristina
 * identità, stato comune e attributi specifici del tipo senza conoscere la
 * classe concreta.
 */
class RemoveActivityCommand : public Command
{
public:
    RemoveActivityCommand(ActivityManager* activityManager,
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

    std::unique_ptr<Activity> m_removedActivityPrototype;

    bool m_hasBeenExecuted = false;
};

#endif
