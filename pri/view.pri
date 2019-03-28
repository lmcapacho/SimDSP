#######################################################################
# SimDSP Project file include.
#
# Copyright (c) 2019 lmcapacho
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

HEADERS += \
    src/view/qcustomplot.h \
    src/view/sdkeyboard.h \
    src/view/sdmat.h \
    src/view/sdplot.h \
    src/view/sdview.h
    
SOURCES += \
    src/view/qcustomplot.cpp \
    src/view/sdkeyboard.cpp \
    src/view/sdmat.cpp \
    src/view/sdplot.cpp \
    src/view/sdview.cpp
    
FORMS    += \
    src/view/sdkeyboard.ui \
    src/view/sdmat.ui \
    src/view/sdplot.ui \
    src/view/sdview.ui
