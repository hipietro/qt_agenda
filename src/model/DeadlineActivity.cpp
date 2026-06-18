#include "DeadlineActivity.h"

DeadlineActivity::DeadlineActivity(const QString& title,
                                   const QDateTime& dueDate,
                                   const QString& context,
                                   bool hardDeadline,
                                   const QString& description,
                                   const QString& category,
                                   Priority priority,
                                   bool completed,
                                   const QString& id)
    : Activity(title, description, category, priority, completed, id),
      m_dueDate(dueDate),
      m_context(context),
      m_hardDeadline(hardDeadline)
{
}

QDateTime DeadlineActivity::dueDate() const
{
    return m_dueDate;
}

void DeadlineActivity::setDueDate(const QDateTime& dueDate)
{
    m_dueDate = dueDate;
    touch();
}

QString DeadlineActivity::context() const
{
    return m_context;
}

void DeadlineActivity::setContext(const QString& context)
{
    m_context = context;
    touch();
}

bool DeadlineActivity::isHardDeadline() const
{
    return m_hardDeadline;
}

void DeadlineActivity::setHardDeadline(bool hardDeadline)
{
    m_hardDeadline = hardDeadline;
    touch();
}

QDateTime DeadlineActivity::primaryDate() const
{
    return m_dueDate;
}

bool DeadlineActivity::isOverdue(const QDateTime& now) const
{
    return !isCompleted() && m_dueDate.isValid() && m_dueDate < now;
}

QString DeadlineActivity::summary() const
{
    QString text = QString("Deadline: %1 | Due: %2")
            .arg(title(), m_dueDate.toString("yyyy-MM-dd HH:mm"));

    if (!m_context.isEmpty()) {
        text += QString(" | Context: %1").arg(m_context);
    }

    text += m_hardDeadline ? " | Hard deadline" : " | Soft deadline";

    return text;
}

std::unique_ptr<Activity> DeadlineActivity::clone() const
{
    return std::make_unique<DeadlineActivity>(*this);
}

ActivityKind DeadlineActivity::kind() const
{
    return ActivityKind::Deadline;
}