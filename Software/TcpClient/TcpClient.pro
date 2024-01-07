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

QT += core gui widgets

CONFIG += c++2a
CONFIG += qwt

QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion
QMAKE_RPATHDIR += $ORIGIN

DEFINES += APPLICATION_VERSION=\\\"1.0\\\"

INCLUDEPATH += include
LIBS += -L. -lReDeX

SOURCES += \
    aboutdialog.cpp \
    device.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwidget.cpp \
    plotwindow.cpp \
    powerwindow.cpp \
    testpointwidget.cpp

HEADERS += \
    aboutdialog.h \
    device.h \
    mainwindow.h \
    plotwidget.h \
    plotwindow.h \
    powerwindow.h \
    testpointwidget.h \
    units.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    plotwindow.ui \
    powerwindow.ui \
    testpointwidget.ui

RESOURCES += \
    TcpClient.qrc
