#include "ActivityDetailPresentationController.h"

#include "ActivityDetailVisitor.h"
#include "model/Activity.h"
#include "model/ActivityManager.h"

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

ActivityDetailPresentationController::ActivityDetailPresentationController(
    QListWidget* activityList,
    QTextEdit* legacyDetailView,
    const ActivityManager* activityManager,
    QObject* parent)
    : QObject(parent),
      m_activityList(activityList),
      m_legacyDetailView(legacyDetailView),
      m_activityManager(activityManager)
{
    if (!m_activityList || !m_legacyDetailView || !m_activityManager) {
        return;
    }

    QWidget* detailParent = m_legacyDetailView->parentWidget();
    if (!detailParent || !detailParent->layout()) {
        return;
    }

    m_scrollArea = new QScrollArea(detailParent);
    m_scrollArea->setObjectName("activityDetailScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea#activityDetailScrollArea { background:#FFFFFF; border:1px solid #D0D0D0; border-radius:6px; }"
        "QScrollArea#activityDetailScrollArea > QWidget > QWidget { background:#FFFFFF; }");

    detailParent->layout()->replaceWidget(m_legacyDetailView, m_scrollArea);
    m_legacyDetailView->hide();

    connect(m_activityList,
            &QListWidget::currentItemChanged,
            this,
            [this](QListWidgetItem* current, QListWidgetItem*) {
                renderItem(current);
            });

    renderCurrentActivity();
}

void ActivityDetailPresentationController::renderCurrentActivity()
{
    renderItem(m_activityList ? m_activityList->currentItem() : nullptr);
}

void ActivityDetailPresentationController::renderItem(QListWidgetItem* item)
{
    if (!m_scrollArea || !m_activityManager || !item) {
        showEmptyState();
        return;
    }

    const QString activityId = item->data(Qt::UserRole).toString();
    const Activity* activity = m_activityManager->findActivityById(activityId);

    if (!activity) {
        showEmptyState();
        return;
    }

    ActivityDetailVisitor visitor(m_scrollArea);
    activity->accept(visitor);

    QWidget* page = visitor.takeWidget();
    if (!page) {
        showEmptyState();
        return;
    }

    setPage(page);
}

void ActivityDetailPresentationController::showEmptyState()
{
    if (!m_scrollArea) {
        return;
    }

    QWidget* page = new QWidget(m_scrollArea);
    page->setObjectName("activityDetailEmptyPage");
    page->setStyleSheet(
        "QWidget#activityDetailEmptyPage { background:#FFFFFF; }"
        "QLabel#activityDetailEmptyTitle { color:#333333; font-size:17px; font-weight:700; }"
        "QLabel#activityDetailEmptyText { color:#707070; font-size:13px; }");

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(8);
    layout->addStretch();

    QLabel* title = new QLabel("No activity selected", page);
    title->setObjectName("activityDetailEmptyTitle");
    title->setAlignment(Qt::AlignCenter);

    QLabel* text = new QLabel(
        "Select an activity from the list to view its information.", page);
    text->setObjectName("activityDetailEmptyText");
    text->setAlignment(Qt::AlignCenter);
    text->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(text);
    layout->addStretch();

    setPage(page);
}

void ActivityDetailPresentationController::setPage(QWidget* page)
{
    if (!m_scrollArea || !page) {
        return;
    }

    if (QWidget* previousPage = m_scrollArea->takeWidget()) {
        previousPage->deleteLater();
    }

    m_scrollArea->setWidget(page);
}
