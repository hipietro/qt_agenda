// Applies non-invasive visual refinements to the main window after widget construction.

#ifndef VISUALPOLISHCONTROLLER_H
#define VISUALPOLISHCONTROLLER_H

#include <QObject>

class QEvent;
class QMainWindow;
class QPushButton;
class QString;
class QWidget;

class VisualPolishController final : public QObject
{
public:
    explicit VisualPolishController(QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void configureWindow(QMainWindow* window);
    void configureSplitters(QMainWindow* window) const;
    void configureActionButtons(QMainWindow* window) const;
    void configureCalendarButtons(QMainWindow* window) const;
    void configureSearchField(QMainWindow* window) const;
    void configureMenuIcons(QMainWindow* window) const;

    QPushButton* buttonWithText(QWidget* root, const QString& text) const;
    QPushButton* buttonStartingWith(QWidget* root, const QString& prefix) const;

    QMainWindow* m_window = nullptr;
};

#endif
