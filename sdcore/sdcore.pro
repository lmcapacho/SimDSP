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

QT       += widgets multimedia printsupport
CONFIG   += c++11

TARGET = sdcore
TEMPLATE = lib

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
    sdplot.cpp \
    sdmat.cpp

HEADERS += sdcore.h\
    sdcore_global.h \
    sdkeyboard.h \
    qcustomplot.h \
    sdfunctions.h \
    sdsignal.h \
    sdaudio.h \
    sdplot.h \
    sdmat.h

INCLUDEPATH += $$PWD/../resources/dependencies/rtaudio

unix {
    target.path = /usr/lib
    INSTALLS += target
    LIBS += -lfftw3 -lm -lmatio
    LIBS += -L$$PWD/../resources/dependencies/rtaudio/libs/linux-64 -lrtaudio
}

win32 {
    INCLUDEPATH += "$$PWD\..\resources\dependencies\fftw3"  \
                   "$$PWD\..\resources\dependencies\matio"
    LIBS += "$$PWD\..\resources\dependencies\fftw3\libfftw3-3.dll" \
            "$$PWD\..\resources\dependencies\matio\libmatio.dll" \
            "$$PWD\..\resources\dependencies\matio\hdf5.dll" \
            "$$PWD\..\resources\dependencies\matio\zlib.dll"
	LIBS += -L$$PWD\..\resources\dependencies\rtaudio\libs\windows-32 -lrtaudio            
}

FORMS += \
    sdkeyboard.ui \
    sdcore.ui \
    sdplot.ui \
    sdmat.ui
