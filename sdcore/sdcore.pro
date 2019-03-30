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

CONFIG   += c++11

QMAKE_CXXFLAGS += -pthread

TARGET = sdcore
TEMPLATE = lib

DEFINES += SIMDSPCORE_LIBRARY

DESTDIR = build
OBJECTS_DIR = build/obj
MOC_DIR = build/moc

SOURCES += sdcore.cpp \
    sdfunctions.cpp \
    sdsignal.cpp \
    sdaudio.cpp

HEADERS += sdcore.h\
    sdcore_global.h \
    sdfunctions.h \
    sdsignal.h \
    sdaudio.h

INCLUDEPATH += $$PWD/../resources/dependencies/rtaudio

unix {
    target.path = /usr/lib
    INSTALLS += target
    LIBS +=  -lm
    LIBS += -L$$PWD/../resources/dependencies/rtaudio/libs/linux-64 -lrtaudio
}

win32 {
    LIBS += -L$$PWD\..\resources\dependencies\rtaudio\libs\windows-64 -lrtaudio
}


