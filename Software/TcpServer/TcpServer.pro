####################################################################################################
#                                                                                                  #
#   This file is part of the ISF ReDeX project.                                                    #
#                                                                                                  #
#   Author:                                                                                        #
#   Marcel Hasler <mahasler@gmail.com>                                                             #
#                                                                                                  #
#   Copyright (c) 2021 - 2023                                                                      #
#   Bonn-Rhein-Sieg University of Applied Sciences                                                 #
#                                                                                                  #
#   This program is free software: you can redistribute it and/or modify it under the terms        #
#   of the GNU General Public License as published by the Free Software Foundation, either         #
#   version 3 of the License, or (at your option) any later version.                               #
#                                                                                                  #
#   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
#   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      #
#   See the GNU General Public License for more details.                                           #
#                                                                                                  #
#   You should have received a copy of the GNU General Public License along with this program.     #
#   If not, see <https:# www.gnu.org/licenses/>.                                                   #
#                                                                                                  #
####################################################################################################

QT -= gui
QT += network serialport xml

CONFIG += c++2a console
CONFIG -= app_bundle

QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion
QMAKE_RPATHDIR += $ORIGIN

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += include
LIBS += -L. -lConductance -lPotentiostat -lSensors

SOURCES += \
    alarm.cpp \
    assertions.cpp \
    conductancenode.cpp \
    conductancesensor.cpp \
    configuration.cpp \
    devicemanager.cpp \
    devicerunner.cpp \
    exception.cpp \
    lockfile.cpp \
    logger.cpp \
    main.cpp \
    messagedispatcher.cpp \
    node.cpp \
    orpsensor.cpp \
    phsensor.cpp \
    potentiostatnode.cpp \
    potentiostatsensor.cpp \
    powermonitor.cpp \
    protocol.cpp \
    sensor.cpp \
    sensorsnode.cpp \
    sensorssensor.cpp \
    statusboard.cpp \
    tcpserver.cpp \
    temperaturesensor.cpp \
    testpoint.cpp

HEADERS += \
    alarm.h \
    assertions.h \
    conductancenode.h \
    conductancesensor.h \
    configuration.h \
    devicemanager.h \
    devicerunner.h \
    exception.h \
    lockfile.h \
    logger.h \
    macro.h \
    messagedispatcher.h \
    node.h \
    orpsensor.h \
    parsererror.h \
    phsensor.h \
    potentiostatnode.h \
    potentiostatsensor.h \
    powermonitor.h \
    protocol.h \
    sensor.h \
    sensorsnode.h \
    sensorssensor.h \
    statusboard.h \
    tcpserver.h \
    temperaturesensor.h \
    testpoint.h
