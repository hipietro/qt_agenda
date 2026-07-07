// Priority values used by activities, filters and list styling.

#ifndef PRIORITY_H
#define PRIORITY_H

#include <QString>

enum class Priority
{
    Low,
    Medium,
    High,
    Critical
};

inline QString priorityToString(Priority priority)
{
    switch (priority) {
    case Priority::Low:
        return "Low";
    case Priority::Medium:
        return "Medium";
    case Priority::High:
        return "High";
    case Priority::Critical:
        return "Critical";
    }

    return "Medium";
}

#endif