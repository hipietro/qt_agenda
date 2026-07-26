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


# ActivityCreationPage.h
path = "src/gui/ActivityCreationPage.h"
text = read(path)
text = replace_once(
    text,
    "class QCheckBox;\n",
    "class QCheckBox;\nclass QEvent;\n",
    "creation QEvent forward declaration",
)
text = replace_once(
    text,
    "    void resetForm();\n\nprivate:\n",
    "    void resetForm();\n\nprotected:\n"
    "    bool eventFilter(QObject* watched, QEvent* event) override;\n\n"
    "private:\n",
    "creation event filter declaration",
)
text = replace_once(
    text,
    "    void updateTypePage();\n    void submit();\n",
    "    void updateTypePage();\n"
    "    void updateTypeStackHeight();\n"
    "    void submit();\n"
    "    void cancel();\n",
    "creation helpers",
)
write(path, text)


# ActivityCreationPage.cpp
path = "src/gui/ActivityCreationPage.cpp"
text = read(path)
text = replace_once(
    text,
    "#include <QAbstractItemView>\n",
    "#include <QAbstractItemView>\n#include <QApplication>\n",
    "creation QApplication include",
)
text = replace_once(
    text,
    "#include <QDialogButtonBox>\n",
    "#include <QDialogButtonBox>\n#include <QEvent>\n",
    "creation QEvent include",
)
text = replace_once(
    text,
    "#include <QLabel>\n",
    "#include <QKeyEvent>\n#include <QLabel>\n#include <QLayout>\n",
    "creation key/layout includes",
)
text = replace_once(
    text,
    "    setupUi();\n    connectSignals();\n    resetForm();\n",
    "    setupUi();\n"
    "    connectSignals();\n"
    "    qApp->installEventFilter(this);\n"
    "    resetForm();\n",
    "creation global event filter install",
)
text = replace_once(
    text,
    "void ActivityCreationPage::setCancelHandler(CancelHandler handler)\n"
    "{\n"
    "    m_cancelHandler = std::move(handler);\n"
    "}\n\n",
    "void ActivityCreationPage::setCancelHandler(CancelHandler handler)\n"
    "{\n"
    "    m_cancelHandler = std::move(handler);\n"
    "}\n\n"
    "bool ActivityCreationPage::eventFilter(QObject* watched, QEvent* event)\n"
    "{\n"
    "    if (!isVisible() || !window() || !window()->isActiveWindow() ||\n"
    "        event->type() != QEvent::KeyPress) {\n"
    "        return QWidget::eventFilter(watched, event);\n"
    "    }\n\n"
    "    if (QApplication::activePopupWidget()) {\n"
    "        return QWidget::eventFilter(watched, event);\n"
    "    }\n\n"
    "    const auto* keyEvent = static_cast<QKeyEvent*>(event);\n"
    "    QWidget* focusWidget = QApplication::focusWidget();\n\n"
    "    const auto isInside = [](QWidget* widget, const QWidget* container) {\n"
    "        while (widget) {\n"
    "            if (widget == container) {\n"
    "                return true;\n"
    "            }\n"
    "            widget = widget->parentWidget();\n"
    "        }\n"
    "        return false;\n"
    "    };\n\n"
    "    if (keyEvent->key() == Qt::Key_Escape) {\n"
    "        cancel();\n"
    "        return true;\n"
    "    }\n\n"
    "    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {\n"
    "        const bool forcedSubmit =\n"
    "            keyEvent->modifiers().testFlag(Qt::ControlModifier) ||\n"
    "            keyEvent->modifiers().testFlag(Qt::MetaModifier);\n\n"
    "        if (!forcedSubmit &&\n"
    "            (isInside(focusWidget, m_descriptionEdit) ||\n"
    "             isInside(focusWidget, m_checklistItemsEdit))) {\n"
    "            return QWidget::eventFilter(watched, event);\n"
    "        }\n\n"
    "        submit();\n"
    "        return true;\n"
    "    }\n\n"
    "    return QWidget::eventFilter(watched, event);\n"
    "}\n\n",
    "creation keyboard behavior",
)
text = replace_once(
    text,
    "    QGroupBox* specificGroup = new QGroupBox(\"Type-specific fields\", contentWidget);\n"
    "    QVBoxLayout* specificLayout = new QVBoxLayout(specificGroup);\n",
    "    QGroupBox* specificGroup = new QGroupBox(\"Type-specific fields\", contentWidget);\n"
    "    specificGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);\n"
    "    QVBoxLayout* specificLayout = new QVBoxLayout(specificGroup);\n",
    "creation compact specific group",
)
text = replace_once(
    text,
    "    m_typeStack = new QStackedWidget(specificGroup);\n"
    "    m_typeStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);\n",
    "    m_typeStack = new QStackedWidget(specificGroup);\n"
    "    m_typeStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);\n",
    "creation compact type stack",
)
text = replace_once(
    text,
    "    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, contentWidget);\n"
    "    m_buttonBox->button(QDialogButtonBox::Ok)->setText(\"Create\");\n"
    "    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(\"Cancel\");\n",
    "    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, contentWidget);\n"
    "    QPushButton* createButton = m_buttonBox->button(QDialogButtonBox::Ok);\n"
    "    QPushButton* cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel);\n"
    "    createButton->setText(\"Create\");\n"
    "    cancelButton->setText(\"Cancel\");\n"
    "    createButton->setToolTip(\"Create activity (Enter; Ctrl/Cmd+Enter in multi-line fields)\");\n"
    "    createButton->setStatusTip(createButton->toolTip());\n"
    "    cancelButton->setToolTip(\"Cancel and return to activity details (Esc)\");\n"
    "    cancelButton->setStatusTip(cancelButton->toolTip());\n",
    "creation shortcut tooltips",
)
text = replace_once(
    text,
    "    updateRecurrenceControls();\n}\n\nvoid ActivityCreationPage::connectSignals()",
    "    updateTypePage();\n"
    "    updateRecurrenceControls();\n"
    "}\n\n"
    "void ActivityCreationPage::connectSignals()",
    "creation initial stack height",
)
text = replace_once(
    text,
    "    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {\n"
    "        if (m_cancelHandler) {\n"
    "            m_cancelHandler();\n"
    "        }\n"
    "    });\n",
    "    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {\n"
    "        cancel();\n"
    "    });\n",
    "creation cancel handler reuse",
)
text = replace_once(
    text,
    "    m_typeStack->setCurrentIndex(m_typeCombo->currentIndex());\n"
    "}\n\n"
    "bool ActivityCreationPage::validateForm() const",
    "    m_typeStack->setCurrentIndex(m_typeCombo->currentIndex());\n"
    "    updateTypeStackHeight();\n"
    "}\n\n"
    "void ActivityCreationPage::updateTypeStackHeight()\n"
    "{\n"
    "    if (!m_typeStack || !m_typeStack->currentWidget()) {\n"
    "        return;\n"
    "    }\n\n"
    "    QWidget* currentPage = m_typeStack->currentWidget();\n"
    "    if (currentPage->layout()) {\n"
    "        currentPage->layout()->activate();\n"
    "    }\n\n"
    "    m_typeStack->setFixedHeight(std::max(96, currentPage->sizeHint().height() + 8));\n"
    "}\n\n"
    "void ActivityCreationPage::cancel()\n"
    "{\n"
    "    if (m_cancelHandler) {\n"
    "        m_cancelHandler();\n"
    "    }\n"
    "}\n\n"
    "bool ActivityCreationPage::validateForm() const",
    "creation stack height and cancel methods",
)
write(path, text)


