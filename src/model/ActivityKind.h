// Small enum used to identify the concrete activity family when needed by UI or JSON.

#ifndef ACTIVITYKIND_H
#define ACTIVITYKIND_H

#include <QString>

enum class ActivityKind
{
    Event,
    Deadline,
    Reminder,
    Checklist
};

inline QString activityKindToString(ActivityKind kind)
{
    switch (kind) {
    case ActivityKind::Event:
        return "Event";
    case ActivityKind::Deadline:
        return "Deadline";
    case ActivityKind::Reminder:
        return "Reminder";
    case ActivityKind::Checklist:
        return "Checklist";
    }

    return "Activity";
}

#endif