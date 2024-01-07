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

project(libReDeX LANGUAGES CXX)
cmake_minimum_required(VERSION 3.14)

set(REDEX_BUILD_LABVIEW_API OFF CACHE BOOL "Build C-API for use with LabVIEW")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fvisibility=hidden -Wall")
endif()

include_directories(include)

set(REDEX_SRC
    include/redex.h
    errorstring.cpp
    errorstring.h
    redex.cpp
    sincfilter.cpp
    sincfilter.h
    tcpclient.cpp
    tcpclient.h
    tcpsocket.cpp
    tcpsocket.h
    voltammogramfilter.cpp
    voltammogramfilter.h
)

if (REDEX_BUILD_LABVIEW_API)
    if (WIN32)
        set(REDEX_CINTOOLS_PREFIX win32)
        set(REDEX_CINTOOLS_LIB labviewv)
    else()
        set(REDEX_CINTOOLS_PREFIX linux)
        set(REDEX_CINTOOLS_LIB lv)
    endif()

    include_directories(../3rdparty/${REDEX_CINTOOLS_PREFIX}/cintools)
    link_directories(../3rdparty/${REDEX_CINTOOLS_PREFIX}/cintools)

    set(REDEX_SRC ${REDEX_SRC} include/redex_lv.h redex_lv.cpp)
    set(REDEX_LINK_LIBRARIES ${REDEX_LINK_LIBRARIES} ${REDEX_CINTOOLS_LIB})
endif()

add_library(ReDeX SHARED ${REDEX_SRC})

if (WIN32)
    target_compile_definitions(ReDeX PRIVATE REDEX_BUILD_PROCESS)
    set_target_properties(ReDeX PROPERTIES PREFIX "")
    set(REDEX_LINK_LIBRARIES ${REDEX_LINK_LIBRARIES} ws2_32 wsock32)
endif()

target_link_libraries(ReDeX ${REDEX_LINK_LIBRARIES})
