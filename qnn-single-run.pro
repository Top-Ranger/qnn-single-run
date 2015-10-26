#-------------------------------------------------
#
# Project created by QtCreator 2015-10-26T12:27:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qnn-single-run
TEMPLATE = app


SOURCES += src/main.cpp\
        src/qnnsinglerun.cpp

HEADERS  += src/qnnsinglerun.h \
    src/additionalsimulationfunctions.hpp

FORMS    += src/qnnsinglerun.ui

unix: LIBS += -L$$PWD/../qnn/ -lqnn
win32: LIBS += -L$$PWD/../qnn/ -lqnn0

INCLUDEPATH += $$PWD/../qnn/src
DEPENDPATH += $$PWD/../qnn/src

OTHER_FILES += \
    LICENSE.GPL3