# ActivityEditPage.h
path = "src/gui/ActivityEditPage.h"
text = read(path)
text = replace_once(
    text,
    "class QCheckBox;\n",
    "class QCheckBox;\nclass QEvent;\n",
    "edit QEvent forward declaration",
)
text = replace_once(
    text,
    "    void clearActivity();\n\nprivate:\n",
    "    void clearActivity();\n\nprotected:\n"
    "    bool eventFilter(QObject* watched, QEvent* event) override;\n\n"
    "private:\n",
    "edit event filter declaration",
)
text = replace_once(
    text,
    "    void populateFromActivity(const Activity& activity);\n"
    "    void submit();\n",
    "    void populateFromActivity(const Activity& activity);\n"
    "    void updateTypeStackHeight();\n"
    "    void submit();\n"
    "    void cancel();\n",
    "edit helpers",
)
write(path, text)


# ActivityEditPage.cpp
path = "src/gui/ActivityEditPage.cpp"
text = read(path)
text = replace_once(
    text,
    "#include <QAbstractItemView>\n",
    "#include <QAbstractItemView>\n#include <QApplication>\n",
    "edit QApplication include",
)
text = replace_once(
    text,
    "#include <QDialogButtonBox>\n",
    "#include <QDialogButtonBox>\n#include <QEvent>\n",
    "edit QEvent include",
)
text = replace_once(
    text,
    "#include <QLabel>\n",
    "#include <QKeyEvent>\n#include <QLabel>\n#include <QLayout>\n",
    "edit key/layout includes",
)
text = replace_once(
    text,
    "#include <QScrollArea>\n",
    "#include <QScrollArea>\n#include <QSizePolicy>\n",
    "edit size policy include",
)
text = replace_once(
    text,
    "    setupUi();\n    clearActivity();\n",
    "    setupUi();\n"
    "    qApp->installEventFilter(this);\n"
    "    clearActivity();\n",
    "edit global event filter install",
)
text = replace_once(
    text,
    "void ActivityEditPage::setCancelHandler(CancelHandler handler)\n"
    "{\n"
    "    m_cancelHandler = std::move(handler);\n"
    "}\n\n",
    "void ActivityEditPage::setCancelHandler(CancelHandler handler)\n"
    "{\n"
    "    m_cancelHandler = std::move(handler);\n"
    "}\n\n"
    "bool ActivityEditPage::eventFilter(QObject* watched, QEvent* event)\n"
    "{\n"
    "    if (!isVisible() || !window() || !window()->isActiveWindow() ||\n"
    "        event->type() != QEvent::KeyPress) {\n"
    "        return QWidget::eventFilter(watched, event);\n"
    "    }\n\n"
    "    if (QApplication::activePopupWidget()) {\n"
    "        return QWidget::eventFilter(watched, event);\n"
    "    }\n\n"
    "    const auto* keyEvent = static_cast<QKeyEvent*>(event);\n"
    "    QWidget* focusWidget = QApplication::focusWidget();\n\n"
    "    const auto isInside = [](QWidget* widget, const QWidget* container) {\n"
    "        while (widget) {\n"
    "            if (widget == container) {\n"
    "                return true;\n"
    "            }\n"
    "            widget = widget->parentWidget();\n"
    "        }\n"
    "        return false;\n"
    "    };\n\n"
    "    if (keyEvent->key() == Qt::Key_Escape) {\n"
    "        const bool inlineChecklistEditorActive =\n"
    "            focusWidget && focusWidget != m_checklistItemsList &&\n"
    "            isInside(focusWidget, m_checklistItemsList);\n\n"
    "        if (inlineChecklistEditorActive) {\n"
    "            return QWidget::eventFilter(watched, event);\n"
    "        }\n\n"
    "        cancel();\n"
    "        return true;\n"
    "    }\n\n"
    "    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {\n"
    "        const bool forcedSubmit =\n"
    "            keyEvent->modifiers().testFlag(Qt::ControlModifier) ||\n"
    "            keyEvent->modifiers().testFlag(Qt::MetaModifier);\n\n"
    "        if (!forcedSubmit &&\n"
    "            (isInside(focusWidget, m_descriptionEdit) ||\n"
    "             isInside(focusWidget, m_checklistItemsList) ||\n"
    "             focusWidget == m_checklistNewItemEdit)) {\n"
    "            return QWidget::eventFilter(watched, event);\n"
    "        }\n\n"
    "        submit();\n"
    "        return true;\n"
    "    }\n\n"
    "    return QWidget::eventFilter(watched, event);\n"
    "}\n\n",
    "edit keyboard behavior",
)
text = replace_once(
    text,
    "    QGroupBox* specificGroup = new QGroupBox(\"Type-specific fields\", contentWidget);\n"
    "    QVBoxLayout* specificLayout = new QVBoxLayout(specificGroup);\n",
    "    QGroupBox* specificGroup = new QGroupBox(\"Type-specific fields\", contentWidget);\n"
    "    specificGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);\n"
    "    QVBoxLayout* specificLayout = new QVBoxLayout(specificGroup);\n",
    "edit compact specific group",
)
text = replace_once(
    text,
    "    m_typeStack = new QStackedWidget(this);\n",
    "    m_typeStack = new QStackedWidget(specificGroup);\n"
    "    m_typeStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);\n",
    "edit compact type stack",
)
text = replace_once(
    text,
    "    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);\n"
    "    m_buttonBox->button(QDialogButtonBox::Ok)->setText(\"Save changes\");\n"
    "    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(\"Cancel\");\n",
    "    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);\n"
    "    QPushButton* saveButton = m_buttonBox->button(QDialogButtonBox::Ok);\n"
    "    QPushButton* cancelButton = m_buttonBox->button(QDialogButtonBox::Cancel);\n"
    "    saveButton->setText(\"Save changes\");\n"
    "    cancelButton->setText(\"Cancel\");\n"
    "    saveButton->setToolTip(\"Save changes (Enter; Ctrl/Cmd+Enter in multi-line fields)\");\n"
    "    saveButton->setStatusTip(saveButton->toolTip());\n"
    "    cancelButton->setToolTip(\"Cancel and return to activity details (Esc)\");\n"
    "    cancelButton->setStatusTip(cancelButton->toolTip());\n",
    "edit shortcut tooltips",
)
text = replace_once(
    text,
    "    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {\n"
    "        if (m_cancelHandler) {\n"
    "            m_cancelHandler();\n"
    "        }\n"
    "        clearActivity();\n"
    "    });\n",
    "    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {\n"
    "        cancel();\n"
    "    });\n",
    "edit cancel handler reuse",
)
text = replace_once(
    text,
    "    connect(m_addChecklistItemButton, &QPushButton::clicked, this, [this]() {\n",
    "    connect(m_checklistNewItemEdit, &QLineEdit::returnPressed,\n"
    "            m_addChecklistItemButton, &QPushButton::click);\n\n"
    "    connect(m_addChecklistItemButton, &QPushButton::clicked, this, [this]() {\n",
    "edit checklist Enter behavior",
)
text = replace_once(
    text,
    "    mainLayout->addWidget(m_buttonBox);\n\n"
    "    updateRecurrenceControls();\n"
    "}\n\n"
    "void ActivityEditPage::populateFromActivity",
    "    mainLayout->addWidget(m_buttonBox);\n\n"
    "    updateTypeStackHeight();\n"
    "    updateRecurrenceControls();\n"
    "}\n\n"
    "void ActivityEditPage::populateFromActivity",
    "edit initial stack height",
)
text = replace_once(
    text,
    "    ActivityEditFormVisitor visitor(*this, ActivityEditFormVisitor::Operation::Populate);\n"
    "    activity.accept(visitor);\n"
    "    populateRecurrence(activity);\n"
    "}\n\n"
    "bool ActivityEditPage::validateForm() const",
    "    ActivityEditFormVisitor visitor(*this, ActivityEditFormVisitor::Operation::Populate);\n"
    "    activity.accept(visitor);\n"
    "    updateTypeStackHeight();\n"
    "    populateRecurrence(activity);\n"
    "}\n\n"
    "void ActivityEditPage::updateTypeStackHeight()\n"
    "{\n"
    "    if (!m_typeStack || !m_typeStack->currentWidget()) {\n"
    "        return;\n"
    "    }\n\n"
    "    QWidget* currentPage = m_typeStack->currentWidget();\n"
    "    if (currentPage->layout()) {\n"
    "        currentPage->layout()->activate();\n"
    "    }\n\n"
    "    m_typeStack->setFixedHeight(std::max(96, currentPage->sizeHint().height() + 8));\n"
    "}\n\n"
    "void ActivityEditPage::cancel()\n"
    "{\n"
    "    if (m_cancelHandler) {\n"
    "        m_cancelHandler();\n"
    "    }\n"
    "    clearActivity();\n"
    "}\n\n"
    "bool ActivityEditPage::validateForm() const",
    "edit stack height and cancel methods",
)
write(path, text)


