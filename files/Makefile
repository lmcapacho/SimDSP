##################################################
# Project: SimDSP
# Name: Makefile
# Author: lmcapacho (lmcapacho@uniquindio.edu.co)
# Creation Date: 2017-01
# Last Modified: 2017-03
##################################################

CC			= gcc
CXX			= g++
CXXFLAGS 	= -O3 -Wall -fPIC -I/home/lmcapacho/Dropbox/SimDSP/sdcore
LDFLAGS		= -shared -L/home/lmcapacho/Dropbox/SimDSP/sdcore/build
TARGET_LIB 	= libsdcode.so

SRCS 		= $(wildcard ../*.cpp)
OBJS 		= $(subst ../,,$(subst .cpp,.o,$(SRCS)))


.PHONY: all
all: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@ -lsdcore

%.o:../%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET_LIB) *.o
