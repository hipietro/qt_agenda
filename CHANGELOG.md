# Changelog

All notable repository releases are documented in this file.

## [Unreleased]

- complete source-release installation guide;
- persistent Qt PATH configuration for macOS;
- clarified source-only distribution and platform validation;
- documentation comments added to previously uncommented source files.

## [1.0.0] - 2026-07-28

### Added

- polymorphic events, deadlines, reminders, and checklists;
- Visitor-built list cards, detail pages, edit forms, and JSON serialization;
- internal creation and editing workflows inside `MainWindow`;
- JSON persistence with factory-based reconstruction;
- categories, templates, recurrence, monthly overview, filters, sorting, and fuzzy search;
- undo/redo through an abstract Command hierarchy;
- keyboard shortcuts and contextual mouse/trackpad interactions;
- SVG icons, resizable splitters, and responsive visual polish;
- core, GUI, architecture, and persistence regression tests;
- Linux clean-build validation through the supplied course Docker image.

### Distribution

- source code only;
- no signed macOS bundle, DMG, AppImage, or Windows installer is included.
