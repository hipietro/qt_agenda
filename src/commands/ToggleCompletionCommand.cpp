// Toggle completion command implementation.

#include "ToggleCompletionCommand.h"

#include "model/Activity.h"
#include "model/ActivityManager.h"

namespace {

QString activityLabel(const QString& title)
{
    return title.trimmed().isEmpty()
        ? QStringLiteral("activity")
        : QStringLiteral("activity \"%1\"").arg(title);
}

QString completionStateLabel(bool completed)
{
    return completed ? QStringLiteral("completed") : QStringLiteral("active");
}

QString completionActionLabel(bool completed)
{
    return completed ? QStringLiteral("Mark completed") : QStringLiteral("Mark active");
}

} // namespace

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
     * Capture the transition once. After undo, redo must reapply the original
     * destination state rather than toggling whatever state happens to exist.
     */
    if (!m_stateCaptured) {
        m_previousCompleted = activity->isCompleted();
        m_newCompleted = !m_previousCompleted;
        m_activityTitle = activity->title();
        m_stateCaptured = true;
    }

    activity->setCompleted(m_newCompleted);
    m_hasBeenExecuted = true;

    return true;
}

bool ToggleCompletionCommand::undo()
{
    if (!m_activityManager || !m_hasBeenExecuted || !m_stateCaptured) {
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
    return QStringLiteral("Mark %1 %2")
        .arg(activityLabel(m_activityTitle), completionStateLabel(m_newCompleted));
}

QString ToggleCompletionCommand::undoDescription() const
{
    return completionActionLabel(m_newCompleted);
}

QString ToggleCompletionCommand::redoDescription() const
{
    return completionActionLabel(m_newCompleted);
}

QString ToggleCompletionCommand::activityId() const
{
    return m_activityId;
}
