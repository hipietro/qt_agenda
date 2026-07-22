QT += core testlib

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_jsonserializationvisitor

INCLUDEPATH += \
    ../../src \
    ../../src/model \
    ../../src/persistence

SOURCES += \
    tst_jsonserializationvisitor.cpp \
    ../../src/model/Activity.cpp \
    ../../src/model/EventActivity.cpp \
    ../../src/model/DeadlineActivity.cpp \
    ../../src/model/ReminderActivity.cpp \
    ../../src/model/ChecklistActivity.cpp \
    ../../src/model/RecurrenceRule.cpp \
    ../../src/persistence/ActivityJsonSerializer.cpp \
    ../../src/persistence/ActivityJsonSerializationVisitor.cpp

HEADERS += \
    ../../src/model/Priority.h \
    ../../src/model/ActivityKind.h \
    ../../src/model/ActivityVisitor.h \
    ../../src/model/Activity.h \
    ../../src/model/EventActivity.h \
    ../../src/model/DeadlineActivity.h \
    ../../src/model/ReminderActivity.h \
    ../../src/model/ChecklistActivity.h \
    ../../src/model/RecurrenceRule.h \
    ../../src/persistence/ActivityJsonSerializer.h \
    ../../src/persistence/ActivityJsonSerializationVisitor.h
