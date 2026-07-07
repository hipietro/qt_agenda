// Filtering and sorting implementation used by the main list.

#include "ActivityFilter.h"

#include <algorithm>

std::vector<const Activity*> ActivityFilter::apply(const std::vector<const Activity*>& activities,
                                                   const Criteria& criteria)
{
    return apply(activities, criteria, QDateTime::currentDateTime());
}

std::vector<const Activity*> ActivityFilter::apply(const std::vector<const Activity*>& activities,
                                                   const Criteria& criteria,
                                                   const QDateTime& now)
{
    std::vector<const Activity*> result;

    for (const Activity* activity : activities) {
        if (matches(activity, criteria, now)) {
            result.push_back(activity);
        }
    }

    sortActivities(result, criteria.sortKey, criteria.sortOrder);

    return result;
}

bool ActivityFilter::matches(const Activity* activity,
                             const Criteria& criteria,
                             const QDateTime& now)
{
    if (!activity) {
        return false;
    }

    if (criteria.kind.has_value() && activity->kind() != criteria.kind.value()) {
        return false;
    }

    if (!criteria.category.trimmed().isEmpty()) {
        if (normalizedText(activity->category()) != normalizedText(criteria.category)) {
            return false;
        }
    }

    if (criteria.priority.has_value() && activity->priority() != criteria.priority.value()) {
        return false;
    }

    if (criteria.recurring.has_value() &&
        activity->hasRecurrence() != criteria.recurring.value()) {
        return false;
    }

    switch (criteria.completion) {
    case CompletionFilter::CompletedOnly:
        if (!activity->isCompleted()) {
            return false;
        }
        break;

    case CompletionFilter::ActiveOnly:
        if (activity->isCompleted()) {
            return false;
        }
        break;

    case CompletionFilter::Any:
        break;
    }

    switch (criteria.overdue) {
    case OverdueFilter::OverdueOnly:
        if (!activity->isOverdue(now)) {
            return false;
        }
        break;

    case OverdueFilter::NotOverdueOnly:
        if (activity->isOverdue(now)) {
            return false;
        }
        break;

    case OverdueFilter::Any:
        break;
    }

    const QDateTime primaryDate = activity->primaryDate();

    if (criteria.fromDate.has_value()) {
        if (!primaryDate.isValid() || primaryDate < criteria.fromDate.value()) {
            return false;
        }
    }

    if (criteria.toDate.has_value()) {
        if (!primaryDate.isValid() || primaryDate > criteria.toDate.value()) {
            return false;
        }
    }

    return true;
}

void ActivityFilter::sortActivities(std::vector<const Activity*>& activities,
                                    SortKey sortKey,
                                    SortOrder sortOrder)
{
    if (sortKey == SortKey::None) {
        return;
    }

    std::sort(activities.begin(), activities.end(),
              [sortKey, sortOrder](const Activity* first, const Activity* second) {
                  if (!first || !second) {
                      return first != nullptr;
                  }

                  int comparison = 0;

                  switch (sortKey) {
                  case SortKey::Title:
                      comparison = QString::compare(
                          normalizedText(first->title()),
                          normalizedText(second->title())
                      );
                      break;

                  case SortKey::PrimaryDate:
                      if (first->primaryDate() < second->primaryDate()) {
                          comparison = -1;
                      } else if (second->primaryDate() < first->primaryDate()) {
                          comparison = 1;
                      }
                      break;

                  case SortKey::Priority:
                      comparison = priorityRank(first->priority()) - priorityRank(second->priority());
                      break;

                  case SortKey::Completion:
                      comparison = static_cast<int>(first->isCompleted()) -
                                   static_cast<int>(second->isCompleted());
                      break;

                  case SortKey::CreatedAt:
                      if (first->createdAt() < second->createdAt()) {
                          comparison = -1;
                      } else if (second->createdAt() < first->createdAt()) {
                          comparison = 1;
                      }
                      break;

                  case SortKey::UpdatedAt:
                      if (first->updatedAt() < second->updatedAt()) {
                          comparison = -1;
                      } else if (second->updatedAt() < first->updatedAt()) {
                          comparison = 1;
                      }
                      break;

                  case SortKey::None:
                      comparison = 0;
                      break;
                  }

                  if (comparison == 0) {
                      comparison = QString::compare(
                          normalizedText(first->title()),
                          normalizedText(second->title())
                      );
                  }

                  if (comparison == 0) {
                      comparison = QString::compare(first->id(), second->id());
                  }

                  if (sortOrder == SortOrder::Descending) {
                      return comparison > 0;
                  }

                  return comparison < 0;
              });
}

QString ActivityFilter::normalizedText(const QString& text)
{
    return text.simplified().toLower();
}

int ActivityFilter::priorityRank(Priority priority)
{
    switch (priority) {
    case Priority::Low:
        return 0;
    case Priority::Medium:
        return 1;
    case Priority::High:
        return 2;
    case Priority::Critical:
        return 3;
    }

    return 1;
}