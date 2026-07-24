#!/usr/bin/env python3
"""Apply the in-window navigation foundation for issue 46.

The script uses guarded replacements and removes itself after a successful run.
"""

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


def patch_header() -> None:
    path = "src/gui/MainWindow.h"
    text = read(path)

    text = replace_once(
        text,
        "class QTextEdit;\nclass QAction;\n",
        "class QTextEdit;\nclass QStackedWidget;\nclass QWidget;\nclass QAction;\n",
        "MainWindow forward declarations",
    )

    text = replace_once(
        text,
        "    void setupUi();\n    void setupMenuBar();\n    void connectSignals();\n",
        "    void setupUi();\n    void setupMenuBar();\n    void connectSignals();\n\n"
        "    QWidget* createWorkflowPlaceholderPage(const QString& title,\n"
        "                                           const QString& description);\n"
        "    void openAgendaPage();\n"
        "    void openCreationPage();\n"
        "    void openEditingPage();\n"
        "    void showPage(QWidget* page);\n",
        "MainWindow navigation declarations",
    )

    text = replace_once(
        text,
        "    CommandHistory m_commandHistory;\n\n    QLineEdit* m_searchEdit = nullptr;\n",
        "    CommandHistory m_commandHistory;\n\n"
        "    QStackedWidget* m_pageStack = nullptr;\n"
        "    QWidget* m_agendaPage = nullptr;\n"
        "    QWidget* m_creationPage = nullptr;\n"
        "    QWidget* m_editingPage = nullptr;\n\n"
        "    QLineEdit* m_searchEdit = nullptr;\n",
        "MainWindow navigation members",
    )

    write(path, text)


def patch_source() -> None:
    path = "src/gui/MainWindow.cpp"
    text = read(path)

    text = replace_once(
        text,
        "#include <QStatusBar>\n#include <QStringList>\n",
        "#include <QStatusBar>\n#include <QStackedWidget>\n#include <QStringList>\n",
        "QStackedWidget include",
    )

    text = replace_once(
        text,
        """    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    QSplitter* splitter = new QSplitter(centralWidget);
""",
        """    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_pageStack = new QStackedWidget(centralWidget);
    m_pageStack->setObjectName("mainPageStack");

    m_agendaPage = new QWidget(m_pageStack);
    m_agendaPage->setObjectName("agendaPage");

    QVBoxLayout* mainLayout = new QVBoxLayout(m_agendaPage);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    QSplitter* splitter = new QSplitter(m_agendaPage);
""",
        "MainWindow stacked root",
    )

    text = replace_once(
        text,
        """    mainLayout->addWidget(splitter, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::setupMenuBar()
""",
        """    mainLayout->addWidget(splitter, 1);

    m_pageStack->addWidget(m_agendaPage);

    m_creationPage = createWorkflowPlaceholderPage(
        "Create activity",
        "The activity creation form will be hosted on this page. "
        "The dedicated form migration is implemented separately so the existing agenda remains stable."
    );
    m_creationPage->setObjectName("activityCreationPage");
    m_pageStack->addWidget(m_creationPage);

    m_editingPage = createWorkflowPlaceholderPage(
        "Edit activity",
        "The activity editing form will be hosted on this page. "
        "Keeping it inside the page stack preserves the agenda filters and selection while navigating."
    );
    m_editingPage->setObjectName("activityEditingPage");
    m_pageStack->addWidget(m_editingPage);

    rootLayout->addWidget(m_pageStack, 1);
    setCentralWidget(centralWidget);
    openAgendaPage();
}

QWidget* MainWindow::createWorkflowPlaceholderPage(const QString& title,
                                                   const QString& description)
{
    QWidget* page = new QWidget(m_pageStack);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(16);

    QLabel* titleLabel = new QLabel(title, page);
    titleLabel->setObjectName("pageTitle");

    QLabel* descriptionLabel = new QLabel(description, page);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QPushButton* backButton = new QPushButton("Back to agenda", page);
    backButton->setObjectName("primaryButton");

    connect(backButton, &QPushButton::clicked, this, [this]() {
        openAgendaPage();
    });

    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch(1);
    layout->addWidget(backButton, 0, Qt::AlignLeft);

    return page;
}

void MainWindow::openAgendaPage()
{
    showPage(m_agendaPage);
    updateActionButtons();
}

void MainWindow::openCreationPage()
{
    showPage(m_creationPage);
}

void MainWindow::openEditingPage()
{
    showPage(m_editingPage);
}

void MainWindow::showPage(QWidget* page)
{
    if (!m_pageStack || !page || m_pageStack->indexOf(page) < 0) {
        return;
    }

    m_pageStack->setCurrentWidget(page);
}

void MainWindow::setupMenuBar()
""",
        "MainWindow navigation implementation",
    )

    write(path, text)


def main() -> None:
    patch_header()
    patch_source()

    script_path = Path(__file__).resolve()
    script_path.unlink()
    print("Issue 46 navigation patch applied successfully.")


if __name__ == "__main__":
    main()
