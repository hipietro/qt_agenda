#include "ActivityDetailVisitor.h"

#include "model/Activity.h"
#include "model/ChecklistActivity.h"
#include "model/DeadlineActivity.h"
#include "model/EventActivity.h"
#include "model/ReminderActivity.h"

#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

ActivityDetailVisitor::ActivityDetailVisitor(QWidget* parent)
    : m_parent(parent)
{
}

QWidget* ActivityDetailVisitor::takeWidget()
{
    QWidget* result = m_widget;
    m_widget = nullptr;
    return result;
}

void ActivityDetailVisitor::visit(const EventActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* page = createPage(activity, "EVENT", "#3F51B5", contentLayout);

    QGridLayout* scheduleLayout = nullptr;
    QWidget* scheduleSection = createSection("Event schedule", page, scheduleLayout);

    addKeyValue(scheduleLayout, 0, "Starts", formattedDateTime(activity.startDateTime()), page);
    addKeyValue(scheduleLayout, 1, "Ends", formattedDateTime(activity.endDateTime()), page);

    const qint64 durationSeconds = activity.startDateTime().secsTo(activity.endDateTime());
    addKeyValue(scheduleLayout, 2, "Duration", durationText(durationSeconds), page);
    addKeyValue(scheduleLayout,
                3,
                "Location",
                activity.location().trimmed().isEmpty()
                    ? "No location specified"
                    : activity.location().trimmed(),
                page);

    const QString participants = activity.participants().isEmpty()
            ? "No participants"
            : activity.participants().join(", ");
    addKeyValue(scheduleLayout, 4, "Participants", participants, page);

    contentLayout->addWidget(scheduleSection);
    addCommonSections(activity, contentLayout, page);
    contentLayout->addStretch();

    storeResult(page);
}

void ActivityDetailVisitor::visit(const DeadlineActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* page = createPage(activity, "DEADLINE", "#C62828", contentLayout);

    QGridLayout* deadlineLayout = nullptr;
    QWidget* deadlineSection = createSection("Deadline status", page, deadlineLayout);

    const QDateTime now = QDateTime::currentDateTime();
    QString dueState;

    if (activity.isCompleted()) {
        dueState = "Completed";
    } else {
        const qint64 remainingSeconds = now.secsTo(activity.dueDate());
        dueState = remainingSeconds < 0
                ? QString("Overdue by %1").arg(relativeTimeText(-remainingSeconds))
                : QString("Due in %1").arg(relativeTimeText(remainingSeconds));
    }

    addKeyValue(deadlineLayout, 0, "Due date", formattedDateTime(activity.dueDate()), page);
    addKeyValue(deadlineLayout, 1, "Time state", dueState, page);
    addKeyValue(deadlineLayout,
                2,
                "Context",
                activity.context().trimmed().isEmpty()
                    ? "No context specified"
                    : activity.context().trimmed(),
                page);
    addKeyValue(deadlineLayout,
                3,
                "Requirement",
                activity.isHardDeadline() ? "Hard deadline" : "Flexible deadline",
                page);

    contentLayout->addWidget(deadlineSection);
    addCommonSections(activity, contentLayout, page);
    contentLayout->addStretch();

    storeResult(page);
}

void ActivityDetailVisitor::visit(const ReminderActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* page = createPage(activity, "REMINDER", "#8E44AD", contentLayout);

    QGridLayout* reminderLayout = nullptr;
    QWidget* reminderSection = createSection("Reminder settings", page, reminderLayout);

    const QDateTime alertDateTime = activity.reminderDateTime().addSecs(
        -static_cast<qint64>(activity.advanceMinutes()) * 60);

    addKeyValue(reminderLayout,
                0,
                "Reminder time",
                formattedDateTime(activity.reminderDateTime()),
                page);
    addKeyValue(reminderLayout,
                1,
                "Advance notice",
                activity.advanceMinutes() > 0
                    ? QString("%1 minutes before").arg(activity.advanceMinutes())
                    : "At the scheduled time",
                page);
    addKeyValue(reminderLayout, 2, "Alert time", formattedDateTime(alertDateTime), page);
    addKeyValue(reminderLayout,
                3,
                "Reminder note",
                activity.reminderNote().trimmed().isEmpty()
                    ? "No reminder note"
                    : activity.reminderNote().trimmed(),
                page);

    contentLayout->addWidget(reminderSection);
    addCommonSections(activity, contentLayout, page);
    contentLayout->addStretch();

    storeResult(page);
}

