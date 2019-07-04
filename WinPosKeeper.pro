TEMPLATE = app
TARGET = WinPosKeeper

CONFIG += debug_and_release
QT += core gui widgets

INCLUDEPATH += src

LIBS += -luser32 -lgdi32 -lpsapi

SOURCES += $$files(src/*.cpp, true)
HEADERS += $$files(src/*.h, true)
FORMS   += $$files(src/*.ui, true)

RESOURCES += resources/app.qrc

PRECOMPILED_HEADER = src/global.h
SOURCES           -= src/global.h.cpp
