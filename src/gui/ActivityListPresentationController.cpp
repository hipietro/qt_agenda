#include "ActivityListPresentationController.h"

#include "ActivityListItemVisitor.h"
#include "model/Activity.h"
#include "model/ActivityManager.h"

#include <QAbstractItemModel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSize>
#include <QStyle>
#include <QWidget>

#include <algorithm>

ActivityListPresentationController::ActivityListPresentationController(
    QListWidget* list,
    const ActivityManager* activityManager,
    QObject* parent)
    : QObject(parent),
      m_list(list),
      m_activityManager(activityManager)
{
    if (!m_list || !m_activityManager) {
        return;
    }

    connect(m_list->model(),
            &QAbstractItemModel::rowsInserted,
            this,
            [this](const QModelIndex&, int first, int last) {
                for (int row = first; row <= last; ++row) {
                    renderRow(row);
                }
            });

    connect(m_list,
            &QListWidget::currentItemChanged,
            this,
            [this](QListWidgetItem* current, QListWidgetItem* previous) {
                updateSelection(current, previous);
            });

    renderAll();
    updateSelection(m_list->currentItem(), nullptr);
}

void ActivityListPresentationController::renderAll()
{
    if (!m_list) {
        return;
    }

    for (int row = 0; row < m_list->count(); ++row) {
        renderRow(row);
    }
}

void ActivityListPresentationController::renderRow(int row)
{
    if (!m_list || !m_activityManager) {
        return;
    }

    QListWidgetItem* item = m_list->item(row);
    if (!item) {
        return;
    }

    const QString activityId = item->data(Qt::UserRole).toString();
    const Activity* activity = m_activityManager->findActivityById(activityId);
    if (!activity) {
        return;
    }

    ActivityListItemVisitor visitor(m_list);
    activity->accept(visitor);

    QWidget* card = visitor.takeWidget();
    if (!card) {
        return;
    }

    if (QWidget* previousCard = m_list->itemWidget(item)) {
        m_list->removeItemWidget(item);
        previousCard->deleteLater();
    }

    // The QListWidgetItem is only a container for the custom card.
    // Keeping its original text would paint it underneath the widget.
    item->setText(QString());
    item->setToolTip(visitor.toolTip());

    card->setProperty("selected", item == m_list->currentItem());
    card->ensurePolished();
    card->adjustSize();

    const int cardHeight = std::max(card->minimumHeight(), card->sizeHint().height());
    item->setSizeHint(QSize(0, cardHeight + 10));

    m_list->setItemWidget(item, card);
    setCardSelected(card, item == m_list->currentItem());
}

void ActivityListPresentationController::updateSelection(QListWidgetItem* current,
                                                          QListWidgetItem* previous)
{
    if (!m_list) {
        return;
    }

    if (previous) {
        setCardSelected(m_list->itemWidget(previous), false);
    }
    if (current) {
        setCardSelected(m_list->itemWidget(current), true);
    }
}

void ActivityListPresentationController::setCardSelected(QWidget* card, bool selected)
{
    if (!card) {
        return;
    }

    card->setProperty("selected", selected);

    const QList<QWidget*> widgets = card->findChildren<QWidget*>();
    card->style()->unpolish(card);
    card->style()->polish(card);

    for (QWidget* widget : widgets) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }

    card->update();
}
