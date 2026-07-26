#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def write(path, text):
    (ROOT / path).write_text(text, encoding="utf-8")


def replace_once(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected one match, found {count}")
    return text.replace(old, new, 1)


def patch_header(path, class_name):
    text = read(path)
    text = replace_once(text, "class QGroupBox;\n", "class QEvent;\nclass QGroupBox;\n", f"{class_name} QEvent declaration")
    marker = "private:\n    void setupUi();\n"
    replacement = "protected:\n    bool eventFilter(QObject* watched, QEvent* event) override;\n\nprivate:\n    void setupUi();\n    void installKeyboardHandling();\n"
    text = replace_once(text, marker, replacement, f"{class_name} keyboard declarations")
    write(path, text)


def patch_creation_cpp():
    path = "src/gui/ActivityCreationPage.cpp"
    text = read(path)
    text = replace_once(text, "#include <QDateTimeEdit>\n", "#include <QDateTimeEdit>\n#include <QEvent>\n", "creation QEvent include")
    text = replace_once(text, "#include <QMessageBox>\n", "#include <QKeyEvent>\n#include <QMessageBox>\n", "creation QKeyEvent include")
    text = replace_once(text, "    connectSignals();\n    resetForm();\n", "    connectSignals();\n    installKeyboardHandling();\n    resetForm();\n", "creation install keyboard")
    text = replace_once(
        text,
        "    m_buttonBox->button(QDialogButtonBox::Ok)->setText(\"Create\");\n    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(\"Cancel\");\n",
        "    m_buttonBox->button(QDialogButtonBox::Ok)->setText(\"Create\");\n    m_buttonBox->button(QDialogButtonBox::Ok)->setToolTip(\"Create activity (Enter)\");\n    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(\"Cancel\");\n    m_buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(\"Cancel creation (Esc)\");\n",
        "creation button hints",
    )
    insert_before = "void ActivityCreationPage::updateTypePage()\n"
    keyboard_code = '''bool ActivityCreationPage::eventFilter(QObject* watched, QEvent* event)\n{\n    if (event->type() != QEvent::KeyPress) {\n        return QWidget::eventFilter(watched, event);\n    }\n\n    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);\n\n    if (keyEvent->key() == Qt::Key_Escape) {\n        if (m_cancelHandler) {\n            m_cancelHandler();\n        }\n        return true;\n    }\n\n    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {\n        const bool multilineEditor = qobject_cast<QTextEdit*>(watched) != nullptr;\n        const bool explicitSubmit = keyEvent->modifiers().testFlag(Qt::ControlModifier) ||\n                                    keyEvent->modifiers().testFlag(Qt::MetaModifier);\n\n        if (!multilineEditor || explicitSubmit) {\n            submit();\n            return true;\n        }\n    }\n\n    return QWidget::eventFilter(watched, event);\n}\n\nvoid ActivityCreationPage::installKeyboardHandling()\n{\n    installEventFilter(this);\n\n    const QList<QWidget*> childWidgets = findChildren<QWidget*>();\n    for (QWidget* widget : childWidgets) {\n        widget->installEventFilter(this);\n    }\n}\n\n'''
    text = replace_once(text, insert_before, keyboard_code + insert_before, "creation keyboard implementation")
    write(path, text)


def patch_edit_cpp():
    path = "src/gui/ActivityEditPage.cpp"
    text = read(path)
    text = replace_once(text, "#include <QDateTimeEdit>\n", "#include <QDateTimeEdit>\n#include <QEvent>\n", "edit QEvent include")
    text = replace_once(text, "#include <QMessageBox>\n", "#include <QKeyEvent>\n#include <QMessageBox>\n", "edit QKeyEvent include")
    text = replace_once(text, "#include <QScrollArea>\n", "#include <QScrollArea>\n#include <QSizePolicy>\n", "edit QSizePolicy include")
    text = replace_once(text, "    setupUi();\n}\n", "    setupUi();\n    installKeyboardHandling();\n}\n", "edit install keyboard")
    text = replace_once(
        text,
        "    QGroupBox* specificGroup = new QGroupBox(\"Type-specific fields\", contentWidget);\n",
        "    QGroupBox* specificGroup = new QGroupBox(\"Type-specific fields\", contentWidget);\n    specificGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);\n",
        "compact type-specific group",
    )
    text = replace_once(
        text,
        "    m_typeStack = new QStackedWidget(this);\n",
        "    m_typeStack = new QStackedWidget(specificGroup);\n    m_typeStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);\n",
        "compact type stack",
    )
    text = replace_once(
        text,
        "    m_buttonBox->button(QDialogButtonBox::Ok)->setText(\"Save changes\");\n    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(\"Cancel\");\n",
        "    m_buttonBox->button(QDialogButtonBox::Ok)->setText(\"Save changes\");\n    m_buttonBox->button(QDialogButtonBox::Ok)->setToolTip(\"Save changes (Enter)\");\n    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(\"Cancel\");\n    m_buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(\"Cancel editing (Esc)\");\n",
        "edit button hints",
    )
    insert_before = "void ActivityEditPage::populateFromActivity(const Activity& activity)\n"
    keyboard_code = '''bool ActivityEditPage::eventFilter(QObject* watched, QEvent* event)\n{\n    if (event->type() != QEvent::KeyPress) {\n        return QWidget::eventFilter(watched, event);\n    }\n\n    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);\n\n    if (keyEvent->key() == Qt::Key_Escape) {\n        if (m_cancelHandler) {\n            m_cancelHandler();\n        }\n        return true;\n    }\n\n    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {\n        const bool multilineEditor = qobject_cast<QTextEdit*>(watched) != nullptr;\n        const bool explicitSubmit = keyEvent->modifiers().testFlag(Qt::ControlModifier) ||\n                                    keyEvent->modifiers().testFlag(Qt::MetaModifier);\n\n        if (!multilineEditor || explicitSubmit) {\n            submit();\n            return true;\n        }\n    }\n\n    return QWidget::eventFilter(watched, event);\n}\n\nvoid ActivityEditPage::installKeyboardHandling()\n{\n    installEventFilter(this);\n\n    const QList<QWidget*> childWidgets = findChildren<QWidget*>();\n    for (QWidget* widget : childWidgets) {\n        widget->installEventFilter(this);\n    }\n}\n\n'''
    text = replace_once(text, insert_before, keyboard_code + insert_before, "edit keyboard implementation")
    write(path, text)


def patch_main_window():
    path = "src/gui/MainWindow.cpp"
    text = read(path)
    text = replace_once(text, "#include <QStatusBar>\n", "#include <QStatusBar>\n#include <QStyle>\n", "QStyle include")
    marker = "    const bool workflowActive = m_workspaceStack &&\n        m_detailPage &&\n        m_workspaceStack->currentWidget() != m_detailPage;\n\n"
    replacement = marker + '''    const bool editingActive = m_workspaceStack &&\n        m_editingPage &&\n        m_workspaceStack->currentWidget() == m_editingPage;\n\n    auto applyButtonRole = [](QPushButton* button, const QString& role) {\n        if (!button || button->objectName() == role) {\n            return;\n        }\n\n        button->setObjectName(role);\n        button->style()->unpolish(button);\n        button->style()->polish(button);\n        button->update();\n    };\n\n    applyButtonRole(m_addButton, editingActive ? \"primaryButton\" : \"accentButton\");\n    applyButtonRole(m_editButton, editingActive ? \"accentButton\" : \"primaryButton\");\n\n'''
    text = replace_once(text, marker, replacement, "workflow button styling")
    write(path, text)


patch_header("src/gui/ActivityCreationPage.h", "creation page")
patch_header("src/gui/ActivityEditPage.h", "edit page")
patch_creation_cpp()
patch_edit_cpp()
patch_main_window()
print("Issue 48 UI refinements applied")
