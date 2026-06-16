#ifndef DEADLINEACTIVITY_H
#define DEADLINEACTIVITY_H

#include "Activity.h"

class DeadlineActivity : public Activity
{
public:
    DeadlineActivity(const QString& title,
                     const QDateTime& dueDate,
                     const QString& context = QString(),
                     bool hardDeadline = true,
                     const QString& description = QString(),
                     const QString& category = QString(),
                     Priority priority = Priority::Medium,
                     bool completed = false,
                     const QString& id = QString());

    QDateTime dueDate() const;
    void setDueDate(const QDateTime& dueDate);

    QString context() const;
    void setContext(const QString& context);

    bool isHardDeadline() const;
    void setHardDeadline(bool hardDeadline);

    QDateTime primaryDate() const override;
    bool isOverdue(const QDateTime& now) const override;
    QString summary() const override;
    std::unique_ptr<Activity> clone() const override;

private:
    QDateTime m_dueDate;
    QString m_context;
    bool m_hardDeadline;
};

#endif