QT += core gui widgets testlib

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TARGET = agenda_tests
TEMPLATE = app

INCLUDEPATH += \
    ../src \
    ../src/model \
    ../src/gui \
    ../src/persistence \
    ../src/commands

SOURCES += \
    AgendaTests.cpp \
    ../src/model/Activity.cpp \
    ../src/model/EventActivity.cpp \
    ../src/model/DeadlineActivity.cpp \
    ../src/model/ReminderActivity.cpp \
    ../src/model/ChecklistActivity.cpp \
    ../src/model/ActivityManager.cpp \
    ../src/model/SearchEngine.cpp \
    ../src/model/ActivityFilter.cpp \
    ../src/model/Category.cpp \
    ../src/model/RecurrenceRule.cpp \
    ../src/model/ActivityTemplate.cpp \
    ../src/model/ActivityTemplateManager.cpp \
    ../src/model/CategoryManager.cpp \
    ../src/gui/ActivityListItemVisitor.cpp \
    ../src/gui/ActivityDetailVisitor.cpp \
    ../src/persistence/ActivityFactoryRegistry.cpp \
    ../src/persistence/ActivityJsonSerializer.cpp \
    ../src/persistence/ActivityJsonSerializationVisitor.cpp \
    ../src/persistence/AgendaJsonStorage.cpp \
    ../src/commands/RemoveActivityCommand.cpp \
    ../src/commands/AddActivityCommand.cpp \
    ../src/commands/ToggleCompletionCommand.cpp \
    ../src/commands/CommandHistory.cpp \
    ../src/commands/UpdateActivityCommand.cpp

HEADERS += \
    ../src/model/Priority.h \
    ../src/model/ActivityKind.h \
    ../src/model/ActivityVisitor.h \
    ../src/model/Activity.h \
    ../src/model/EventActivity.h \
    ../src/model/DeadlineActivity.h \
    ../src/model/ReminderActivity.h \
    ../src/model/ChecklistActivity.h \
    ../src/model/ActivityManager.h \
    ../src/model/SearchEngine.h \
    ../src/model/ActivityFilter.h \
    ../src/model/Category.h \
    ../src/model/RecurrenceRule.h \
    ../src/model/ActivityTemplate.h \
    ../src/model/ActivityTemplateManager.h \
    ../src/model/CategoryManager.h \
    ../src/gui/ActivityListItemVisitor.h \
    ../src/gui/ActivityDetailVisitor.h \
    ../src/persistence/ActivityFactoryRegistry.h \
    ../src/persistence/ActivityJsonSerializer.h \
    ../src/persistence/ActivityJsonSerializationVisitor.h \
    ../src/persistence/AgendaJsonStorage.h \
    ../src/commands/Command.h \
    ../src/commands/RemoveActivityCommand.h \
    ../src/commands/AddActivityCommand.h \
    ../src/commands/ToggleCompletionCommand.h \
    ../src/commands/CommandHistory.h \
    ../src/commands/UpdateActivityCommand.h
