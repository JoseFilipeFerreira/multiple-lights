## Copyright 2009-2019 Intel Corporation
## SPDX-License-Identifier: Apache-2.0


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was ospcommonConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/ospcommon_Exports.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/ospcommon_macros.cmake")

check_required_components("ospcommon")

## Stash incoming CMAKE_MODULE_PATH ##

set(OSPCOMMON_CALLERS_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})

## Create ospcommon tasking target ##

set(OSPCOMMON_TASKING_SYSTEM      TBB)

set(OSPCOMMON_TASKING_TBB         TRUE)
set(OSPCOMMON_TASKING_OPENMP      FALSE)
set(OSPCOMMON_TASKING_INTERNAL    FALSE)
set(OSPCOMMON_TASKING_DEBUG       FALSE)

ospcommon_create_tasking_target(TRUE)

## Restore CMAKE_MODULE_PATH ##

set(CMAKE_MODULE_PATH ${OSPCOMMON_CALLERS_CMAKE_MODULE_PATH})

## Standard signal that the package was found ##

set(OSPCOMMON_FOUND TRUE)
