// Remove command implementation. The removed activity is kept so undo can restore it.

#include "RemoveActivityCommand.h"

#include "model/Activity.h"
#include "model/ActivityManager.h"

namespace {

QString activityLabel(const QString& title)
{
    return title.trimmed().isEmpty()
        ? QStringLiteral("activity")
        : QStringLiteral("activity \"%1\"").arg(title);
}

} // namespace

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
    return QStringLiteral("Delete %1").arg(activityLabel(m_activityTitle));
}

QString RemoveActivityCommand::undoDescription() const
{
    return QStringLiteral("Delete activity");
}

QString RemoveActivityCommand::redoDescription() const
{
    return QStringLiteral("Delete activity");
}

QString RemoveActivityCommand::activityId() const
{
    return m_activityId;
}
