// Cross-cutting presentation refinements for the in-window activity workflows.

#ifndef ACTIVITYWORKFLOWPOLISHCONTROLLER_H
#define ACTIVITYWORKFLOWPOLISHCONTROLLER_H

#include <QObject>

class QDialogButtonBox;
class QEvent;
class QGroupBox;
class QLineEdit;
class QListWidget;
class QMainWindow;
class QPushButton;
class QStackedWidget;
class QWidget;

class ActivityWorkflowPolishController final : public QObject
{
public:
    explicit ActivityWorkflowPolishController(QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void configureWindow(QMainWindow* window);
    void configureFormPage(QWidget* page, bool editingPage);
    void compactTypeSpecificSection(QWidget* page);
    void updateWorkflowButtonState();

    QDialogButtonBox* buttonBoxFor(QWidget* page) const;
    QPushButton* buttonWithText(QWidget* root, const QString& text) const;
    QGroupBox* typeSpecificGroup(QWidget* page) const;
    QStackedWidget* typeStack(QWidget* page) const;
    bool isInside(QWidget* widget, const QWidget* container) const;

    QMainWindow* m_window = nullptr;
    QStackedWidget* m_workspaceStack = nullptr;
    QWidget* m_creationPage = nullptr;
    QWidget* m_editingPage = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_editButton = nullptr;
    QLineEdit* m_newChecklistItemEdit = nullptr;
    QListWidget* m_editChecklistList = nullptr;
};

#endif
