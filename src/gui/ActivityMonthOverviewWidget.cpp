#include "ActivityMonthOverviewWidget.h"

#include "model/Activity.h"
#include "model/ActivityVisitor.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QAbstractButton>
#include <QCursor>
#include <QDateTime>
#include <QFont>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QSizePolicy>
#include <QStyle>
#include <QStyleOptionFocusRect>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace {

class ActivityMonthMarkerVisitor final : public ActivityVisitor
{
public:
    void visit(const EventActivity&) override
    {
        color = QColor(QStringLiteral("#3F51B5"));
    }

    void visit(const DeadlineActivity&) override
    {
        color = QColor(QStringLiteral("#E00022"));
    }

    void visit(const ReminderActivity&) override
    {
        color = QColor(QStringLiteral("#AB47BC"));
    }

    void visit(const ChecklistActivity&) override
    {
        color = QColor(QStringLiteral("#0B8F2E"));
    }

    QColor color;
};

} // namespace

class MonthDayButton final : public QAbstractButton
{
public:
    explicit MonthDayButton(QWidget* parent = nullptr)
        : QAbstractButton(parent)
    {
        setFocusPolicy(Qt::StrongFocus);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMinimumSize(0, 25);
        setMaximumHeight(27);
    }

    void setDayData(const QDate& date,
                    int activityCount,
                    const QVector<QColor>& markerColors,
                    bool selected,
                    bool today,
                    const QString& tooltip)
    {
        m_date = date;
        m_activityCount = activityCount;
        m_markerColors = markerColors;
        m_selected = selected;
        m_today = today;

        const bool valid = m_date.isValid();

        // Invalid leading and trailing cells keep their grid position but draw nothing.
        setVisible(true);
        setEnabled(valid);
        setCursor(QCursor(valid ? Qt::PointingHandCursor : Qt::ArrowCursor));
        setToolTip(valid ? tooltip : QString());
        setAccessibleName(valid
                              ? QStringLiteral("%1, %2 activities")
                                    .arg(QLocale().toString(m_date, QLocale::LongFormat))
                                    .arg(m_activityCount)
                              : QString());
        update();
    }

    QDate date() const
    {
        return m_date;
    }

    QSize sizeHint() const override
    {
        return QSize(34, 26);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (!m_date.isValid()) {
            return;
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF cellRect = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
        QColor background = palette().color(QPalette::Base);
        QColor border = palette().color(QPalette::Midlight);
        qreal borderWidth = 1.0;

        if (m_selected) {
            background = QColor(QStringLiteral("#E3E8FF"));
            border = QColor(QStringLiteral("#3F51B5"));
            borderWidth = 1.7;
        } else if (underMouse() && isEnabled()) {
            background = palette().color(QPalette::AlternateBase);
            border = palette().color(QPalette::Mid);
        } else if (m_activityCount > 0) {
            background = palette().color(QPalette::AlternateBase);
        }

        painter.setPen(QPen(border, borderWidth));
        painter.setBrush(background);
        painter.drawRoundedRect(cellRect, 4.0, 4.0);

        QFont dayFont = font();
        dayFont.setPointSizeF(std::max(8.0, dayFont.pointSizeF() - 0.5));
        dayFont.setBold(m_today || m_selected);
        painter.setFont(dayFont);
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(rect().adjusted(3, 2, -3, -7),
                         Qt::AlignHCenter | Qt::AlignTop,
                         QString::number(m_date.day()));

        if (m_activityCount > 1 && width() >= 35) {
            QFont countFont = font();
            countFont.setPointSizeF(std::max(6.5, countFont.pointSizeF() - 2.0));
            countFont.setBold(true);
            painter.setFont(countFont);
            painter.setPen(palette().color(QPalette::Mid));
            painter.drawText(QRect(width() - 15, 1, 13, 10),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(m_activityCount));
        }

        const int markerCount = std::min(4, static_cast<int>(m_markerColors.size()));
        if (markerCount > 0) {
            const int diameter = width() < 31 ? 4 : 5;
            const int spacing = 2;
            const int totalWidth = markerCount * diameter + (markerCount - 1) * spacing;
            int x = std::max(2, (width() - totalWidth) / 2);
            const int y = height() - diameter - 3;

            painter.setPen(Qt::NoPen);
            for (int index = 0; index < markerCount; ++index) {
                painter.setBrush(m_markerColors.at(index));
                painter.drawEllipse(QRect(x, y, diameter, diameter));
                x += diameter + spacing;
            }
        } else if (m_today) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(QStringLiteral("#3F51B5")));
            painter.drawEllipse(QPointF(width() / 2.0, height() - 4.0), 2.0, 2.0);
        }

