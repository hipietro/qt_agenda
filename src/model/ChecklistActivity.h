#ifndef CHECKLISTACTIVITY_H
#define CHECKLISTACTIVITY_H

#include "Activity.h"

#include <QVector>

struct ChecklistItem
{
    QString text;
    bool completed;
};

class ChecklistActivity : public Activity
{
public:
    ChecklistActivity(const QString& title,
                      const QDateTime& dueDate = QDateTime(),
                      const QVector<ChecklistItem>& items = QVector<ChecklistItem>(),
                      const QString& description = QString(),
                      const QString& category = QString(),
                      Priority priority = Priority::Medium,
                      bool completed = false,
                      const QString& id = QString());

    QDateTime dueDate() const;
    void setDueDate(const QDateTime& dueDate);

    QVector<ChecklistItem> items() const;
    void setItems(const QVector<ChecklistItem>& items);

    void addItem(const QString& text);
    bool removeItem(int index);
    bool setItemCompleted(int index, bool completed);

    int totalItems() const;
    int completedItems() const;
    double progressPercentage() const;

    bool isCompleted() const override;
    QDateTime primaryDate() const override;
    bool isOverdue(const QDateTime& now) const override;
    QString summary() const override;
    std::unique_ptr<Activity> clone() const override;

private:
    bool allSubtasksCompleted() const;

    QDateTime m_dueDate;
    QVector<ChecklistItem> m_items;
};

#endif