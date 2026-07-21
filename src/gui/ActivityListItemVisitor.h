// Builds compact list cards without exposing GUI classes to the logical model.

#ifndef ACTIVITYLISTITEMVISITOR_H
#define ACTIVITYLISTITEMVISITOR_H

#include "model/ActivityVisitor.h"

#include <QDateTime>
#include <QString>

class Activity;
class QVBoxLayout;
class QWidget;

class ActivityListItemVisitor final : public ActivityVisitor
{
public:
    explicit ActivityListItemVisitor(QWidget* parent = nullptr);

    QWidget* takeWidget();
    QString toolTip() const;

    void visit(const EventActivity& activity) override;
    void visit(const DeadlineActivity& activity) override;
    void visit(const ReminderActivity& activity) override;
    void visit(const ChecklistActivity& activity) override;

private:
    QWidget* createCard(const Activity& activity,
                        const QString& typeText,
                        const QString& accentColor,
                        QVBoxLayout*& contentLayout);
    void addCommonFooter(const Activity& activity, QVBoxLayout* contentLayout);
    void storeResult(QWidget* widget, const QString& toolTip);

    QString priorityText(const Activity& activity) const;
    QString formattedDateTime(const QDateTime& dateTime) const;

    QWidget* m_parent = nullptr;
    QWidget* m_widget = nullptr;
    QString m_toolTip;
};

#endif
