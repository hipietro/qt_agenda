#!/usr/bin/env python3
"""Move creation/edit workflows into a responsive right-side workspace.

Run after apply_issue_47_patch.py and fix_issue_47_widget_lookup.py.
The script performs guarded transformations and removes itself after success.
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def write(relative: str, content: str) -> None:
    (ROOT / relative).write_text(content, encoding="utf-8")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def regex_once(text: str, pattern: str, replacement: str, label: str) -> str:
    updated, count = re.subn(pattern, replacement, text, count=1, flags=re.S)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return updated


def patch_header() -> None:
    path = "src/gui/MainWindow.h"
    text = read(path)

    text = replace_once(
        text,
        "class QPushButton;\nclass QTextEdit;\nclass QStackedWidget;\n",
        "class QPushButton;\nclass QResizeEvent;\nclass QScrollArea;\nclass QSplitter;\nclass QTextEdit;\nclass QStackedWidget;\n",
        "responsive widget forward declarations",
    )

    text = replace_once(
        text,
        "protected:\n    void closeEvent(QCloseEvent* event) override;\n",
        "protected:\n    void closeEvent(QCloseEvent* event) override;\n"
        "    void resizeEvent(QResizeEvent* event) override;\n",
        "resize event declaration",
    )

    text = replace_once(
        text,
        "    void openEditingPage();\n    void showPage(QWidget* page);\n",
        "    void openEditingPage();\n"
        "    void showPage(QWidget* page);\n"
        "    void updateResponsiveWorkspace();\n",
        "responsive navigation declaration",
    )

    text = replace_once(
        text,
        "    QStackedWidget* m_pageStack = nullptr;\n"
        "    QWidget* m_agendaPage = nullptr;\n"
        "    ActivityCreationPage* m_creationPage = nullptr;\n"
        "    QWidget* m_editingPage = nullptr;\n",
        "    QStackedWidget* m_pageStack = nullptr;\n"
        "    QWidget* m_agendaPage = nullptr;\n"
        "    QSplitter* m_mainSplitter = nullptr;\n"
        "    QScrollArea* m_leftScrollArea = nullptr;\n"
        "    QStackedWidget* m_workspaceStack = nullptr;\n"
        "    QWidget* m_detailPage = nullptr;\n"
        "    ActivityCreationPage* m_creationPage = nullptr;\n"
        "    QWidget* m_editingPage = nullptr;\n",
        "responsive workspace members",
    )

    write(path, text)


def patch_source() -> None:
    path = "src/gui/MainWindow.cpp"
    text = read(path)

    text = replace_once(
        text,
        "#include <QPushButton>\n#include <QScrollArea>\n",
        "#include <QPushButton>\n#include <QResizeEvent>\n#include <QScrollArea>\n",
        "QResizeEvent include",
    )

    text = replace_once(
        text,
        "void MainWindow::closeEvent(QCloseEvent* event)\n"
        "{\n"
        "    if (confirmDiscardUnsavedChanges()) {\n"
        "        event->accept();\n"
        "    } else {\n"
        "        event->ignore();\n"
        "    }\n"
        "}\n\n",
        "void MainWindow::closeEvent(QCloseEvent* event)\n"
        "{\n"
        "    if (confirmDiscardUnsavedChanges()) {\n"
        "        event->accept();\n"
        "    } else {\n"
        "        event->ignore();\n"
        "    }\n"
        "}\n\n"
        "void MainWindow::resizeEvent(QResizeEvent* event)\n"
        "{\n"
        "    QMainWindow::resizeEvent(event);\n"
        "    updateResponsiveWorkspace();\n"
        "}\n\n",
        "resize event implementation",
    )

    text = replace_once(
        text,
        "    QSplitter* splitter = new QSplitter(m_agendaPage);\n"
        "    splitter->setChildrenCollapsible(false);\n",
        "    m_mainSplitter = new QSplitter(m_agendaPage);\n"
        "    m_mainSplitter->setChildrenCollapsible(false);\n",
        "main splitter member",
    )

    text = replace_once(
        text,
        "    QScrollArea* leftScrollArea = new QScrollArea(splitter);\n"
        "    leftScrollArea->setWidgetResizable(true);\n"
        "    leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);\n"
        "    leftScrollArea->setFrameShape(QFrame::NoFrame);\n"
        "    leftScrollArea->setMinimumWidth(360);\n",
        "    m_leftScrollArea = new QScrollArea(m_mainSplitter);\n"
        "    m_leftScrollArea->setWidgetResizable(true);\n"
        "    m_leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);\n"
        "    m_leftScrollArea->setFrameShape(QFrame::NoFrame);\n"
        "    m_leftScrollArea->setMinimumWidth(320);\n",
        "left workspace panel member",
    )

    text = text.replace("new QWidget(leftScrollArea)", "new QWidget(m_leftScrollArea)")

    right_workspace = r'''    m_workspaceStack = new QStackedWidget(m_mainSplitter);
    m_workspaceStack->setObjectName("workspaceStack");
    m_workspaceStack->setMinimumWidth(280);

    m_detailPage = new QWidget(m_workspaceStack);
    m_detailPage->setObjectName("activityDetailPage");

    QVBoxLayout* rightLayout = new QVBoxLayout(m_detailPage);
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

    m_leftScrollArea->setWidget(leftPanel);

    m_mainSplitter->addWidget(m_leftScrollArea);
    m_mainSplitter->addWidget(m_workspaceStack);
    m_mainSplitter->setStretchFactor(0, 12);
    m_mainSplitter->setStretchFactor(1, 8);
    m_mainSplitter->setSizes({590, 360});
'''

    text = regex_once(
        text,
        r"    QWidget\* rightPanel = new QWidget\(splitter\);.*?"
        r"    splitter->setSizes\(\{590, 360\}\);\n",
        right_workspace,
        "right-side workspace stack",
    )

    text = replace_once(
        text,
        "    mainLayout->addWidget(splitter, 1);\n",
        "    mainLayout->addWidget(m_mainSplitter, 1);\n",
        "main splitter layout",
    )

    workspace_pages = r'''    m_pageStack->addWidget(m_agendaPage);

    m_creationPage = new ActivityCreationPage(m_categoryManager, m_workspaceStack);
    m_creationPage->setCreatedHandler([this](std::unique_ptr<Activity> activity) {
        addCreatedActivity(std::move(activity));
    });
    m_creationPage->setCancelHandler([this]() {
        openAgendaPage();
    });
    m_workspaceStack->addWidget(m_creationPage);

    m_editingPage = createWorkflowPlaceholderPage(
        "Edit activity",
        "The activity editing form will replace this workspace in issue 48. "
        "The agenda remains visible on wide windows and collapses only when space is limited."
    );
    m_editingPage->setObjectName("activityEditingPage");
    m_workspaceStack->addWidget(m_editingPage);
'''

    text = regex_once(
        text,
        r"    m_pageStack->addWidget\(m_agendaPage\);\n\n"
        r"    m_creationPage = new ActivityCreationPage\(m_categoryManager, m_pageStack\);.*?"
        r"    m_pageStack->addWidget\(m_editingPage\);\n",
        workspace_pages,
        "move workflow pages into right workspace",
    )

    text = replace_once(
        text,
        "    QWidget* page = new QWidget(m_pageStack);\n",
        "    QWidget* page = new QWidget(m_workspaceStack);\n",
        "placeholder workspace parent",
    )

    navigation = r'''void MainWindow::openAgendaPage()
{
    showPage(m_detailPage);
}

void MainWindow::openCreationPage()
{
    if (!m_creationPage) {
        return;
    }

    synchronizeCategoryManagerFromActivities();
    m_creationPage->resetForm();
    showPage(m_creationPage);
}

void MainWindow::openEditingPage()
{
    showPage(m_editingPage);
}

void MainWindow::showPage(QWidget* page)
{
    if (!m_workspaceStack || !page || m_workspaceStack->indexOf(page) < 0) {
        return;
    }

    m_workspaceStack->setCurrentWidget(page);
    updateResponsiveWorkspace();
    updateActionButtons();
}

void MainWindow::updateResponsiveWorkspace()
{
    if (!m_mainSplitter || !m_leftScrollArea || !m_workspaceStack || !m_detailPage) {
        return;
    }

    const bool workflowActive = m_workspaceStack->currentWidget() != m_detailPage;
    const bool compactWindow = width() < 1050;
    const bool showAgendaPanel = !(workflowActive && compactWindow);

    m_leftScrollArea->setVisible(showAgendaPanel);

    const int totalWidth = std::max(1, m_mainSplitter->width());

    if (!showAgendaPanel) {
        m_mainSplitter->setSizes({0, totalWidth});
        return;
    }

    int leftWidth = workflowActive
        ? totalWidth * 32 / 100
        : totalWidth * 60 / 100;

    if (workflowActive) {
        leftWidth = std::max(320, std::min(leftWidth, 420));
    } else {
        leftWidth = std::max(360, std::min(leftWidth, 650));
    }

    m_mainSplitter->setSizes({leftWidth, std::max(1, totalWidth - leftWidth)});
}

void MainWindow::setupMenuBar()'''

    text = regex_once(
        text,
        r"void MainWindow::openAgendaPage\(\)\n\{.*?\n\}\n\n"
        r"void MainWindow::setupMenuBar\(\)",
        navigation,
        "responsive workspace navigation",
    )

    text = replace_once(
        text,
        "void MainWindow::updateActionButtons()\n"
        "{\n"
        "    if (m_addButton) {\n"
        "        m_addButton->setEnabled(m_activityManager != nullptr);\n"
        "    }\n",
        "void MainWindow::updateActionButtons()\n"
        "{\n"
        "    const bool workflowActive = m_workspaceStack &&\n"
        "        m_detailPage &&\n"
        "        m_workspaceStack->currentWidget() != m_detailPage;\n\n"
        "    if (m_addButton) {\n"
        "        m_addButton->setEnabled(m_activityManager != nullptr && !workflowActive);\n"
        "    }\n",
        "workflow action guard",
    )

    text = replace_once(
        text,
        "                                     !m_templateManager->isEmpty());\n",
        "                                     !m_templateManager->isEmpty() &&\n"
        "                                     !workflowActive);\n",
        "template action guard",
    )

    text = replace_once(
        text,
        "    const bool hasSelection = activity != nullptr;\n",
        "    const bool hasSelection = activity != nullptr && !workflowActive;\n",
        "selection action guard",
    )

    text = text.replace(
        "m_commandHistory.canUndo());",
        "!workflowActive && m_commandHistory.canUndo());",
    )
    text = text.replace(
        "m_commandHistory.canRedo());",
        "!workflowActive && m_commandHistory.canRedo());",
    )

    write(path, text)


def audit() -> None:
    header = read("src/gui/MainWindow.h")
    source = read("src/gui/MainWindow.cpp")

    required = [
        "m_workspaceStack",
        "m_detailPage",
        "updateResponsiveWorkspace",
        "resizeEvent(QResizeEvent* event)",
    ]

    missing = [token for token in required if token not in header and token not in source]
    if missing:
        raise RuntimeError("adaptive workspace tokens missing: " + ", ".join(missing))

    if "new ActivityCreationPage(m_categoryManager, m_pageStack)" in source:
        raise RuntimeError("creation page is still attached to the root page stack")


def main() -> None:
    patch_header()
    patch_source()
    audit()

    Path(__file__).resolve().unlink()
    print("Issue 47 adaptive right-side workspace applied successfully.")


if __name__ == "__main__":
    main()
