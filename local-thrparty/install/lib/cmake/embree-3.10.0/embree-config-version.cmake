## Copyright 2009-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

SET(PACKAGE_VERSION 3.10.0)

IF (${PACKAGE_FIND_VERSION_MAJOR} EQUAL 3)
  IF (${PACKAGE_FIND_VERSION} VERSION_LESS 3.10.0)
    SET(PACKAGE_VERSION_COMPATIBLE 1)  
  ENDIF()
  IF (${PACKAGE_FIND_VERSION} VERSION_EQUAL 3.10.0)
    SET(PACKAGE_VERSION_EXACT 1)  
  ENDIF()
ELSE()
  SET(PACKAGE_VERSION_UNSUITABLE 1)
ENDIF()
