QT += core gui widgets

CONFIG += c++17

TARGET = agenda_qt
TEMPLATE = app

INCLUDEPATH += \
    src \
    src/model

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
    src/model/CategoryManager.cpp

HEADERS += \
    src/model/Priority.h \
    src/model/ActivityKind.h \
    src/model/Activity.h \
    src/model/EventActivity.h \
    src/model/DeadlineActivity.h \
    src/model/ReminderActivity.h \
    src/model/ChecklistActivity.h \
    src/model/ActivityManager.h \
    src/model/SearchEngine.h \
    src/model/ActivityFilter.h \
    src/model/Category.h \
    src/model/CategoryManager.h

RESOURCES += \
    resources/resources.qrc