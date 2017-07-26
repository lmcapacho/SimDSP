#-------------------------------------------------
#
# Project created by QtCreator 2016-09-22T19:28:03
#
#-------------------------------------------------

QT       += core gui multimedia help
QT       -= sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SimDSP
TEMPLATE = app

INCLUDEPATH += sdcore

LIBS += -L../sdcore/build -lsdcore

SOURCES += main.cpp\
        simdsp.cpp \
    highlighter.cpp \
    sdeditor.cpp \
    sdeditortab.cpp \
    sdproject.cpp \
    sdprojectexplorer.cpp \
    sdbuilder.cpp \
    sdnewfile.cpp \
    sdnewproject.cpp \
    sdhelp.cpp

HEADERS  += simdsp.h \
    highlighter.h \
    sdeditor.h \
    sdeditortab.h \
    sdproject.h \
    sdprojectexplorer.h \
    sdbuilder.h \
    sdnewfile.h \
    sdnewproject.h \
    sdhelp.h

FORMS    += simdsp.ui \
    sdeditortab.ui \
    sdprojectexplorer.ui \
    sdnewfile.ui \
    sdnewproject.ui \
    sdhelp.ui

RESOURCES += \
    textfiles.qrc
