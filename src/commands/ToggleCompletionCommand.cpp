#include "ToggleCompletionCommand.h"

#include "model/Activity.h"
#include "model/ActivityManager.h"

ToggleCompletionCommand::ToggleCompletionCommand(ActivityManager* activityManager,
                                                 const QString& activityId)
    : m_activityManager(activityManager),
      m_activityId(activityId)
{
}

bool ToggleCompletionCommand::execute()
{
    if (!m_activityManager || m_activityId.trimmed().isEmpty()) {
        return false;
    }

    Activity* activity = m_activityManager->findActivityById(m_activityId);

    if (!activity) {
        return false;
    }

    /*
     * Salvo lo stato iniziale solo alla prima esecuzione.
     * Così un futuro redo non cambia il valore originale usato da undo().
     */
    if (!m_hasBeenExecuted) {
        m_previousCompleted = activity->isCompleted();
        m_newCompleted = !m_previousCompleted;
        m_activityTitle = activity->title();
    }

    activity->setCompleted(m_newCompleted);
    m_hasBeenExecuted = true;

    return true;
}

bool ToggleCompletionCommand::undo()
{
    if (!m_activityManager || !m_hasBeenExecuted) {
        return false;
    }

    Activity* activity = m_activityManager->findActivityById(m_activityId);

    if (!activity) {
        return false;
    }

    activity->setCompleted(m_previousCompleted);
    m_hasBeenExecuted = false;

    return true;
}

QString ToggleCompletionCommand::description() const
{
    if (!m_activityTitle.trimmed().isEmpty()) {
        return QString("Toggle completion for \"%1\"").arg(m_activityTitle);
    }

    return "Toggle completion";
}

QString ToggleCompletionCommand::activityId() const
{
    return m_activityId;
}