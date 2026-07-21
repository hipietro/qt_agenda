QT += core testlib

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_activityvisitor

INCLUDEPATH += \
    ../../src \
    ../../src/model

SOURCES += \
    tst_activityvisitor.cpp \
    ../../src/model/Activity.cpp \
    ../../src/model/EventActivity.cpp \
    ../../src/model/DeadlineActivity.cpp \
    ../../src/model/ReminderActivity.cpp \
    ../../src/model/ChecklistActivity.cpp \
    ../../src/model/RecurrenceRule.cpp

HEADERS += \
    ../../src/model/Priority.h \
    ../../src/model/ActivityKind.h \
    ../../src/model/ActivityVisitor.h \
    ../../src/model/Activity.h \
    ../../src/model/EventActivity.h \
    ../../src/model/DeadlineActivity.h \
    ../../src/model/ReminderActivity.h \
    ../../src/model/ChecklistActivity.h \
    ../../src/model/RecurrenceRule.h
