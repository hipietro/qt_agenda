#include "ActivityListItemVisitor.h"

#include "model/Activity.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QDate>
#include <QDateTime>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

ActivityListItemVisitor::ActivityListItemVisitor(QWidget* parent)
    : m_parent(parent)
{
}

QWidget* ActivityListItemVisitor::takeWidget()
{
    QWidget* result = m_widget;
    m_widget = nullptr;
    return result;
}

QString ActivityListItemVisitor::toolTip() const
{
    return m_toolTip;
}

void ActivityListItemVisitor::visit(const EventActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* card = createCard(activity, "EVENT", "#3F51B5", contentLayout);

    const QDateTime start = activity.startDateTime();
    const QDateTime end = activity.endDateTime();
    const QString timing = start.date() == end.date()
            ? QString("%1  •  %2–%3")
                  .arg(start.date().toString("ddd d MMM"),
                       start.time().toString("HH:mm"),
                       end.time().toString("HH:mm"))
            : QString("%1  →  %2")
                  .arg(formattedDateTime(start), formattedDateTime(end));

    QLabel* timingLabel = new QLabel(timing, card);
    timingLabel->setObjectName("activityCardPrimary");
    contentLayout->addWidget(timingLabel);

    const QString location = activity.location().trimmed().isEmpty()
            ? "No location"
            : activity.location().trimmed();
    QLabel* locationLabel = new QLabel(QString("Location: %1").arg(location), card);
    locationLabel->setObjectName("activityCardSecondary");
    contentLayout->addWidget(locationLabel);

    addCommonFooter(activity, contentLayout);

    storeResult(card,
                QString("Event: %1\n%2\nLocation: %3")
                    .arg(activity.title(), timing, location));
}

void ActivityListItemVisitor::visit(const DeadlineActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* card = createCard(activity, "DEADLINE", "#C62828", contentLayout);

    const bool overdue = !activity.isCompleted()
            && activity.isOverdue(QDateTime::currentDateTime());
    const QString state = activity.isCompleted()
            ? "COMPLETED"
            : overdue ? "OVERDUE"
                      : activity.isHardDeadline() ? "HARD DEADLINE" : "DUE";

    QHBoxLayout* dueLayout = new QHBoxLayout();
    dueLayout->setContentsMargins(0, 0, 0, 0);
    dueLayout->setSpacing(8);

    QLabel* dueLabel = new QLabel(
        QString("Due %1").arg(formattedDateTime(activity.dueDate())), card);
    dueLabel->setObjectName("activityCardPrimary");

    QLabel* stateLabel = new QLabel(state, card);
    stateLabel->setObjectName("activityStateBadge");
    stateLabel->setStyleSheet(overdue
        ? "background:#FDE7E7; color:#9B1C1C; border:1px solid #E5A5A5; border-radius:4px; padding:2px 6px; font-weight:700;"
        : "background:#F1F1F1; color:#444444; border:1px solid #D2D2D2; border-radius:4px; padding:2px 6px; font-weight:700;");

    dueLayout->addWidget(dueLabel, 1);
    dueLayout->addWidget(stateLabel, 0);
    contentLayout->addLayout(dueLayout);

    const QString context = activity.context().trimmed().isEmpty()
            ? "No context"
            : activity.context().trimmed();
    QLabel* contextLabel = new QLabel(QString("Context: %1").arg(context), card);
    contextLabel->setObjectName("activityCardSecondary");
    contentLayout->addWidget(contextLabel);

    addCommonFooter(activity, contentLayout);

    storeResult(card,
                QString("Deadline: %1\nDue: %2\nState: %3\nContext: %4")
                    .arg(activity.title(),
                         formattedDateTime(activity.dueDate()),
                         state,
                         context));
}

void ActivityListItemVisitor::visit(const ReminderActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* card = createCard(activity, "REMINDER", "#8E44AD", contentLayout);

    QLabel* dateLabel = new QLabel(
        QString("Remind at %1").arg(formattedDateTime(activity.reminderDateTime())), card);
    dateLabel->setObjectName("activityCardPrimary");
    contentLayout->addWidget(dateLabel);

    const QString alertText = activity.advanceMinutes() > 0
            ? QString("Alert %1 min before").arg(activity.advanceMinutes())
            : "Alert at the scheduled time";
    QLabel* alertLabel = new QLabel(alertText, card);
    alertLabel->setObjectName("activityCardSecondary");
    contentLayout->addWidget(alertLabel);

    const QString note = activity.reminderNote().trimmed();
    if (!note.isEmpty()) {
        QLabel* noteLabel = new QLabel(note, card);
        noteLabel->setObjectName("activityCardNote");
        noteLabel->setWordWrap(true);
        contentLayout->addWidget(noteLabel);
    }

    addCommonFooter(activity, contentLayout);

    storeResult(card,
                QString("Reminder: %1\n%2\n%3")
                    .arg(activity.title(),
                         formattedDateTime(activity.reminderDateTime()),
                         alertText));
}

