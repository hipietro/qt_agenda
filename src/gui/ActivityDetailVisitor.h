// Builds type-specific detail pages through Activity Visitor dispatch.

#ifndef ACTIVITYDETAILVISITOR_H
#define ACTIVITYDETAILVISITOR_H

#include "model/ActivityVisitor.h"

#include <QString>

class Activity;
class QDateTime;
class QGridLayout;
class QVBoxLayout;
class QWidget;

class ActivityDetailVisitor final : public ActivityVisitor
{
public:
    explicit ActivityDetailVisitor(QWidget* parent = nullptr);

    QWidget* takeWidget();

    void visit(const EventActivity& activity) override;
    void visit(const DeadlineActivity& activity) override;
    void visit(const ReminderActivity& activity) override;
    void visit(const ChecklistActivity& activity) override;

private:
    QWidget* createPage(const Activity& activity,
                        const QString& typeText,
                        const QString& accentColor,
                        QVBoxLayout*& contentLayout);
    QWidget* createSection(const QString& title,
                           QWidget* parent,
                           QGridLayout*& gridLayout) const;
    void addKeyValue(QGridLayout* layout,
                     int row,
                     const QString& key,
                     const QString& value,
                     QWidget* parent) const;
    void addCommonSections(const Activity& activity,
                           QVBoxLayout* contentLayout,
                           QWidget* page) const;
    void storeResult(QWidget* widget);

    QString priorityText(const Activity& activity) const;
    QString statusText(const Activity& activity) const;
    QString recurrenceText(const Activity& activity) const;
    QString formattedDateTime(const QDateTime& dateTime) const;
    QString durationText(qint64 seconds) const;
    QString relativeTimeText(qint64 seconds) const;

    QWidget* m_parent = nullptr;
    QWidget* m_widget = nullptr;
};

#endif
