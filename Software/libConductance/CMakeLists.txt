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

project(libConductance LANGUAGES CXX)
cmake_minimum_required(VERSION 3.14)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libusb REQUIRED libusb-1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-O2 -fvisibility=hidden -Wall")

include_directories(include)

if (WIN32)
    # Needed for __attribute__((__packed__))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
endif()

add_library(Conductance SHARED
    include/conductance/analyzer.h
    include/conductance/databuffer.h
    include/conductance/device.h
    include/conductance/deviceinfo.h
    include/conductance/global.h
    analyzer.cpp
    databuffer.cpp
    device.cpp
    deviceinfo.cpp
)

if (WIN32)
    target_compile_definitions(Conductance PRIVATE ISF_BUILD_PROCESS)
    set_target_properties(Conductance PROPERTIES PREFIX "")
endif()

target_link_libraries(Conductance usb-1.0)
