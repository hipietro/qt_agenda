// Visitor contract for operations that depend on the concrete activity type.
// The interface belongs to the logical model and intentionally has no Qt Widgets dependencies.

#ifndef ACTIVITYVISITOR_H
#define ACTIVITYVISITOR_H

class EventActivity;
class DeadlineActivity;
class ReminderActivity;
class ChecklistActivity;

class ActivityVisitor
{
public:
    virtual ~ActivityVisitor() = default;

    virtual void visit(const EventActivity& activity) = 0;
    virtual void visit(const DeadlineActivity& activity) = 0;
    virtual void visit(const ReminderActivity& activity) = 0;
    virtual void visit(const ChecklistActivity& activity) = 0;
};

#endif
