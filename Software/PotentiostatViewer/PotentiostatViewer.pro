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

TARGET = PotentiostatViewer
TEMPLATE = app

CONFIG += c++2a
CONFIG += qwt

QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion
QMAKE_RPATHDIR += $ORIGIN

INCLUDEPATH += include
LIBS += -L. -lPotentiostat -lsft

DEFINES += APPLICATION_VERSION=\\\"1.0\\\"

SOURCES += \
    CoSeSt2/CoSeSt2.cpp \
    aboutdialog.cpp \
    calibrationdialog.cpp \
    devicelistener.cpp \
    fftplot.cpp \
    fftwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwidget.cpp \
    serialportwidget.cpp \
    setupwidget.cpp \
    signalfilter.cpp \
    sincfilter.cpp \
    storagewidget.cpp

HEADERS += \
    CoSeSt2/CoSeSt2.h \
    aboutdialog.h \
    averagingbuffer.h \
    calibrationdialog.h \
    device.h \
    devicelistener.h \
    fftplot.h \
    fftwindow.h \
    mainwindow.h \
    plotwidget.h \
    serialportwidget.h \
    settings.h \
    setupwidget.h \
    signalfilter.h \
    sincfilter.h \
    storagewidget.h

FORMS += \
    aboutdialog.ui \
    calibrationdialog.ui \
    fftwindow.ui \
    mainwindow.ui \
    serialportwidget.ui \
    setupwidget.ui \
    storagewidget.ui

RESOURCES += \
    PotentiostatViewer.qrc
