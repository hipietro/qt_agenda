QT += core gui widgets

CONFIG += c++17

TARGET = agenda_qt
TEMPLATE = app

INCLUDEPATH += \
    src \
    src/model \
    src/gui \
    src/persistence

SOURCES += \
    src/main.cpp \
    src/model/Activity.cpp \
    src/model/EventActivity.cpp \
    src/model/DeadlineActivity.cpp \
    src/model/ReminderActivity.cpp \
    src/model/ChecklistActivity.cpp \
    src/model/ActivityManager.cpp \
    src/model/SearchEngine.cpp \
    src/model/ActivityFilter.cpp \
    src/model/Category.cpp \
    src/model/RecurrenceRule.cpp \
    src/model/ActivityTemplate.cpp \
    src/model/ActivityTemplateManager.cpp \
    src/model/CategoryManager.cpp \
    src/gui/MainWindow.cpp \
    src/gui/ActivityListItemVisitor.cpp \
    src/gui/ActivityListPresentationController.cpp \
    src/gui/ActivityDetailVisitor.cpp \
    src/gui/ActivityDetailPresentationController.cpp \
    src/persistence/ActivityFactoryRegistry.cpp \
    src/persistence/ActivityJsonSerializer.cpp \
    src/persistence/ActivityJsonSerializationVisitor.cpp \
    src/persistence/AgendaJsonStorage.cpp \
    src/gui/ActivityCreationDialog.cpp \
    src/gui/ActivityEditDialog.cpp \
    src/gui/CategoryManagementDialog.cpp \
    src/commands/RemoveActivityCommand.cpp \
    src/commands/AddActivityCommand.cpp \
    src/commands/ToggleCompletionCommand.cpp \
    src/commands/CommandHistory.cpp \
    src/commands/UpdateActivityCommand.cpp

HEADERS += \
    src/model/Priority.h \
    src/model/ActivityKind.h \
    src/model/ActivityVisitor.h \
    src/model/Activity.h \
    src/model/EventActivity.h \
    src/model/DeadlineActivity.h \
    src/model/ReminderActivity.h \
    src/model/ChecklistActivity.h \
    src/model/ActivityManager.h \
    src/model/SearchEngine.h \
    src/model/ActivityFilter.h \
    src/model/Category.h \
    src/model/RecurrenceRule.h \
    src/model/ActivityTemplate.h \
    src/model/ActivityTemplateManager.h \
    src/model/CategoryManager.h \
    src/gui/MainWindow.h \
    src/gui/ActivityListItemVisitor.h \
    src/gui/ActivityListPresentationController.h \
    src/gui/ActivityDetailVisitor.h \
    src/gui/ActivityDetailPresentationController.h \
    src/persistence/ActivityFactoryRegistry.h \
    src/persistence/ActivityJsonSerializer.h \
    src/persistence/ActivityJsonSerializationVisitor.h \
    src/persistence/AgendaJsonStorage.h \
    src/gui/ActivityCreationDialog.h \
    src/gui/ActivityEditDialog.h \
    src/gui/CategoryManagementDialog.h \
    src/commands/Command.h \
    src/commands/RemoveActivityCommand.h \
    src/commands/UpdateActivityCommand.h \
    src/commands/ToggleCompletionCommand.h \
    src/commands/CommandHistory.h \
    src/commands/AddActivityCommand.h

RESOURCES += \
    resources/resources.qrc
