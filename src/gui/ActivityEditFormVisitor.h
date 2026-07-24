// Visitor that owns all concrete-type-specific edit form behavior.

#ifndef ACTIVITYEDITFORMVISITOR_H
#define ACTIVITYEDITFORMVISITOR_H

#include "model/Activity.h"
#include "model/ActivityVisitor.h"

#include <QDateTime>
#include <QString>

#include <memory>

class ActivityEditDialog;

class ActivityEditFormVisitor final : public ActivityVisitor
{
public:
    enum class Operation
    {
        Populate,
        Validate,
        Build
    };

    ActivityEditFormVisitor(ActivityEditDialog& dialog, Operation operation);

    bool isValid() const;
    QString errorTitle() const;
    QString errorMessage() const;
    QDateTime primaryDate() const;
    std::unique_ptr<Activity> takeActivity();

    void visit(const EventActivity& activity) override;
    void visit(const DeadlineActivity& activity) override;
    void visit(const ReminderActivity& activity) override;
    void visit(const ChecklistActivity& activity) override;

private:
    void setError(const QString& title, const QString& message);

    ActivityEditDialog& m_dialog;
    Operation m_operation;
    bool m_valid = true;
    QString m_errorTitle;
    QString m_errorMessage;
    QDateTime m_primaryDate;
    std::unique_ptr<Activity> m_activity;
};

#endif
