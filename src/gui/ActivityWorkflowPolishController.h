// Cross-cutting presentation refinements for the in-window activity workflows.

#ifndef ACTIVITYWORKFLOWPOLISHCONTROLLER_H
#define ACTIVITYWORKFLOWPOLISHCONTROLLER_H

#include <QObject>
#include <QPointer>

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

    QPointer<QMainWindow> m_window;
    QPointer<QStackedWidget> m_workspaceStack;
    QPointer<QWidget> m_creationPage;
    QPointer<QWidget> m_editingPage;
    QPointer<QPushButton> m_addButton;
    QPointer<QPushButton> m_editButton;
    QPointer<QLineEdit> m_newChecklistItemEdit;
    QPointer<QListWidget> m_editChecklistList;
};

#endif
