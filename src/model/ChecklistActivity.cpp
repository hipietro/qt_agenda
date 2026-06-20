#include "ChecklistActivity.h"

ChecklistActivity::ChecklistActivity(const QString& title,
                                     const QDateTime& dueDate,
                                     const QVector<ChecklistItem>& items,
                                     const QString& description,
                                     const QString& category,
                                     Priority priority,
                                     bool completed,
                                     const QString& id,
                                     const QDateTime& createdAt,
                                     const QDateTime& updatedAt)
    : Activity(title, description, category, priority, completed, id, createdAt, updatedAt),
      m_dueDate(dueDate),
      m_items(items)
{
}

QDateTime ChecklistActivity::dueDate() const
{
    return m_dueDate;
}

void ChecklistActivity::setDueDate(const QDateTime& dueDate)
{
    m_dueDate = dueDate;
    touch();
}

QVector<ChecklistItem> ChecklistActivity::items() const
{
    return m_items;
}

void ChecklistActivity::setItems(const QVector<ChecklistItem>& items)
{
    m_items = items;
    touch();
}

void ChecklistActivity::addItem(const QString& text)
{
    if (!text.trimmed().isEmpty()) {
        m_items.push_back({text.trimmed(), false});
        touch();
    }
}

bool ChecklistActivity::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return false;
    }

    m_items.removeAt(index);
    touch();
    return true;
}

bool ChecklistActivity::setItemCompleted(int index, bool completed)
{
    if (index < 0 || index >= m_items.size()) {
        return false;
    }

    m_items[index].completed = completed;
    touch();
    return true;
}

int ChecklistActivity::totalItems() const
{
    return m_items.size();
}

int ChecklistActivity::completedItems() const
{
    int count = 0;

    for (const ChecklistItem& item : m_items) {
        if (item.completed) {
            ++count;
        }
    }

    return count;
}

double ChecklistActivity::progressPercentage() const
{
    if (m_items.isEmpty()) {
        return 0.0;
    }

    return (static_cast<double>(completedItems()) / static_cast<double>(totalItems())) * 100.0;
}

bool ChecklistActivity::isCompleted() const
{
    return Activity::isCompleted() || allSubtasksCompleted();
}

QDateTime ChecklistActivity::primaryDate() const
{
    return m_dueDate.isValid() ? m_dueDate : createdAt();
}

bool ChecklistActivity::isOverdue(const QDateTime& now) const
{
    return !isCompleted() && m_dueDate.isValid() && m_dueDate < now;
}

QString ChecklistActivity::summary() const
{
    return QString("Checklist: %1 | Progress: %2/%3 (%4%)")
            .arg(title())
            .arg(completedItems())
            .arg(totalItems())
            .arg(progressPercentage(), 0, 'f', 0);
}

std::unique_ptr<Activity> ChecklistActivity::clone() const
{
    return std::make_unique<ChecklistActivity>(*this);
}

bool ChecklistActivity::allSubtasksCompleted() const
{
    if (m_items.isEmpty()) {
        return false;
    }

    for (const ChecklistItem& item : m_items) {
        if (!item.completed) {
            return false;
        }
    }

    return true;
}

ActivityKind ChecklistActivity::kind() const
{
    return ActivityKind::Checklist;
}