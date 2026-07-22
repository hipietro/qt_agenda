// Replaces the legacy text view with a safe, scrollable visitor-rendered detail panel.

#ifndef ACTIVITYDETAILPRESENTATIONCONTROLLER_H
#define ACTIVITYDETAILPRESENTATIONCONTROLLER_H

#include <QObject>

class ActivityManager;
class QListWidget;
class QListWidgetItem;
class QScrollArea;
class QTextEdit;
class QWidget;

class ActivityDetailPresentationController final : public QObject
{
public:
    ActivityDetailPresentationController(QListWidget* activityList,
                                         QTextEdit* legacyDetailView,
                                         const ActivityManager* activityManager,
                                         QObject* parent = nullptr);

private:
    void renderCurrentActivity();
    void renderItem(QListWidgetItem* item);
    void showEmptyState();
    void setPage(QWidget* page);

    QListWidget* m_activityList = nullptr;
    QTextEdit* m_legacyDetailView = nullptr;
    const ActivityManager* m_activityManager = nullptr;
    QScrollArea* m_scrollArea = nullptr;
};

#endif
