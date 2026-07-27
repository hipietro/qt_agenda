// Update command implementation. Keeps complete polymorphic old and new versions.

#include "UpdateActivityCommand.h"

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

UpdateActivityCommand::UpdateActivityCommand(ActivityManager* activityManager,
                                             const QString& activityId,
                                             std::unique_ptr<Activity> updatedActivity)
    : m_activityManager(activityManager),
      m_activityId(activityId),
      m_updatedActivityPrototype(std::move(updatedActivity))
{
    if (m_updatedActivityPrototype) {
        m_activityTitle = m_updatedActivityPrototype->title();
    }
}

bool UpdateActivityCommand::execute()
{
    if (!m_activityManager ||
        m_activityId.trimmed().isEmpty() ||
        !m_updatedActivityPrototype) {
        return false;
    }

    const Activity* currentActivity = m_activityManager->findActivityById(m_activityId);

    if (!currentActivity) {
        return false;
    }

    // Capture the original polymorphic state only on the first execution.
    if (!m_previousActivityPrototype) {
        m_previousActivityPrototype = currentActivity->clone();

        if (!m_previousActivityPrototype) {
            return false;
        }
    }

    std::unique_ptr<Activity> updatedActivity = m_updatedActivityPrototype->clone();

    if (!updatedActivity) {
        return false;
    }

    if (!m_activityManager->replaceActivity(m_activityId, std::move(updatedActivity))) {
        return false;
    }

    m_hasBeenExecuted = true;
    return true;
}

bool UpdateActivityCommand::undo()
{
    if (!m_activityManager ||
        !m_hasBeenExecuted ||
        !m_previousActivityPrototype) {
        return false;
    }

    if (!m_activityManager->findActivityById(m_activityId)) {
        return false;
    }

    std::unique_ptr<Activity> previousActivity = m_previousActivityPrototype->clone();

    if (!previousActivity) {
        return false;
    }

    if (!m_activityManager->replaceActivity(m_activityId, std::move(previousActivity))) {
        return false;
    }

    m_hasBeenExecuted = false;
    return true;
}

QString UpdateActivityCommand::description() const
{
    return QStringLiteral("Update %1").arg(activityLabel(m_activityTitle));
}

QString UpdateActivityCommand::undoDescription() const
{
    return QStringLiteral("Restore previous version of %1")
        .arg(activityLabel(m_activityTitle));
}

QString UpdateActivityCommand::redoDescription() const
{
    return QStringLiteral("Reapply edits to %1")
        .arg(activityLabel(m_activityTitle));
}

QString UpdateActivityCommand::activityId() const
{
    return m_activityId;
}
