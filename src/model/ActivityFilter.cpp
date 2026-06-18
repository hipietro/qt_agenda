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

                  bool result = false;

                  switch (sortKey) {
                  case SortKey::Title:
                      result = normalizedText(first->title()) < normalizedText(second->title());
                      break;

                  case SortKey::PrimaryDate:
                      result = first->primaryDate() < second->primaryDate();
                      break;

                  case SortKey::Priority:
                      result = priorityRank(first->priority()) < priorityRank(second->priority());
                      break;

                  case SortKey::Completion:
                      result = first->isCompleted() < second->isCompleted();
                      break;

                  case SortKey::CreatedAt:
                      result = first->createdAt() < second->createdAt();
                      break;

                  case SortKey::UpdatedAt:
                      result = first->updatedAt() < second->updatedAt();
                      break;

                  case SortKey::None:
                      result = false;
                      break;
                  }

                  if (sortOrder == SortOrder::Descending) {
                      return !result && first != second;
                  }

                  return result;
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