void ActivityDetailVisitor::visit(const ChecklistActivity& activity)
{
    QVBoxLayout* contentLayout = nullptr;
    QWidget* page = createPage(activity, "CHECKLIST", "#2E7D32", contentLayout);

    QGridLayout* progressLayout = nullptr;
    QWidget* progressSection = createSection("Checklist progress", page, progressLayout);

    const int totalItems = activity.totalItems();
    const int completedItems = activity.completedItems();
    const int progress = totalItems > 0
            ? static_cast<int>((completedItems * 100.0) / totalItems)
            : 0;

    addKeyValue(progressLayout,
                0,
                "Due date",
                activity.dueDate().isValid()
                    ? formattedDateTime(activity.dueDate())
                    : "No due date",
                page);
    addKeyValue(progressLayout,
                1,
                "Completed",
                QString("%1 of %2 items").arg(completedItems).arg(totalItems),
                page);

    QProgressBar* progressBar = new QProgressBar(page);
    progressBar->setRange(0, 100);
    progressBar->setValue(progress);
    progressBar->setFormat(QString("%1%").arg(progress));
    progressBar->setObjectName("activityDetailProgress");
    progressLayout->addWidget(progressBar, 2, 0, 1, 2);

    contentLayout->addWidget(progressSection);

    QGridLayout* itemsLayout = nullptr;
    QWidget* itemsSection = createSection("Checklist items", page, itemsLayout);

    if (activity.items().isEmpty()) {
        QLabel* emptyLabel = new QLabel("No checklist items", page);
        emptyLabel->setObjectName("activityDetailMuted");
        itemsLayout->addWidget(emptyLabel, 0, 0, 1, 2);
    } else {
        int row = 0;
        for (const ChecklistItem& item : activity.items()) {
            QLabel* marker = new QLabel(item.completed ? "✓" : "○", page);
            marker->setObjectName(item.completed
                                  ? "activityChecklistMarkerDone"
                                  : "activityChecklistMarkerOpen");
            marker->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
            marker->setFixedWidth(24);

            QLabel* itemLabel = new QLabel(item.text, page);
            itemLabel->setWordWrap(true);
            itemLabel->setObjectName(item.completed
                                     ? "activityChecklistItemDone"
                                     : "activityChecklistItemOpen");

            itemsLayout->addWidget(marker, row, 0);
            itemsLayout->addWidget(itemLabel, row, 1);
            ++row;
        }
    }

    itemsLayout->setColumnStretch(1, 1);
    contentLayout->addWidget(itemsSection);
    addCommonSections(activity, contentLayout, page);
    contentLayout->addStretch();

    storeResult(page);
}

