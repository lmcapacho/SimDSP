#######################################################################
# SimDSP Project file.
#
# Copyright (c) 2017 lmcapacho
#
# This file is part of SimDSP.
#
# SimDSP is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SimDSP is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SimDSP.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################

QT       += widgets multimedia

TARGET = sdcore
TEMPLATE = lib

LIBS += -lfftw3 -lm

DEFINES += SIMDSPCORE_LIBRARY

DESTDIR = build
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

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
