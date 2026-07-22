// Serializes concrete activity data through Visitor double dispatch.

#ifndef ACTIVITYJSONSERIALIZATIONVISITOR_H
#define ACTIVITYJSONSERIALIZATIONVISITOR_H

#include "model/ActivityVisitor.h"

#include <QJsonObject>

class ActivityJsonSerializationVisitor final : public ActivityVisitor
{
public:
    QJsonObject json() const;

    void visit(const EventActivity& activity) override;
    void visit(const DeadlineActivity& activity) override;
    void visit(const ReminderActivity& activity) override;
    void visit(const ChecklistActivity& activity) override;

private:
    QJsonObject m_json;
};

#endif
