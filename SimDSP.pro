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

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

RESOURCES += \
    simdspresources.qrc

include(pri/actions.pri)
include(pri/editor.pri)
include(pri/project.pri)
include(pri/view.pri)
include(pri/simdsp.pri)

DESTDIR = $$PWD
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

unix {

    isEmpty(PREFIX) {
            PREFIX = /usr
    }
    BINDIR = $$PREFIX/bin
    INCLUDEDIR = $$PREFIX/include
    DATADIR = $$PREFIX/share
    PKGDATADIR = $$DATADIR/simdsp

    target.path = $$BINDIR

    bin.path = $$PKGDATADIR
    bin.files = simdsp

    include.path = $$INCLUDEDIR
    include.files += sdcore/simdsp.h sdcore/sdfunctions.h

    desktop.path = $$DATADIR/applications
    desktop.files += simdsp.desktop

    icon.path = $$DATADIR/icons
    icon.extra = install -D -m 0644 $$PWD/resources/images/simdsp_icon.png $(INSTALL_ROOT)$$DATADIR/icons/simdsp.png

    INSTALLS += target bin include desktop icon

    LIBS += -lfftw3 -lm -lmatio
}

win32 {
    INCLUDEPATH += "$$PWD\resources\dependencies\fftw3"  \
                   "$$PWD\resources\dependencies\matio"
                   
    LIBS += "$$PWD\resources\dependencies\fftw3\libfftw3-3.dll" \
            "$$PWD\resources\dependencies\matio\libmatio.dll" \
            "$$PWD\resources\dependencies\matio\hdf5.dll" \
            "$$PWD\resources\dependencies\matio\zlib.dll"
}

TARGET = SimDSP
TEMPLATE = app
