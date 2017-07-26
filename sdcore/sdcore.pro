#-------------------------------------------------
#
# Project created by QtCreator 2017-01-31T16:39:37
#
#-------------------------------------------------

QT       += widgets multimedia

TARGET = sdcore
TEMPLATE = lib

LIBS += -lfftw3 -lm

DEFINES += SIMDSPCORE_LIBRARY

SOURCES += sdcore.cpp \
    sdkeyboard.cpp \
    qcustomplot.cpp \
    sdfunctions.cpp \
    sdsignal.cpp \
    sdaudio.cpp \
    sdplot.cpp

HEADERS += sdcore.h\
    sdcore_global.h \
    sdkeyboard.h \
    qcustomplot.h \
    sdfunctions.h \
    sdsignal.h \
    sdaudio.h \
    sdplot.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    sdkeyboard.ui \
    sdcore.ui \
    sdplot.ui
