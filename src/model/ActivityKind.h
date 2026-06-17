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

inline QString activityKindToItalianString(ActivityKind kind)
{
    switch (kind) {
    case ActivityKind::Event:
        return "Evento";
    case ActivityKind::Deadline:
        return "Scadenza";
    case ActivityKind::Reminder:
        return "Promemoria";
    case ActivityKind::Checklist:
        return "Checklist";
    }

    return "Attività";
}

#endif