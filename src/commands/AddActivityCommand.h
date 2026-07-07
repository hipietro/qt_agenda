// Undoable command for adding an activity.

#ifndef ADDACTIVITYCOMMAND_H
#define ADDACTIVITYCOMMAND_H

#include "Command.h"

#include <QString>

#include <memory>

class Activity;
class ActivityManager;

/*
 * Command concreta per aggiungere un'attività.
 *
 * Ho scelto di conservare un prototipo dell'attività perché la command deve
 * poter rieseguire l'aggiunta anche dopo un undo, senza dipendere dalla GUI.
 */
class AddActivityCommand : public Command
{
public:
    AddActivityCommand(ActivityManager* activityManager,
                       std::unique_ptr<Activity> activity);

    bool execute() override;
    bool undo() override;
    QString description() const override;

    QString activityId() const;

private:
    ActivityManager* m_activityManager = nullptr;

    std::unique_ptr<Activity> m_activityPrototype;

    QString m_activityId;
    QString m_activityTitle;

    bool m_hasBeenExecuted = false;
};

#endif