QWidget* ActivityDetailVisitor::createPage(const Activity& activity,
                                           const QString& typeText,
                                           const QString& accentColor,
                                           QVBoxLayout*& contentLayout)
{
    QWidget* page = new QWidget(m_parent);
    page->setObjectName("activityDetailPage");
    page->setStyleSheet(
        "QWidget#activityDetailPage { background:#FFFFFF; color:#222222; }"
        "QFrame#activityDetailHeader { background:#F7F8FC; border:1px solid #D9DDEA; border-radius:10px; }"
        "QFrame#activityDetailSection { background:#FFFFFF; border:1px solid #DFDFDF; border-radius:8px; }"
        "QLabel#activityDetailTitle { font-size:20px; font-weight:700; color:#1F1F1F; }"
        "QLabel#activityDetailSectionTitle { font-size:14px; font-weight:700; color:#333333; }"
        "QLabel#activityDetailKey { color:#666666; font-size:12px; font-weight:600; }"
        "QLabel#activityDetailValue { color:#222222; font-size:13px; }"
        "QLabel#activityDetailMuted { color:#777777; font-size:13px; }"
        "QLabel#activityChecklistMarkerDone { color:#2E7D32; font-size:18px; font-weight:700; }"
        "QLabel#activityChecklistMarkerOpen { color:#8A8A8A; font-size:18px; }"
        "QLabel#activityChecklistItemDone { color:#777777; text-decoration:line-through; }"
        "QLabel#activityChecklistItemOpen { color:#222222; }"
        "QProgressBar#activityDetailProgress { background:#E5E9E5; border:none; border-radius:5px; height:14px; text-align:center; color:#222222; font-size:10px; }"
        "QProgressBar#activityDetailProgress::chunk { background:#43A047; border-radius:5px; }");

    contentLayout = new QVBoxLayout(page);
    contentLayout->setContentsMargins(14, 14, 14, 14);
    contentLayout->setSpacing(12);

    QFrame* header = new QFrame(page);
    header->setObjectName("activityDetailHeader");
    header->setStyleSheet(
        QString("QFrame#activityDetailHeader { border-left:5px solid %1; }")
            .arg(accentColor));

    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(14, 12, 14, 12);
    headerLayout->setSpacing(8);

    QHBoxLayout* titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(activity.title(), header);
    titleLabel->setObjectName("activityDetailTitle");
    titleLabel->setWordWrap(true);

    QLabel* typeBadge = new QLabel(typeText, header);
    typeBadge->setStyleSheet(
        QString("background:%1; color:#FFFFFF; border-radius:5px; padding:3px 8px; font-size:10px; font-weight:700;")
            .arg(accentColor));

    titleLayout->addWidget(titleLabel, 1);
    titleLayout->addWidget(typeBadge, 0, Qt::AlignTop);
    headerLayout->addLayout(titleLayout);

    QLabel* stateBadge = new QLabel(statusText(activity), header);
    stateBadge->setAlignment(Qt::AlignCenter);
    stateBadge->setMaximumWidth(120);

    if (activity.isCompleted()) {
        stateBadge->setStyleSheet(
            "background:#E7F4EA; color:#236A32; border:1px solid #A8D5B1; border-radius:5px; padding:3px 8px; font-weight:700;");
    } else if (activity.isOverdue(QDateTime::currentDateTime())) {
        stateBadge->setStyleSheet(
            "background:#FDE7E7; color:#9B1C1C; border:1px solid #E5A5A5; border-radius:5px; padding:3px 8px; font-weight:700;");
    } else {
        stateBadge->setStyleSheet(
            "background:#E8EDFF; color:#303F9F; border:1px solid #B8C2F0; border-radius:5px; padding:3px 8px; font-weight:700;");
    }

    headerLayout->addWidget(stateBadge, 0, Qt::AlignLeft);
    contentLayout->addWidget(header);

    return page;
}

QWidget* ActivityDetailVisitor::createSection(const QString& title,
                                              QWidget* parent,
                                              QGridLayout*& gridLayout) const
{
    QFrame* section = new QFrame(parent);
    section->setObjectName("activityDetailSection");

    QVBoxLayout* sectionLayout = new QVBoxLayout(section);
    sectionLayout->setContentsMargins(12, 10, 12, 12);
    sectionLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(title, section);
    titleLabel->setObjectName("activityDetailSectionTitle");
    sectionLayout->addWidget(titleLabel);

    gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setHorizontalSpacing(16);
    gridLayout->setVerticalSpacing(8);
    gridLayout->setColumnStretch(1, 1);
    sectionLayout->addLayout(gridLayout);

    return section;
}

void ActivityDetailVisitor::addKeyValue(QGridLayout* layout,
                                        int row,
                                        const QString& key,
                                        const QString& value,
                                        QWidget* parent) const
{
    QLabel* keyLabel = new QLabel(key, parent);
    keyLabel->setObjectName("activityDetailKey");
    keyLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QLabel* valueLabel = new QLabel(value, parent);
    valueLabel->setObjectName("activityDetailValue");
    valueLabel->setWordWrap(true);
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    layout->addWidget(keyLabel, row, 0);
    layout->addWidget(valueLabel, row, 1);
}