void ActivityListItemVisitor::visit(const ChecklistActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* card = createCard(activity, "CHECKLIST", "#2E7D32", contentLayout);
    card->setMinimumHeight(104);

    const int total = activity.totalItems();
    const int completed = activity.completedItems();
    const int progress = total > 0 ? qRound((completed * 100.0) / total) : 0;

    QHBoxLayout* progressTextLayout = new QHBoxLayout();
    progressTextLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* dueLabel = new QLabel(
        activity.dueDate().isValid()
            ? QString("Due %1").arg(formattedDateTime(activity.dueDate()))
            : "No due date",
        card);
    dueLabel->setObjectName("activityCardPrimary");

    QLabel* countLabel = new QLabel(
        QString("%1/%2 completed").arg(completed).arg(total), card);
    countLabel->setObjectName("activityCardSecondary");

    progressTextLayout->addWidget(dueLabel, 1);
    progressTextLayout->addWidget(countLabel, 0);
    contentLayout->addLayout(progressTextLayout);

    QProgressBar* progressBar = new QProgressBar(card);
    progressBar->setObjectName("activityProgress");
    progressBar->setRange(0, 100);
    progressBar->setValue(progress);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(8);
    progressBar->setStyleSheet(
        "QProgressBar { background:#E3E7E3; border:none; border-radius:4px; }"
        "QProgressBar::chunk { background:#43A047; border-radius:4px; }");
    contentLayout->addWidget(progressBar);

    addCommonFooter(activity, contentLayout);

    storeResult(card,
                QString("Checklist: %1\nProgress: %2/%3 (%4%)")
                    .arg(activity.title())
                    .arg(completed)
                    .arg(total)
                    .arg(progress));
}

QWidget* ActivityListItemVisitor::createCard(const Activity& activity,
                                              const QString& typeText,
                                              const QString& accentColor,
                                              QVBoxLayout*& contentLayout)
{
    QWidget* card = new QWidget(m_parent);
    card->setObjectName("activityCard");
    card->setProperty("selected", false);
    card->setAttribute(Qt::WA_TransparentForMouseEvents);
    card->setMinimumHeight(88);
    card->setStyleSheet(
        "QWidget#activityCard { background:transparent; }"
        "QWidget#activityCard QLabel { background:transparent; color:#222222; }"
        "QWidget#activityCard[selected=\"true\"] QLabel { color:#FFFFFF; }"
        "QLabel#activityCardTitle { font-size:14px; font-weight:700; }"
        "QLabel#activityCardPrimary { font-weight:600; }"
        "QLabel#activityCardSecondary, QLabel#activityCardFooter { color:#555555; font-size:12px; }"
        "QLabel#activityCardNote { color:#444444; font-size:12px; }"
        "QWidget#activityCard[selected=\"true\"] QLabel#activityCardSecondary,"
        "QWidget#activityCard[selected=\"true\"] QLabel#activityCardFooter,"
        "QWidget#activityCard[selected=\"true\"] QLabel#activityCardNote { color:#F2F4FF; }");

    QHBoxLayout* rootLayout = new QHBoxLayout(card);
    rootLayout->setContentsMargins(6, 6, 8, 6);
    rootLayout->setSpacing(9);

    QFrame* accent = new QFrame(card);
    accent->setFixedWidth(5);
    accent->setStyleSheet(
        QString("background:%1; border-radius:2px;").arg(accentColor));
    rootLayout->addWidget(accent);

    QWidget* content = new QWidget(card);
    content->setStyleSheet("background:transparent;");
    contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(3);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(activity.title(), card);
    titleLabel->setObjectName("activityCardTitle");
    titleLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    QFont titleFont = titleLabel->font();
    titleFont.setStrikeOut(activity.isCompleted());
    titleLabel->setFont(titleFont);

    QLabel* typeLabel = new QLabel(typeText, card);
    typeLabel->setObjectName("activityTypeBadge");
    typeLabel->setStyleSheet(
        QString("background:%1; color:#FFFFFF; border-radius:4px; padding:2px 6px; font-size:10px; font-weight:700;")
            .arg(accentColor));

    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(typeLabel, 0);
    contentLayout->addLayout(headerLayout);

    rootLayout->addWidget(content, 1);
    return card;
}

void ActivityListItemVisitor::addCommonFooter(const Activity& activity,
                                               QVBoxLayout* contentLayout)
{
    QStringList parts;
    parts << (activity.category().trimmed().isEmpty()
                  ? "No category"
                  : activity.category().trimmed());
    parts << QString("Priority: %1").arg(priorityText(activity));

    if (activity.hasRecurrence()) {
        parts << "Repeating";
    }
    if (activity.isCompleted()) {
        parts << "Completed";
    }

    QLabel* footer = new QLabel(parts.join("  •  "), m_parent);
    footer->setObjectName("activityCardFooter");
    contentLayout->addWidget(footer);
}

void ActivityListItemVisitor::storeResult(QWidget* widget, const QString& toolTip)
{
    m_widget = widget;
    m_toolTip = toolTip;
}

QString ActivityListItemVisitor::priorityText(const Activity& activity) const
{
    switch (activity.priority()) {
    case Priority::Low:
        return "Low";
    case Priority::Medium:
        return "Medium";
    case Priority::High:
        return "High";
    case Priority::Critical:
        return "Critical";
    }

    return "Unknown";
}

QString ActivityListItemVisitor::formattedDateTime(const QDateTime& dateTime) const
{
    return dateTime.isValid()
            ? dateTime.toString("ddd d MMM, HH:mm")
            : "Not set";
}
