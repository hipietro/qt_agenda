// Applies visitor-built cards to the main activity list and keeps selection readable.

#ifndef ACTIVITYLISTPRESENTATIONCONTROLLER_H
#define ACTIVITYLISTPRESENTATIONCONTROLLER_H

#include <QObject>

class ActivityManager;
class QListWidget;
class QListWidgetItem;
class QWidget;

class ActivityListPresentationController final : public QObject
{
public:
    ActivityListPresentationController(QListWidget* list,
                                       const ActivityManager* activityManager,
                                       QObject* parent = nullptr);

private:
    void renderAll();
    void renderRow(int row);
    void updateSelection(QListWidgetItem* current, QListWidgetItem* previous);
    void setCardSelected(QWidget* card, bool selected);

    QListWidget* m_list = nullptr;
    const ActivityManager* m_activityManager = nullptr;
};

#endif
