#ifndef ACTIVITYMONTHOVERVIEWWIDGET_H
#define ACTIVITYMONTHOVERVIEWWIDGET_H

#include <QColor>
#include <QDate>
#include <QMap>
#include <QStringList>
#include <QVector>
#include <QWidget>

#include <functional>
#include <optional>
#include <vector>

class Activity;
class QLabel;
class MonthDayButton;

class ActivityMonthOverviewWidget final : public QWidget
{
public:
    using DateSelectedHandler = std::function<void(const QDate&)>;

    explicit ActivityMonthOverviewWidget(QWidget* parent = nullptr);

    void setActivities(const std::vector<const Activity*>& activities);
    void setSelectedDate(const std::optional<QDate>& date);
    void clearSelectedDate();
    void setDateSelectedHandler(DateSelectedHandler handler);

    QDate displayedMonth() const;
    std::optional<QDate> selectedDate() const;

private:
    struct DaySummary
    {
        int total = 0;
        QVector<QColor> colors;
        QStringList titles;
    };

    void setupUi();
    void rebuildCalendar();
    void changeMonth(int monthOffset);
    void showCurrentMonth();
    QString tooltipForDate(const QDate& date, const DaySummary& summary) const;

    QDate m_displayedMonth;
    std::optional<QDate> m_selectedDate;
    DateSelectedHandler m_dateSelectedHandler;
    QMap<QDate, DaySummary> m_daySummaries;

    QLabel* m_monthLabel = nullptr;
    QVector<MonthDayButton*> m_dayButtons;
};

#endif
