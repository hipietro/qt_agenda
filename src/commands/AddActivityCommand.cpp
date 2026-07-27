// Add command implementation. Stores enough data to undo and redo the insertion.

#include "AddActivityCommand.h"

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
    return QStringLiteral("Add %1").arg(activityLabel(m_activityTitle));
}

QString AddActivityCommand::undoDescription() const
{
    return QStringLiteral("Add activity");
}

QString AddActivityCommand::redoDescription() const
{
    return QStringLiteral("Add activity");
}

QString AddActivityCommand::activityId() const
{
    return m_activityId;
}
