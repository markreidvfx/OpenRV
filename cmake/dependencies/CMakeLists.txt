#
# Copyright (C) 2022  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

INCLUDE(rv_create_std_deps_vars)
INCLUDE(rv_make_std_lib_name)
INCLUDE(rv_copy_lib_bin_folders)

IF(NOT TARGET dependencies)
  ADD_CUSTOM_TARGET(dependencies)
ENDIF()

IF(NOT DEFINED RV_DEPS_LIST)
  SET(RV_DEPS_LIST
      ""
      CACHE INTERNAL ""
  )
ENDIF()

FIND_PACKAGE(Git QUIET)
IF(GIT_FOUND
   AND EXISTS "${PROJECT_SOURCE_DIR}/.git"
)
  # Update submodules as needed
  OPTION(GIT_SUBMODULE "Check submodules during build" ON)
  IF(GIT_SUBMODULE)
    MESSAGE(STATUS "Updating submodules")
    EXECUTE_PROCESS(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    IF(NOT GIT_SUBMOD_RESULT EQUAL "0")
      MESSAGE(WARNING "Unable to init or update git submodules (${GIT_SUBMOD_RESULT})")
    ENDIF()
  ENDIF()
ENDIF()

IF(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/pub/CMakeLists.txt")
  MESSAGE(FATAL_ERROR "Unable to find required git submodules. GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
ENDIF()

IF(RV_TARGET_WINDOWS)
  INCLUDE(expat)
ENDIF()

# TODO_QT: Not generic enough.. But maybe we could use something like NOT CY2023 since after 2023, it is Qt6
IF(RV_VFX_PLATFORM STREQUAL CY2023)
  INCLUDE(qt5)
ELSEIF(RV_VFX_PLATFORM STREQUAL CY2024)
  INCLUDE(qt6)
ENDIF()

INCLUDE(openssl) # Python3 requirement
INCLUDE(python3)
INCLUDE(imgui)
INCLUDE(nanobind)
INCLUDE(pyimgui)
INCLUDE(pyimplot)
INCLUDE(boost)
INCLUDE(dav1d)
IF(RV_TARGET_WINDOWS)
  INCLUDE(atomic_ops)
ENDIF()
IF(RV_TARGET_DARWIN
   OR RV_TARGET_LINUX
)
  INCLUDE(gc)
  INCLUDE(glew)
ENDIF()
INCLUDE(imath)
INCLUDE(zlib)
INCLUDE(ffmpeg)
INCLUDE(doctest)
INCLUDE(jpegturbo) # jpegturbo huge advantage over jpeg is that it does come with CMake support!
INCLUDE(png)
INCLUDE(tiff) # depends on jpeg, jpegturbo
# src/pub: lmcs: src/pub will be removed eventually, this path will needed to change.
ADD_SUBDIRECTORY(../../src/pub/lcms src/pub/lcms) # required by raw
INCLUDE(raw) # depends on jpeg, zlib
INCLUDE(openjpeg) # depends on zlib, tiff, png
INCLUDE(openjph)
INCLUDE(webp)
INCLUDE(openexr)
INCLUDE(ocio)
# src/pub: freetype: src/pub will be removed eventually, this path will needed to change.
ADD_SUBDIRECTORY(../../src/pub/freetype src/pub/freetype) # required by oiio
INCLUDE(oiio) # depends on openexr and most image formats above and ocio (actually, there's inter-dependency between ocio and oiio)
IF(RV_TARGET_WINDOWS)
  INCLUDE(pcre2)
ENDIF()
INCLUDE(yaml-cpp) # depends on OCIO

INCLUDE(bmd.cmake)
INCLUDE(spdlog)
INCLUDE(aja.cmake)

LIST(REMOVE_DUPLICATES RV_DEPS_LIST)
SET(RV_DEPS_LIST
    ${RV_DEPS_LIST}
    CACHE INTERNAL ""
)

MESSAGE(STATUS "Using AJA:          ${RV_DEPS_AJA_VERSION}")
MESSAGE(STATUS "Using atomic_ops:   ${RV_DEPS_ATOMIC_OPS_VERSION}")
IF(RV_DEPS_BMD_VERSION)
  MESSAGE(STATUS "Using BMD:          ${RV_DEPS_BMD_VERSION}")
ENDIF()
MESSAGE(STATUS "Using Boost:        ${RV_DEPS_BOOST_VERSION}")
MESSAGE(STATUS "Using Dav1d:        ${RV_DEPS_DAV1D_VERSION}")
MESSAGE(STATUS "Using FFMPEG:       ${RV_DEPS_FFMPEG_VERSION}")
IF(RV_TARGET_WINDOWS)
  MESSAGE(STATUS "Using atomic_ops:   ${RV_DEPS_ATOMIC_OPS_VERSION}")
ENDIF()
IF(RV_TARGET_DARWIN
   OR RV_TARGET_LINUX
)
  MESSAGE(STATUS "Using GC:           ${RV_DEPS_GC_VERSION}")
  MESSAGE(STATUS "Using GLEW:         ${RV_DEPS_GLEW_VERSION}")
ENDIF()
MESSAGE(STATUS "Using Imath:        ${RV_DEPS_IMATH_VERSION}")
MESSAGE(STATUS "Using JpegTurbo:    ${RV_DEPS_JPEGTURBO_VERSION}")
MESSAGE(STATUS "Using OpenEXR:      ${RV_DEPS_OPENEXR_VERSION}")
MESSAGE(STATUS "Using OpenColorIO   ${RV_DEPS_OCIO_VERSION}")
MESSAGE(STATUS "Using OpenImageIO   ${RV_DEPS_OIIO_VERSION}")
MESSAGE(STATUS "Using OpenJpeg:     ${RV_DEPS_OPENJPEG_VERSION}")
MESSAGE(STATUS "Using OpenJPH:      ${RV_DEPS_OPENJPH_VERSION}")
MESSAGE(STATUS "Using OpenSSL:      ${RV_DEPS_OPENSSL_VERSION}")
IF(RV_TARGET_WINDOWS)
  MESSAGE(STATUS "Using PCRE2:        ${RV_DEPS_PCRE2_VERSION}")
ENDIF()
MESSAGE(STATUS "Using PNG:          ${RV_DEPS_PNG_VERSION}")
MESSAGE(STATUS "Using Python3:      ${RV_DEPS_PYTHON3_VERSION}")

RV_VFX_SET_VARIABLE(_rv_deps_pyside_version CY2023 "Using PySide2:      ${RV_DEPS_PYSIDE_VERSION}" CY2024 "Using PySide6:      ${RV_DEPS_PYSIDE_VERSION}")
MESSAGE(STATUS "${_rv_deps_pyside_version}")

RV_VFX_SET_VARIABLE(_rv_deps_qt_version CY2023 "Using Qt5:          ${RV_DEPS_QT5_VERSION}" CY2024 "Using Qt6:          ${RV_DEPS_QT6_VERSION}")
MESSAGE(STATUS "${_rv_deps_qt_version}")

MESSAGE(STATUS "Using Raw:          ${RV_DEPS_RAW_VERSION}")
MESSAGE(STATUS "Using Tiff:         ${RV_DEPS_TIFF_VERSION}")
MESSAGE(STATUS "Using WebP:         ${RV_DEPS_WEBP_VERSION}")
MESSAGE(STATUS "Using Yaml-CPP:     ${RV_DEPS_YAML_CPP_VERSION} (Using OCIO's own copy)")
MESSAGE(STATUS "Using zlib:         ${RV_DEPS_ZLIB_VERSION}")
MESSAGE(STATUS "Using spdlog:       ${RV_DEPS_SPDLOG_VERSION}")
