#######################################################################
# SimDSP Project file include.
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

HEADERS += \
    src/project/sdproject.h \
    src/project/sdprojectexplorer.h \
    src/project/sdnewproject.h \
    src/project/sdbuildoptions.h \
    src/project/sdeditlib.h
    
SOURCES += \
    src/project/sdproject.cpp \
    src/project/sdnewproject.cpp \
    src/project/sdprojectexplorer.cpp \
    src/project/sdbuildoptions.cpp \
    src/project/sdeditlib.cpp
    
FORMS    += \
    src/project/sdprojectexplorer.ui \
    src/project/sdnewproject.ui \
    src/project/sdbuildoptions.ui \
    src/project/sdeditlib.ui


