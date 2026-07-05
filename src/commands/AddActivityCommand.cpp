#include "AddActivityCommand.h"

#include "model/Activity.h"
#include "model/ActivityManager.h"

AddActivityCommand::AddActivityCommand(ActivityManager* activityManager,
                                       std::unique_ptr<Activity> activity)
    : m_activityManager(activityManager),
      m_activityPrototype(std::move(activity))
{
    if (m_activityPrototype) {
        m_activityId = m_activityPrototype->id();
        m_activityTitle = m_activityPrototype->title();
    }
}

bool AddActivityCommand::execute()
{
    if (!m_activityManager || !m_activityPrototype || m_activityId.trimmed().isEmpty()) {
        return false;
    }

    /*
     * Evito di aggiungere due volte la stessa attività.
     * Questo protegge la command se execute() viene richiamato mentre
     * l'attività è già presente nel manager.
     */
    if (m_activityManager->findActivityById(m_activityId)) {
        return false;
    }

    std::unique_ptr<Activity> activityToAdd = m_activityPrototype->clone();

    if (!activityToAdd) {
        return false;
    }

    if (!m_activityManager->addActivity(std::move(activityToAdd))) {
        return false;
    }

    m_hasBeenExecuted = true;
    return true;
}

bool AddActivityCommand::undo()
{
    if (!m_activityManager || !m_hasBeenExecuted) {
        return false;
    }

    if (!m_activityManager->findActivityById(m_activityId)) {
        return false;
    }

    if (!m_activityManager->removeActivity(m_activityId)) {
        return false;
    }

    m_hasBeenExecuted = false;
    return true;
}

QString AddActivityCommand::description() const
{
    if (!m_activityTitle.trimmed().isEmpty()) {
        return QString("Add activity \"%1\"").arg(m_activityTitle);
    }

    return "Add activity";
}

QString AddActivityCommand::activityId() const
{
    return m_activityId;
}