void ActivityDetailVisitor::addCommonSections(const Activity& activity,
                                              QVBoxLayout* contentLayout,
                                              QWidget* page) const
{
    QGridLayout* overviewLayout = nullptr;
    QWidget* overviewSection = createSection("Overview", page, overviewLayout);

    addKeyValue(overviewLayout,
                0,
                "Category",
                activity.category().trimmed().isEmpty()
                    ? "No category"
                    : activity.category().trimmed(),
                page);
    addKeyValue(overviewLayout, 1, "Priority", priorityText(activity), page);
    addKeyValue(overviewLayout, 2, "Status", statusText(activity), page);
    addKeyValue(overviewLayout, 3, "Recurrence", recurrenceText(activity), page);

    if (activity.hasRecurrence()) {
        const QDateTime nextOccurrence = activity.nextOccurrenceAfter(QDateTime::currentDateTime());
        addKeyValue(overviewLayout,
                    4,
                    "Next occurrence",
                    nextOccurrence.isValid()
                        ? formattedDateTime(nextOccurrence)
                        : "No future occurrence",
                    page);
    }

    contentLayout->addWidget(overviewSection);

    if (!activity.description().trimmed().isEmpty()) {
        QGridLayout* descriptionLayout = nullptr;
        QWidget* descriptionSection = createSection("Description", page, descriptionLayout);

        QLabel* descriptionLabel = new QLabel(activity.description().trimmed(), page);
        descriptionLabel->setObjectName("activityDetailValue");
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        descriptionLayout->addWidget(descriptionLabel, 0, 0, 1, 2);

        contentLayout->addWidget(descriptionSection);
    }

    QGridLayout* metadataLayout = nullptr;
    QWidget* metadataSection = createSection("Activity history", page, metadataLayout);
    addKeyValue(metadataLayout, 0, "Created", formattedDateTime(activity.createdAt()), page);
    addKeyValue(metadataLayout, 1, "Updated", formattedDateTime(activity.updatedAt()), page);
    contentLayout->addWidget(metadataSection);
}

void ActivityDetailVisitor::storeResult(QWidget* widget)
{
    m_widget = widget;
}

QString ActivityDetailVisitor::priorityText(const Activity& activity) const
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

QString ActivityDetailVisitor::statusText(const Activity& activity) const
{
    if (activity.isCompleted()) {
        return "COMPLETED";
    }
    if (activity.isOverdue(QDateTime::currentDateTime())) {
        return "OVERDUE";
    }
    return "ACTIVE";
}

QString ActivityDetailVisitor::recurrenceText(const Activity& activity) const
{
    return activity.hasRecurrence() ? "Repeating" : "Does not repeat";
}

QString ActivityDetailVisitor::formattedDateTime(const QDateTime& dateTime) const
{
    return dateTime.isValid()
            ? dateTime.toString("ddd d MMM yyyy, HH:mm")
            : "Not set";
}

QString ActivityDetailVisitor::durationText(qint64 seconds) const
{
    if (seconds < 0) {
        return "Invalid duration";
    }

    const qint64 days = seconds / 86400;
    seconds %= 86400;
    const qint64 hours = seconds / 3600;
    const qint64 minutes = (seconds % 3600) / 60;

    QStringList parts;
    if (days > 0) {
        parts << QString("%1 d").arg(days);
    }
    if (hours > 0) {
        parts << QString("%1 h").arg(hours);
    }
    if (minutes > 0 || parts.isEmpty()) {
        parts << QString("%1 min").arg(minutes);
    }

    return parts.join(" ");
}

QString ActivityDetailVisitor::relativeTimeText(qint64 seconds) const
{
    if (seconds < 60) {
        return "less than a minute";
    }

    const qint64 days = seconds / 86400;
    const qint64 hours = (seconds % 86400) / 3600;
    const qint64 minutes = (seconds % 3600) / 60;

    QStringList parts;
    if (days > 0) {
        parts << QString("%1 day%2").arg(days).arg(days == 1 ? "" : "s");
    }
    if (hours > 0 && parts.size() < 2) {
        parts << QString("%1 hour%2").arg(hours).arg(hours == 1 ? "" : "s");
    }
    if (minutes > 0 && parts.size() < 2) {
        parts << QString("%1 minute%2").arg(minutes).arg(minutes == 1 ? "" : "s");
    }

    return parts.join(" ");
}
