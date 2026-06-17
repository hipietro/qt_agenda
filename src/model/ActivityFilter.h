#ifndef ACTIVITYFILTER_H
#define ACTIVITYFILTER_H

#include "Activity.h"
#include "ActivityKind.h"
#include "Priority.h"

#include <QDateTime>
#include <QString>

#include <optional>
#include <vector>

class ActivityFilter
{
public:
    enum class CompletionFilter
    {
        Any,
        CompletedOnly,
        ActiveOnly
    };

    enum class OverdueFilter
    {
        Any,
        OverdueOnly,
        NotOverdueOnly
    };

    enum class SortKey
    {
        None,
        Title,
        PrimaryDate,
        Priority,
        Completion,
        CreatedAt,
        UpdatedAt
    };

    enum class SortOrder
    {
        Ascending,
        Descending
    };

    struct Criteria
    {
        std::optional<ActivityKind> kind;
        QString category;
        std::optional<Priority> priority;
        CompletionFilter completion = CompletionFilter::Any;
        OverdueFilter overdue = OverdueFilter::Any;
        std::optional<QDateTime> fromDate;
        std::optional<QDateTime> toDate;
        SortKey sortKey = SortKey::None;
        SortOrder sortOrder = SortOrder::Ascending;
    };

    static std::vector<const Activity*> apply(const std::vector<const Activity*>& activities,
                                              const Criteria& criteria);

    static std::vector<const Activity*> apply(const std::vector<const Activity*>& activities,
                                              const Criteria& criteria,
                                              const QDateTime& now);

private:
    static bool matches(const Activity* activity,
                        const Criteria& criteria,
                        const QDateTime& now);

    static void sortActivities(std::vector<const Activity*>& activities,
                               SortKey sortKey,
                               SortOrder sortOrder);

    static QString normalizedText(const QString& text);
    static int priorityRank(Priority priority);
};

#endif