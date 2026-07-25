#!/usr/bin/env python3
"""Fix detail rendering being routed into the creation-page description field.

The application previously discovered the list and detail widgets by type only.
After introducing ActivityCreationPage, multiple QTextEdit instances exist, so the
lookup must use stable object names. The script removes itself after success.
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


def patch_main_window() -> None:
    path = "src/gui/MainWindow.cpp"
    text = read(path)

    text = replace_once(
        text,
        "    m_activityList = new QListWidget(leftPanel);\n"
        "    m_activityList->setSpacing(4);\n",
        "    m_activityList = new QListWidget(leftPanel);\n"
        "    m_activityList->setObjectName(\"activityList\");\n"
        "    m_activityList->setSpacing(4);\n",
        "activity list object name",
    )

    text = replace_once(
        text,
        "    m_detailView = new QTextEdit(rightPanel);\n"
        "    m_detailView->setReadOnly(true);\n",
        "    m_detailView = new QTextEdit(rightPanel);\n"
        "    m_detailView->setObjectName(\"activityDetailView\");\n"
        "    m_detailView->setReadOnly(true);\n",
        "activity detail object name",
    )

    write(path, text)


def patch_main() -> None:
    path = "src/main.cpp"
    text = read(path)

    text = replace_once(
        text,
        "    QListWidget* activityList = window.findChild<QListWidget*>();\n"
        "    QTextEdit* legacyDetailView = window.findChild<QTextEdit*>();\n",
        "    QListWidget* activityList =\n"
        "        window.findChild<QListWidget*>(\"activityList\");\n"
        "    QTextEdit* activityDetailView =\n"
        "        window.findChild<QTextEdit*>(\"activityDetailView\");\n",
        "named presentation widget lookup",
    )

    text = replace_once(
        text,
        "        legacyDetailView,\n",
        "        activityDetailView,\n",
        "detail controller widget",
    )

    write(path, text)


def main() -> None:
    patch_main_window()
    patch_main()

    Path(__file__).resolve().unlink()
    print("Issue 47 widget lookup fix applied successfully.")


if __name__ == "__main__":
    main()