        if (hasFocus()) {
            QStyleOptionFocusRect focusOption;
            focusOption.initFrom(this);
            focusOption.rect = rect().adjusted(2, 2, -2, -2);
            style()->drawPrimitive(QStyle::PE_FrameFocusRect,
                                   &focusOption,
                                   &painter,
                                   this);
        }
    }

    void enterEvent(QEnterEvent* event) override
    {
        QAbstractButton::enterEvent(event);
        update();
    }

    void leaveEvent(QEvent* event) override
    {
        QAbstractButton::leaveEvent(event);
        update();
    }

private:
    QDate m_date;
    int m_activityCount = 0;
    QVector<QColor> m_markerColors;
    bool m_selected = false;
    bool m_today = false;
};

ActivityMonthOverviewWidget::ActivityMonthOverviewWidget(QWidget* parent)
    : QWidget(parent),
      m_displayedMonth(QDate::currentDate().year(), QDate::currentDate().month(), 1)
{
    setObjectName(QStringLiteral("activityMonthOverview"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(202);
    setMaximumHeight(210);

    // The vertical splitter must not assign a large empty lower pane to the compact calendar.
    if (QWidget* container = parentWidget()) {
        container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        container->setMinimumHeight(232);
        container->setMaximumHeight(242);
    }

    setupUi();
    rebuildCalendar();
}

void ActivityMonthOverviewWidget::setActivities(
    const std::vector<const Activity*>& activities)
{
    m_daySummaries.clear();

    for (const Activity* activity : activities) {
        if (!activity) {
            continue;
        }

        const QDateTime primaryDate = activity->primaryDate();
        if (!primaryDate.isValid()) {
            continue;
        }

        const QDate date = primaryDate.date();
        DaySummary& summary = m_daySummaries[date];
        ++summary.total;
        summary.titles.append(activity->title());

        ActivityMonthMarkerVisitor visitor;
        activity->accept(visitor);

        if (visitor.color.isValid() && !summary.colors.contains(visitor.color)) {
            summary.colors.append(visitor.color);
        }
    }

    rebuildCalendar();
}

void ActivityMonthOverviewWidget::setSelectedDate(const std::optional<QDate>& date)
{
    m_selectedDate = date;

    if (m_selectedDate.has_value() && m_selectedDate->isValid()) {
        m_displayedMonth = QDate(m_selectedDate->year(), m_selectedDate->month(), 1);
    }

    rebuildCalendar();
}

void ActivityMonthOverviewWidget::clearSelectedDate()
{
    if (!m_selectedDate.has_value()) {
        return;
    }

    m_selectedDate.reset();
    rebuildCalendar();
}

void ActivityMonthOverviewWidget::setDateSelectedHandler(DateSelectedHandler handler)
{
    m_dateSelectedHandler = std::move(handler);
}

QDate ActivityMonthOverviewWidget::displayedMonth() const
{
    return m_displayedMonth;
}

std::optional<QDate> ActivityMonthOverviewWidget::selectedDate() const
{
    return m_selectedDate;
}

void ActivityMonthOverviewWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 3, 4, 3);
    mainLayout->setSpacing(3);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(5);

    QPushButton* previousButton = new QPushButton(QStringLiteral("‹"), this);
    previousButton->setToolTip(QStringLiteral("Previous month"));
    previousButton->setFixedSize(28, 26);

    m_monthLabel = new QLabel(this);
    m_monthLabel->setObjectName(QStringLiteral("sectionLabel"));
    m_monthLabel->setAlignment(Qt::AlignCenter);
    m_monthLabel->setMinimumWidth(0);

    QPushButton* currentMonthButton = new QPushButton(QStringLiteral("Today"), this);
    currentMonthButton->setToolTip(QStringLiteral("Show the current month"));
    currentMonthButton->setFixedSize(64, 26);

    QPushButton* nextButton = new QPushButton(QStringLiteral("›"), this);
    nextButton->setToolTip(QStringLiteral("Next month"));
    nextButton->setFixedSize(28, 26);

    headerLayout->addWidget(previousButton);
    headerLayout->addWidget(m_monthLabel, 1);
    headerLayout->addWidget(currentMonthButton);
    headerLayout->addWidget(nextButton);
    mainLayout->addLayout(headerLayout);

    QGridLayout* calendarLayout = new QGridLayout();
    calendarLayout->setContentsMargins(0, 0, 0, 0);
    calendarLayout->setHorizontalSpacing(2);
    calendarLayout->setVerticalSpacing(2);

    const QLocale locale;
    for (int day = 1; day <= 7; ++day) {
        QLabel* weekdayLabel = new QLabel(locale.dayName(day, QLocale::NarrowFormat), this);
        weekdayLabel->setAlignment(Qt::AlignCenter);
        weekdayLabel->setObjectName(QStringLiteral("filterFieldLabel"));
        weekdayLabel->setFixedHeight(15);
        calendarLayout->addWidget(weekdayLabel, 0, day - 1);
        calendarLayout->setColumnStretch(day - 1, 1);
    }

    for (int row = 1; row <= 6; ++row) {
        calendarLayout->setRowMinimumHeight(row, 25);
    }

    for (int index = 0; index < 42; ++index) {
        MonthDayButton* dayButton = new MonthDayButton(this);
        m_dayButtons.append(dayButton);
        calendarLayout->addWidget(dayButton, index / 7 + 1, index % 7);

        connect(dayButton, &QAbstractButton::clicked, this, [this, dayButton]() {
            const QDate date = dayButton->date();
            if (!date.isValid()) {
                return;
            }

            m_selectedDate = date;
            rebuildCalendar();

            if (m_dateSelectedHandler) {
                m_dateSelectedHandler(date);
            }
        });
    }

    mainLayout->addLayout(calendarLayout);

    connect(previousButton, &QPushButton::clicked, this, [this]() {
        changeMonth(-1);
    });

    connect(nextButton, &QPushButton::clicked, this, [this]() {
        changeMonth(1);
    });

    connect(currentMonthButton, &QPushButton::clicked, this, [this]() {
        showCurrentMonth();
    });
}

