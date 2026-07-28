#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

failures=0

pass_check() {
    printf 'PASS: %s\n' "$1"
}

fail_check() {
    printf 'FAIL: %s\n' "$1" >&2
    failures=$((failures + 1))
}

require_file() {
    local path="$1"
    local description="$2"

    if [[ -f "$path" ]]; then
        pass_check "$description"
    else
        fail_check "$description (missing: $path)"
    fi
}

require_pattern() {
    local pattern="$1"
    local path="$2"
    local description="$3"

    if grep -Eq "$pattern" "$path"; then
        pass_check "$description"
    else
        fail_check "$description"
    fi
}

printf 'Running static mandatory-requirement audit...\n\n'

require_file "agenda_qt.pro" "qmake project file is present"
require_file "Dockerfile" "course Docker environment is present"
require_file "tests/AgendaTests.cpp" "core regression suite is present"
require_file "tests/GuiInteractionTests.cpp" "GUI regression suite is present"
require_file "examples/sample_agenda.json" "structured example data is present"

unexpected_source_files="$(find src -type f ! -name '*.h' ! -name '*.cpp' -print)"
if [[ -z "$unexpected_source_files" ]]; then
    pass_check "all implementation files under src are C++ headers or sources"
else
    printf '%s\n' "$unexpected_source_files" >&2
    fail_check "non-C++ implementation files found under src"
fi

widget_include_pattern='#include[[:space:]]*[<"](QApplication|QWidget|QMainWindow|QDialog|QPushButton|QLabel|QLineEdit|QTextEdit|QListWidget|QStackedWidget|QFileDialog|QMessageBox|QMenu|QAction)'
model_widget_matches="$(grep -ERn --include='*.h' "$widget_include_pattern" src/model || true)"
if [[ -z "$model_widget_matches" ]]; then
    pass_check "model headers do not include Qt Widgets classes"
else
    printf '%s\n' "$model_widget_matches" >&2
    fail_check "Qt Widgets dependency found in a model header"
fi

for activity in EventActivity DeadlineActivity ReminderActivity ChecklistActivity; do
    require_file "src/model/${activity}.h" "${activity} concrete model class is present"
done

require_pattern 'virtual[[:space:]]+void[[:space:]]+accept\(ActivityVisitor&' \
    "src/model/Activity.h" \
    "Activity exposes Visitor double dispatch"
require_pattern 'class[[:space:]]+ActivityListItemVisitor[[:space:]]+final[[:space:]]*:[[:space:]]*public[[:space:]]+ActivityVisitor' \
    "src/gui/ActivityListItemVisitor.h" \
    "list rendering uses a concrete visitor"
require_pattern 'class[[:space:]]+ActivityDetailVisitor[[:space:]]+final[[:space:]]*:[[:space:]]*public[[:space:]]+ActivityVisitor' \
    "src/gui/ActivityDetailVisitor.h" \
    "detail rendering uses a concrete visitor"
require_pattern 'class[[:space:]]+ActivityEditFormVisitor[[:space:]]+final[[:space:]]*:[[:space:]]*public[[:space:]]+ActivityVisitor' \
    "src/gui/ActivityEditFormVisitor.h" \
    "type-specific edit behavior uses a concrete visitor"
require_pattern 'class[[:space:]]+ActivityJsonSerializationVisitor[[:space:]]+final[[:space:]]*:[[:space:]]*public[[:space:]]+ActivityVisitor' \
    "src/persistence/ActivityJsonSerializationVisitor.h" \
    "JSON serialization uses a concrete visitor"

require_pattern 'virtual[[:space:]]+bool[[:space:]]+execute\(\)[[:space:]]*=[[:space:]]*0' \
    "src/commands/Command.h" \
    "Command hierarchy has dynamic execute behavior"
require_pattern 'virtual[[:space:]]+bool[[:space:]]+undo\(\)[[:space:]]*=[[:space:]]*0' \
    "src/commands/Command.h" \
    "Command hierarchy has dynamic undo behavior"

obsolete_dialog_matches="$(grep -ERn 'Activity(Creation|Edit)Dialog' src agenda_qt.pro || true)"
if [[ -z "$obsolete_dialog_matches" ]]; then
    pass_check "no obsolete creation/edit dialog remains reachable"
else
    printf '%s\n' "$obsolete_dialog_matches" >&2
    fail_check "obsolete creation/edit dialog reference found"
fi

kind_dispatch_matches="$({
    grep -ERn --include='*.h' --include='*.cpp' \
        'switch[[:space:]]*\([^)]*kind[[:space:]]*\(\)' src || true
    grep -ERn --include='*.h' --include='*.cpp' \
        '(if|else[[:space:]]+if)[[:space:]]*\([^)]*kind[[:space:]]*\(\)[[:space:]]*(==|!=)' src || true
} | grep -v '^src/model/ActivityFilter.cpp:' || true)"
if [[ -z "$kind_dispatch_matches" ]]; then
    pass_check "no concrete behavior is dispatched from Activity::kind()"
else
    printf '%s\n' "$kind_dispatch_matches" >&2
    fail_check "unexplained Activity::kind()-based behavior dispatch found"
fi

require_pattern 'm_workspaceStack->addWidget\(m_creationPage\)' \
    "src/gui/MainWindow.cpp" \
    "creation page is hosted inside MainWindow"
require_pattern 'm_workspaceStack->addWidget\(m_editingPage\)' \
    "src/gui/MainWindow.cpp" \
    "editing page is hosted inside MainWindow"
require_pattern 'QFileDialog::getSaveFileName' \
    "src/gui/MainWindow.cpp" \
    "runtime Save As uses a graphical file dialog"
require_pattern 'QFileDialog::getOpenFileName' \
    "src/gui/MainWindow.cpp" \
    "runtime Load uses a graphical file dialog"
require_pattern 'AgendaJsonStorage::saveToFile' \
    "src/gui/MainWindow.cpp" \
    "GUI save action reaches structured persistence"
require_pattern 'AgendaJsonStorage::loadFromFile' \
    "src/gui/MainWindow.cpp" \
    "GUI load action reaches structured persistence"

printf '\n'
if (( failures > 0 )); then
    printf 'Static audit failed with %d problem(s).\n' "$failures" >&2
    exit 1
fi

printf 'Static audit passed. Manual evidence still required for authorship, the clean Docker build, the final PDF report, and the final smoke test.\n'
