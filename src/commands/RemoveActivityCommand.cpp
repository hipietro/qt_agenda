#include "RemoveActivityCommand.h"

#include "model/Activity.h"
#include "model/ActivityManager.h"

RemoveActivityCommand::RemoveActivityCommand(ActivityManager* activityManager,
                                             const QString& activityId)
    : m_activityManager(activityManager),
      m_activityId(activityId)
{
}

bool RemoveActivityCommand::execute()
{
    if (!m_activityManager || m_activityId.trimmed().isEmpty()) {
        return false;
    }

    const Activity* activity = m_activityManager->findActivityById(m_activityId);

    if (!activity) {
        return false;
    }

    /*
     * Prima di eliminare l'attività ne salvo una copia polimorfa.
     * Questa copia sarà usata da undo() per ricostruire l'oggetto eliminato.
     */
    m_removedActivityPrototype = activity->clone();

    if (!m_removedActivityPrototype) {
        return false;
    }

    m_activityTitle = activity->title();

    if (!m_activityManager->removeActivity(m_activityId)) {
        m_removedActivityPrototype.reset();
        return false;
    }

    m_hasBeenExecuted = true;
    return true;
}

bool RemoveActivityCommand::undo()
{
    if (!m_activityManager || !m_hasBeenExecuted || !m_removedActivityPrototype) {
        return false;
    }

    /*
     * Se l'attività esiste già, non provo a reinserirla.
     * Questo evita duplicati con lo stesso id.
     */
    if (m_activityManager->findActivityById(m_activityId)) {
        return false;
    }

    std::unique_ptr<Activity> activityToRestore = m_removedActivityPrototype->clone();

    if (!activityToRestore) {
        return false;
    }

    if (!m_activityManager->addActivity(std::move(activityToRestore))) {
        return false;
    }

    m_hasBeenExecuted = false;
    return true;
}

QString RemoveActivityCommand::description() const
{
    if (!m_activityTitle.trimmed().isEmpty()) {
        return QString("Remove activity \"%1\"").arg(m_activityTitle);
    }

    return "Remove activity";
}

QString RemoveActivityCommand::activityId() const
{
    return m_activityId;
}