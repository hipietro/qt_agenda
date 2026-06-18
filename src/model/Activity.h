#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QString>
#include <QDateTime>
#include <memory>
#include <optional>

#include "Priority.h"
#include "ActivityKind.h"
#include "RecurrenceRule.h"

class Activity
{
public:
    Activity(const QString& title,
             const QString& description = QString(),
             const QString& category = QString(),
             Priority priority = Priority::Medium,
             bool completed = false,
             const QString& id = QString(),
             const QDateTime& createdAt = QDateTime::currentDateTime(),
             const QDateTime& updatedAt = QDateTime::currentDateTime());

    virtual ~Activity() = default;

    QString id() const;

    QString title() const;
    void setTitle(const QString& title);

    QString description() const;
    void setDescription(const QString& description);

    QString category() const;
    void setCategory(const QString& category);

    Priority priority() const;
    void setPriority(Priority priority);

    virtual bool isCompleted() const;
    void setCompleted(bool completed);

    QDateTime createdAt() const;
    QDateTime updatedAt() const;

    bool hasRecurrence() const;
    std::optional<RecurrenceRule> recurrenceRule() const;
    void setRecurrenceRule(const RecurrenceRule& recurrenceRule);
    void clearRecurrenceRule();
    QDateTime nextOccurrenceAfter(const QDateTime& after) const;

    virtual ActivityKind kind() const = 0;
    virtual QDateTime primaryDate() const = 0;
    virtual bool isOverdue(const QDateTime& now) const = 0;
    virtual QString summary() const = 0;
    virtual std::unique_ptr<Activity> clone() const = 0;

    std::unique_ptr<Activity> cloneWithNewId() const;

protected:
    void touch();

private:
    QString m_id;
    QString m_title;
    QString m_description;
    QString m_category;
    Priority m_priority;
    bool m_completed;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    std::optional<RecurrenceRule> m_recurrenceRule;

    static QString generateId();
};

#endif