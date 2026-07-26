#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def write(path: str, content: str) -> None:
    (ROOT / path).write_text(content, encoding="utf-8")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected one match, found {count}")
    return text.replace(old, new, 1)


# Fix small portability/type details in the new widget implementation.
path = "src/gui/ActivityMonthOverviewWidget.cpp"
text = read(path)
text = replace_once(
    text,
    "#include <QDateTime>\n" if "#include <QDateTime>\n" in text else "#include <QAbstractButton>\n",
    "#include <QAbstractButton>\n#include <QCursor>\n" if "#include <QDateTime>\n" not in text else "#include <QDateTime>\n#include <QCursor>\n",
    "month widget QCursor include",
)
text = text.replace(
    "setCursor(valid ? Qt::PointingHandCursor : Qt::ArrowCursor);",
    "setCursor(QCursor(valid ? Qt::PointingHandCursor : Qt::ArrowCursor));",
)
text = text.replace(
    "const int markerCount = std::min(4, m_markerColors.size());",
    "const int markerCount = std::min(4, static_cast<int>(m_markerColors.size()));",
)
text = text.replace(
    "const int visibleTitles = std::min(5, summary.titles.size());",
    "const int visibleTitles = std::min(5, static_cast<int>(summary.titles.size()));",
)
write(path, text)


# Integrate the month overview and exact-date filtering into MainWindow.
path = "src/gui/MainWindow.cpp"
text = read(path)
text = replace_once(
    text,
    '#include "ActivityEditPage.h"\n',
    '#include "ActivityEditPage.h"\n#include "ActivityMonthOverviewWidget.h"\n',
    "MainWindow month overview include",
)
text = replace_once(
    text,
    "#include <QScrollArea>\n",
    "#include <QScrollArea>\n#include <QSignalBlocker>\n",
    "MainWindow QSignalBlocker include",
)
text = replace_once(
    text,
    "#include <QTextEdit>\n",
    "#include <QTextEdit>\n#include <QTime>\n",
    "MainWindow QTime include",
)

filter_marker = '''    filterGridLayout->addWidget(createFilterCell("Due state", m_overdueCombo), 2, 0);
    filterGridLayout->addWidget(createFilterCell("Sort by", m_sortCombo), 2, 1);
    filterGridLayout->setColumnStretch(0, 1);
'''
filter_replacement = '''    filterGridLayout->addWidget(createFilterCell("Due state", m_overdueCombo), 2, 0);
    filterGridLayout->addWidget(createFilterCell("Sort by", m_sortCombo), 2, 1);

    m_clearFiltersButton = new QPushButton("Clear filters", filterPanel);
    m_clearFiltersButton->setObjectName("primaryButton");
    m_clearFiltersButton->setToolTip(
        "Reset type, priority, category, status, recurrence, due-state and date filters");
    filterGridLayout->addWidget(m_clearFiltersButton, 3, 0, 1, 2);

    filterGridLayout->setColumnStretch(0, 1);
'''
text = replace_once(text, filter_marker, filter_replacement, "clear filters button")

