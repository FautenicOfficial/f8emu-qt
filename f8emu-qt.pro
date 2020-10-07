#-------------------------------------------------
#
# Project created by QtCreator 2018-07-25T15:54:04
#
#-------------------------------------------------

QT += core gui multimedia
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O3
QMAKE_CXXFLAGS_RELEASE *= -O2
# Linux:
# DEFINES += __LINUX_PULSE__
# LIBS += -lpthread -lpulse-simple -lpulse
# Mac (untested):
# DEFINES += __MACOSX_CORE__
# LIBS += -framework CoreAudio -framework CoreFoundation -lpthread
# ICON = f8emu-qt.icns
# Windows:
DEFINES += __WINDOWS_WASAPI__
LIBS += -lole32 -lwinmm -lksuser -luuid
RC_FILE = f8emu-qt.rc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = f8emu-qt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    6502.cpp \
    mainwindow.cpp \
    rtaudio-unix/RtAudio.cpp \
    dialoginput.cpp \
    dialogstdctrl.cpp \
    dialogextctrl.cpp \
    dialogpresskey.cpp \
    dialogtileviewer.cpp \
    dialogbackgroundviewer.cpp \
    dialoghexeditor.cpp \
    dialogdebugger.cpp \
    dialoggameinfo.cpp \
    dialogeditbreakpoint.cpp \
    dialogsetsaveslot.cpp \
    dialogmemorywatch.cpp \
    dialogeditaddress.cpp

HEADERS += \
        mainwindow.h \
    6502.h \
    rtaudio-unix/RtAudio.h \
    dialoginput.h \
    dialogstdctrl.h \
    dialogextctrl.h \
    dialogpresskey.h \
    dialogtileviewer.h \
    dialogbackgroundviewer.h \
    dialoghexeditor.h \
    dialogdebugger.h \
    dialoggameinfo.h \
    dialogeditbreakpoint.h \
    dialogsetsaveslot.h \
    dialogmemorywatch.h \
    dialogeditaddress.h

FORMS += \
        mainwindow.ui \
    dialoginput.ui \
    dialogstdctrl.ui \
    dialogextctrl.ui \
    dialogpresskey.ui \
    dialogtileviewer.ui \
    dialogbackgroundviewer.ui \
    dialoghexeditor.ui \
    dialogdebugger.ui \
    dialoggameinfo.ui \
    dialogeditbreakpoint.ui \
    dialogsetsaveslot.ui \
    dialogmemorywatch.ui \
    dialogeditaddress.ui

DISTFILES += \
    f8emu-qt.rc
