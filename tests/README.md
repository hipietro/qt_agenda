# Test suite

The project uses Qt Test in a separate qmake target so the production executable and the tests remain independent.

## macOS with the project Qt installation

From the repository root:

```bash
rm -rf build-tests
mkdir build-tests
cd build-tests
~/Qt/6.10.1/macos/bin/qmake ../tests/agenda_tests.pro
make -j"$(sysctl -n hw.ncpu)"
QTEST_FUNCTION_TIMEOUT=15000 ./agenda_tests -o -,txt
```

The GUI interaction test must use the normal macOS platform plugin. Qt's `offscreen` plugin does not support native popup menus reliably and can leave a contextual-menu test waiting indefinitely. A small test window may appear briefly while the suite runs.

`QTEST_FUNCTION_TIMEOUT=15000` limits each test function to 15 seconds, so a GUI regression cannot block the terminal for several minutes.

## Linux / Docker-style environment

```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-base-dev-tools xvfb
rm -rf build-tests
mkdir build-tests
cd build-tests
qmake6 ../tests/agenda_tests.pro
make -j2
QTEST_FUNCTION_TIMEOUT=15000 xvfb-run -a ./agenda_tests -o -,txt
```

GitHub Actions runs the same Linux commands for every pull request and for pushes to `main` or a `feature/*` branch. Xvfb provides a real virtual display, which is required for popup-menu interaction tests.

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
- internal creation/editing pages and the main list's single-click, double-click, contextual-menu, and empty-space behavior.

No test uses a user-specific or hard-coded filesystem path. Temporary agenda files are created with `QTemporaryDir`.
