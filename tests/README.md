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
QT_QPA_PLATFORM=offscreen ./agenda_tests -o -,txt
```

The `offscreen` platform keeps widget tests deterministic and prevents test windows from interrupting the desktop session. Remove `QT_QPA_PLATFORM=offscreen` when visually debugging a GUI test.

## Linux / Docker-style environment

```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-base-dev-tools
rm -rf build-tests
mkdir build-tests
cd build-tests
qmake6 ../tests/agenda_tests.pro
make -j2
QT_QPA_PLATFORM=offscreen ./agenda_tests -o -,txt
```

GitHub Actions runs the same Linux commands for every pull request and for pushes to `main` or a `feature/*` branch.

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
