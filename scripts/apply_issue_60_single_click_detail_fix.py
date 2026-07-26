#!/usr/bin/env python3
from pathlib import Path

path = Path("src/gui/ActivityListMouseController.cpp")
text = path.read_text(encoding="utf-8")

old = '''    connect(m_activityList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
                editItem(item);
            });

    connect(m_activityList, &QWidget::customContextMenuRequested,
'''

new = '''    connect(m_activityList, &QListWidget::itemClicked,
            this, [this](QListWidgetItem* item) {
                if (!item || creationActive()) {
                    return;
                }

                m_activityList->setCurrentItem(item);
                item->setSelected(true);

                /*
                 * A normal click always means "show this activity". If editing is
                 * open, cancel only the editing workflow and return to the details
                 * page for the item that Qt has already selected.
                 */
                if (editingActive()) {
                    leaveEditingWorkflow();
                }
            });

    connect(m_activityList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
                editItem(item);
            });

    connect(m_activityList, &QWidget::customContextMenuRequested,
'''

if text.count(old) != 1:
    raise RuntimeError(f"Expected exactly one interaction block, found {text.count(old)}")

path.write_text(text.replace(old, new, 1), encoding="utf-8")
print("Single-click detail navigation added.")