old_detail = '''    QVBoxLayout* rightLayout = new QVBoxLayout(m_detailPage);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    QLabel* detailLabel = new QLabel("Activity details", m_detailPage);
    detailLabel->setObjectName("sectionLabel");

    m_detailView = new QTextEdit(m_detailPage);
    m_detailView->setObjectName("activityDetailView");
    m_detailView->setReadOnly(true);

    rightLayout->addWidget(detailLabel);
    rightLayout->addWidget(m_detailView, 1);

    m_workspaceStack->addWidget(m_detailPage);
'''
new_detail = '''    QVBoxLayout* rightLayout = new QVBoxLayout(m_detailPage);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QSplitter* detailSplitter = new QSplitter(Qt::Vertical, m_detailPage);
    detailSplitter->setObjectName("detailOverviewSplitter");
    detailSplitter->setChildrenCollapsible(false);

    QWidget* detailContainer = new QWidget(detailSplitter);
    QVBoxLayout* detailLayout = new QVBoxLayout(detailContainer);
    detailLayout->setContentsMargins(0, 0, 0, 0);
    detailLayout->setSpacing(8);

    QLabel* detailLabel = new QLabel("Activity details", detailContainer);
    detailLabel->setObjectName("sectionLabel");

    m_detailView = new QTextEdit(detailContainer);
    m_detailView->setObjectName("activityDetailView");
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(135);

    detailLayout->addWidget(detailLabel);
    detailLayout->addWidget(m_detailView, 1);

    QWidget* overviewContainer = new QWidget(detailSplitter);
    QVBoxLayout* overviewLayout = new QVBoxLayout(overviewContainer);
    overviewLayout->setContentsMargins(0, 0, 0, 0);
    overviewLayout->setSpacing(6);

    QLabel* overviewLabel = new QLabel("Monthly overview", overviewContainer);
    overviewLabel->setObjectName("sectionLabel");

    m_monthOverview = new ActivityMonthOverviewWidget(overviewContainer);
    m_monthOverview->setDateSelectedHandler([this](const QDate& date) {
        selectDateFilter(date);
    });

    overviewLayout->addWidget(overviewLabel);
    overviewLayout->addWidget(m_monthOverview, 1);

    detailSplitter->addWidget(detailContainer);
    detailSplitter->addWidget(overviewContainer);
    detailSplitter->setStretchFactor(0, 3);
    detailSplitter->setStretchFactor(1, 2);
    detailSplitter->setSizes({360, 320});

    rightLayout->addWidget(detailSplitter, 1);

    m_workspaceStack->addWidget(m_detailPage);
'''
text = replace_once(text, old_detail, new_detail, "detail and calendar layout")

connect_marker = '''    connect(m_sortCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_activityList, &QListWidget::currentRowChanged, this, [this](int currentRow) {
'''
connect_replacement = '''    connect(m_sortCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshActivityList();
    });

    connect(m_clearFiltersButton, &QPushButton::clicked, this, [this]() {
        clearFilters();
    });

    connect(m_activityList, &QListWidget::currentRowChanged, this, [this](int currentRow) {
'''
text = replace_once(text, connect_marker, connect_replacement, "clear filters connection")

refresh_start = '''void MainWindow::refreshActivityList()
{
    const QString previousSelectedId = selectedActivityId();

    updateCategoryFilterOptions();
'''
refresh_replacement = '''void MainWindow::refreshActivityList()
{
    const QString previousSelectedId = selectedActivityId();

    updateCategoryFilterOptions();
    updateMonthOverview();
'''
text = replace_once(text, refresh_start, refresh_replacement, "month overview refresh")

result_label = '''    const QString query = m_searchEdit->text().trimmed();
    const QString sortText = m_sortCombo ? m_sortCombo->currentText() : "Default";

    if (query.isEmpty()) {
        m_resultCountLabel->setText(QString("Activities shown: %1 | Sort: %2")
                                    .arg(visibleActivities.size())
                                    .arg(sortText));
    } else {
        m_resultCountLabel->setText(QString("Search results: %1 | Query: \\"%2\\" | Sort: %3")
                                    .arg(visibleActivities.size())
                                    .arg(query)
                                    .arg(sortText));
    }
'''
result_replacement = '''    const QString query = m_searchEdit->text().trimmed();
    const QString sortText = m_sortCombo ? m_sortCombo->currentText() : "Default";
    const QString dateSuffix = m_selectedDateFilter.has_value()
        ? QString(" | Date: %1").arg(m_selectedDateFilter->toString("dd MMM yyyy"))
        : QString();

    if (query.isEmpty()) {
        m_resultCountLabel->setText(QString("Activities shown: %1 | Sort: %2%3")
                                    .arg(visibleActivities.size())
                                    .arg(sortText)
                                    .arg(dateSuffix));
    } else {
        m_resultCountLabel->setText(QString("Search results: %1 | Query: \\"%2\\" | Sort: %3%4")
                                    .arg(visibleActivities.size())
                                    .arg(query)
                                    .arg(sortText)
                                    .arg(dateSuffix));
    }
'''
text = replace_once(text, result_label, result_replacement, "date filter result label")