void ActivityMonthOverviewWidget::rebuildCalendar()
{
    if (!m_monthLabel || m_dayButtons.size() != 42 || !m_displayedMonth.isValid()) {
        return;
    }

    const QLocale locale;
    m_monthLabel->setText(locale.toString(m_displayedMonth, QStringLiteral("MMM yyyy")));

    const int firstColumn = m_displayedMonth.dayOfWeek() - 1;
    const int daysInMonth = m_displayedMonth.daysInMonth();
    const QDate today = QDate::currentDate();

    for (int index = 0; index < m_dayButtons.size(); ++index) {
        MonthDayButton* button = m_dayButtons.at(index);
        const int dayNumber = index - firstColumn + 1;

        if (dayNumber < 1 || dayNumber > daysInMonth) {
            button->setDayData(QDate(), 0, {}, false, false, QString());
            continue;
        }

        const QDate date(m_displayedMonth.year(), m_displayedMonth.month(), dayNumber);
        const DaySummary summary = m_daySummaries.value(date);
        const bool selected = m_selectedDate.has_value() && m_selectedDate.value() == date;

        button->setDayData(date,
                           summary.total,
                           summary.colors,
                           selected,
                           date == today,
                           tooltipForDate(date, summary));
    }
}

void ActivityMonthOverviewWidget::changeMonth(int monthOffset)
{
    m_displayedMonth = m_displayedMonth.addMonths(monthOffset);
    rebuildCalendar();
}

void ActivityMonthOverviewWidget::showCurrentMonth()
{
    const QDate today = QDate::currentDate();
    m_displayedMonth = QDate(today.year(), today.month(), 1);
    rebuildCalendar();
}

QString ActivityMonthOverviewWidget::tooltipForDate(
    const QDate& date,
    const DaySummary& summary) const
{
    const QLocale locale;
    QString tooltip = locale.toString(date, QLocale::LongFormat);

    if (summary.total == 0) {
        return tooltip + QStringLiteral("\nNo activities");
    }

    tooltip += QStringLiteral("\n%1 %2")
                   .arg(summary.total)
                   .arg(summary.total == 1 ? QStringLiteral("activity")
                                           : QStringLiteral("activities"));

    const int visibleTitles = std::min(5, static_cast<int>(summary.titles.size()));
    for (int index = 0; index < visibleTitles; ++index) {
        tooltip += QStringLiteral("\n• %1").arg(summary.titles.at(index));
    }

    if (summary.titles.size() > visibleTitles) {
        tooltip += QStringLiteral("\n… and %1 more")
                       .arg(summary.titles.size() - visibleTitles);
    }

    return tooltip;
}
