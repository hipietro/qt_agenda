# Test suite

The project uses Qt Test in separate qmake targets so model/architecture regressions and native GUI interactions can be executed independently.

## macOS with the project Qt installation

From the repository root:

```bash
rm -rf build-tests
mkdir -p build-tests/core build-tests/gui

cd build-tests/core
~/Qt/6.10.1/macos/bin/qmake ../../tests/agenda_tests.pro
make -j"$(sysctl -n hw.ncpu)"
QTEST_FUNCTION_TIMEOUT=15000 ./agenda_tests -o -,txt

cd ../gui
~/Qt/6.10.1/macos/bin/qmake ../../tests/gui_interaction_tests.pro
make -j"$(sysctl -n hw.ncpu)"
QTEST_FUNCTION_TIMEOUT=15000 ./gui_interaction_tests -o -,txt
```

The GUI target uses the normal macOS platform plugin. A small test window or contextual menu may appear briefly. A 1.5-second safety close inside the contextual-menu test prevents a native popup from blocking the terminal indefinitely.

Do not set `QT_QPA_PLATFORM=offscreen` for the GUI target on macOS. The offscreen plugin does not provide the native popup behavior exercised by the contextual-menu regression test.

## Linux / Docker-style environment

```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-base-dev-tools xvfb
rm -rf build-tests
mkdir -p build-tests/core build-tests/gui

cd build-tests/core
qmake6 ../../tests/agenda_tests.pro
make -j2
QTEST_FUNCTION_TIMEOUT=15000 xvfb-run -a ./agenda_tests -o -,txt

cd ../gui
qmake6 ../../tests/gui_interaction_tests.pro
make -j2
QTEST_FUNCTION_TIMEOUT=15000 xvfb-run -a ./gui_interaction_tests -o -,txt
```

GitHub Actions builds and runs both targets for pull requests and pushes to `main` or a `feature/*` branch. Xvfb provides a real virtual display for both targets on Linux because the core suite also verifies QWidget-based Visitor renderers.

## Coverage

The automated suite verifies:

- Visitor double dispatch for Event, Deadline, Reminder, and Checklist activities;
- type-specific list-card and detail-page rendering through visitors;
- JSON serialization and factory-based deserialization for all activity types;
- malformed and unknown JSON rejection;
- full agenda save/load round trips, including categories, templates, recurrence, completion state, participants, and checklist items;
- add, edit, delete, and completion commands across repeated undo/redo cycles;
- redo-branch truncation after a new command;
- filtering, sorting, and searching regressions;
- internal creation/editing pages and the main list's single-click, double-click, contextual-menu action, and empty-space behavior.

No test uses a user-specific or hard-coded filesystem path. Temporary agenda files are created with `QTemporaryDir`.