# MainWindow.cpp
path = "src/gui/MainWindow.cpp"
text = read(path)
text = replace_once(
    text,
    "#include <QStatusBar>\n",
    "#include <QStatusBar>\n#include <QStyle>\n",
    "MainWindow QStyle include",
)
text = replace_once(
    text,
    "    const bool workflowActive = m_workspaceStack &&\n"
    "        m_detailPage &&\n"
    "        m_workspaceStack->currentWidget() != m_detailPage;\n\n"
    "    if (m_addButton) {\n",
    "    const bool workflowActive = m_workspaceStack &&\n"
    "        m_detailPage &&\n"
    "        m_workspaceStack->currentWidget() != m_detailPage;\n"
    "    const bool creationActive = m_workspaceStack &&\n"
    "        m_creationPage &&\n"
    "        m_workspaceStack->currentWidget() == m_creationPage;\n"
    "    const bool editingActive = m_workspaceStack &&\n"
    "        m_editingPage &&\n"
    "        m_workspaceStack->currentWidget() == m_editingPage;\n\n"
    "    const auto setWorkflowButtonState = [](QPushButton* button, bool active) {\n"
    "        if (!button || button->property(\"workflowActive\").toBool() == active) {\n"
    "            return;\n"
    "        }\n\n"
    "        button->setProperty(\"workflowActive\", active);\n"
    "        if (button->style()) {\n"
    "            button->style()->unpolish(button);\n"
    "            button->style()->polish(button);\n"
    "        }\n"
    "        button->update();\n"
    "    };\n\n"
    "    setWorkflowButtonState(m_addButton, creationActive);\n"
    "    setWorkflowButtonState(m_editButton, editingActive);\n\n"
    "    if (m_addButton) {\n",
    "MainWindow active workflow button styling",
)
write(path, text)


