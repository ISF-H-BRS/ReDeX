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

QT += core gui serialport widgets

TARGET = SensorViewer
TEMPLATE = app

CONFIG += c++2a

QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion
QMAKE_RPATHDIR += $ORIGIN

INCLUDEPATH += include
LIBS += -L. -lSensors

DEFINES += APPLICATION_VERSION=\\\"1.0\\\"

SOURCES += \
    aboutdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    nonewidget.cpp \
    phwidget.cpp \
    serialportwidget.cpp \
    temperaturewidget.cpp

HEADERS += \
    aboutdialog.h \
    mainwindow.h \
    nonewidget.h \
    phwidget.h \
    sensorwidget.h \
    serialportwidget.h \
    temperaturewidget.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    phwidget.ui \
    serialportwidget.ui

RESOURCES += \
    SensorViewer.qrc
