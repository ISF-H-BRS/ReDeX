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

TARGET = ConductanceViewer
TEMPLATE = app

CONFIG += c++2a
CONFIG += qwt

QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion
QMAKE_RPATHDIR += $ORIGIN

INCLUDEPATH += include
LIBS += -L. -lConductance

DEFINES += APPLICATION_VERSION=\\\"1.0\\\"

SOURCES += \
    aboutdialog.cpp \
    analysiswidget.cpp \
    deviceselectiondialog.cpp \
    devicewrapper.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwidget.cpp \
    sensorwrapper.cpp \
    setupwidget.cpp

HEADERS += \
    aboutdialog.h \
    analysiswidget.h \
    device.h \
    deviceselectiondialog.h \
    devicewrapper.h \
    mainwindow.h \
    plotwidget.h \
    sensorwrapper.h \
    setupwidget.h \
    units.h

FORMS += \
    aboutdialog.ui \
    analysiswidget.ui \
    deviceselectiondialog.ui \
    mainwindow.ui \
    setupwidget.ui

RESOURCES += \
    ConductanceViewer.qrc