# resources/style.qss
path = "resources/style.qss"
text = read(path)
text = replace_once(
    text,
    "QPushButton#primaryButton:disabled,\n"
    "QPushButton#dangerButton:disabled {\n",
    "QPushButton#accentButton:disabled,\n"
    "QPushButton#primaryButton:disabled,\n"
    "QPushButton#dangerButton:disabled {\n",
    "accent disabled button styling",
)
text = replace_once(
    text,
    "QPushButton#primaryButton:disabled,\n"
    "QPushButton#dangerButton:disabled {\n"
    "    background-color: #e0e0e0;\n"
    "    color: #888888;\n"
    "    border: 1px solid #cfcfcf;\n"
    "}\n\n",
    "QPushButton#accentButton:disabled,\n"
    "QPushButton#primaryButton:disabled,\n"
    "QPushButton#dangerButton:disabled {\n"
    "    background-color: #e0e0e0;\n"
    "    color: #888888;\n"
    "    border: 1px solid #cfcfcf;\n"
    "}\n\n"
    "QPushButton#accentButton[workflowActive=\"true\"],\n"
    "QPushButton#accentButton[workflowActive=\"true\"]:disabled,\n"
    "QPushButton#primaryButton[workflowActive=\"true\"],\n"
    "QPushButton#primaryButton[workflowActive=\"true\"]:disabled {\n"
    "    background-color: #3F51B5;\n"
    "    color: #ffffff;\n"
    "    border: 1px solid #303F9F;\n"
    "    font-weight: 600;\n"
    "}\n\n",
    "active workflow button styling",
)
write(path, text)

print("Issue 48 UI and keyboard refinements applied.")