empty_message = '''        if (query.isEmpty()) {
            m_detailView->setPlainText(
                "No activities are available.\\n\\n"
                "Use Add activity or From template to create a new activity."
            );
        } else {
'''
empty_replacement = '''        if (query.isEmpty()) {
            if (m_selectedDateFilter.has_value()) {
                m_detailView->setPlainText(
                    QString("No activities are scheduled for %1.\\n\\n"
                            "Choose another day or use Clear filters.")
                        .arg(m_selectedDateFilter->toString("dd MMMM yyyy"))
                );
            } else {
                m_detailView->setPlainText(
                    "No activities are available.\\n\\n"
                    "Use Add activity or From template to create a new activity."
                );
            }
        } else {
'''
text = replace_once(text, empty_message, empty_replacement, "date-specific empty state")

method_marker = '''void MainWindow::updateActionButtons()
{
'''
new_methods = '''void MainWindow::updateMonthOverview()
{
    if (!m_monthOverview || !m_activityManager) {
        return;
    }

    m_monthOverview->setActivities(m_activityManager->activities());
    m_monthOverview->setSelectedDate(m_selectedDateFilter);
}

void MainWindow::clearFilters()
{
    const auto resetCombo = [](QComboBox* comboBox) {
        if (!comboBox) {
            return;
        }

        const QSignalBlocker blocker(comboBox);
        comboBox->setCurrentIndex(0);
    };

    resetCombo(m_typeCombo);
    resetCombo(m_priorityCombo);
    resetCombo(m_categoryCombo);
    resetCombo(m_completionCombo);
    resetCombo(m_recurrenceCombo);
    resetCombo(m_overdueCombo);

    m_selectedDateFilter.reset();
    if (m_monthOverview) {
        m_monthOverview->clearSelectedDate();
    }

    refreshActivityList();
}

void MainWindow::selectDateFilter(const QDate& date)
{
    if (!date.isValid()) {
        return;
    }

    m_selectedDateFilter = date;
    if (m_monthOverview) {
        m_monthOverview->setSelectedDate(m_selectedDateFilter);
    }

    refreshActivityList();
}

void MainWindow::updateActionButtons()
{
'''
text = replace_once(text, method_marker, new_methods, "month overview methods")

criteria_marker = '''    if (m_overdueCombo) {
        const int overdueValue = m_overdueCombo->currentData().toInt();

        if (overdueValue == 1) {
            criteria.overdue = ActivityFilter::OverdueFilter::OverdueOnly;
        } else if (overdueValue == 2) {
            criteria.overdue = ActivityFilter::OverdueFilter::NotOverdueOnly;
        }
    }

    const QString sortValue = m_sortCombo
'''
criteria_replacement = '''    if (m_overdueCombo) {
        const int overdueValue = m_overdueCombo->currentData().toInt();

        if (overdueValue == 1) {
            criteria.overdue = ActivityFilter::OverdueFilter::OverdueOnly;
        } else if (overdueValue == 2) {
            criteria.overdue = ActivityFilter::OverdueFilter::NotOverdueOnly;
        }
    }

    if (m_selectedDateFilter.has_value()) {
        criteria.fromDate = QDateTime(m_selectedDateFilter.value(), QTime(0, 0, 0, 0));
        criteria.toDate = QDateTime(m_selectedDateFilter.value(), QTime(23, 59, 59, 999));
    }

    const QString sortValue = m_sortCombo
'''
text = replace_once(text, criteria_marker, criteria_replacement, "exact date criteria")

write(path, text)


# Add the new component to qmake.
path = "agenda_qt.pro"
text = read(path)
text = replace_once(
    text,
    "    src/gui/ActivityListMouseController.cpp \\\n",
    "    src/gui/ActivityListMouseController.cpp \\\n    src/gui/ActivityMonthOverviewWidget.cpp \\\n",
    "qmake month overview source",
)
text = replace_once(
    text,
    "    src/gui/ActivityListMouseController.h \\\n",
    "    src/gui/ActivityListMouseController.h \\\n    src/gui/ActivityMonthOverviewWidget.h \\\n",
    "qmake month overview header",
)
write(path, text)

print("Issue 65 monthly overview integration applied.")
