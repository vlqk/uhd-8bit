#
# Copyright 2010 Ettus Research LLC
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

########################################################################
# This file included, use CMake directory variables
########################################################################

########################################################################
# Conditionally configure the USRP-E100 support
########################################################################
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET(LINUX_TARGET TRUE)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

LIBUHD_REGISTER_COMPONENT("USRP-E100" ENABLE_USRP_E100 OFF "ENABLE_LIBUHD;LINUX_TARGET" OFF)

IF(ENABLE_USRP_E100)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

    LIBUHD_APPEND_SOURCES(
        ${CMAKE_CURRENT_SOURCE_DIR}/clock_ctrl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/clock_ctrl.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/codec_ctrl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/codec_ctrl.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/codec_impl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dboard_impl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dboard_iface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dsp_impl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fpga-downloader.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/io_impl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mboard_impl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usrp_e100_impl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usrp_e100_impl.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usrp_e100_iface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usrp_e100_iface.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usrp_e100_mmap_zero_copy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usrp_e100_regs.hpp
    )
ENDIF(ENABLE_USRP_E100)
