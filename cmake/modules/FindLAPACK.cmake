## ---------------------------------------------------------------------
##
## Copyright (C) 2013 - 2022 by the deal.II authors
##
## This file is part of the deal.II library.
##
## The deal.II library is free software; you can use it, redistribute
## it, and/or modify it under the terms of the GNU Lesser General
## Public License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
## The full text of the license can be found in the file LICENSE.md at
## the top level directory of deal.II.
##
## ---------------------------------------------------------------------

#
# This module is a wrapper around the FindLAPACK.cmake module provided by
# CMake.
#
# This module exports
#
#   LAPACK_FOUND
#   LAPACK_LIBRARIES
#   LAPACK_LINKER_FLAGS
#   BLAS_FOUND
#   BLAS_LIBRARIES
#   BLAS_LINKER_FLAGS
#

#
# We have to use a trick with CMAKE_PREFIX_PATH to make LAPACK_DIR and
# BLAS_DIR work...
#
option(LAPACK_WITH_64BIT_BLAS_INDICES
  "BLAS has 64 bit integers."
  OFF
  )
mark_as_advanced(LAPACK_WITH_64BIT_BLAS_INDICES)

set(LAPACK_DIR "" CACHE PATH "An optional hint to a LAPACK installation")
set(BLAS_DIR "" CACHE PATH "An optional hint to a BLAS installation")
set_if_empty(BLAS_DIR "$ENV{BLAS_DIR}")
set_if_empty(LAPACK_DIR "$ENV{LAPACK_DIR}")

set(_cmake_prefix_path_backup "${CMAKE_PREFIX_PATH}")

# temporarily disable ${CMAKE_SOURCE_DIR}/cmake/modules for module lookup
list(REMOVE_ITEM CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules/)

set(CMAKE_PREFIX_PATH ${BLAS_DIR} ${LAPACK_DIR} ${_cmake_prefix_path_backup})
find_package(BLAS)

set(CMAKE_PREFIX_PATH ${LAPACK_DIR} ${_cmake_prefix_path_backup})
find_package(LAPACK)

set(CMAKE_PREFIX_PATH ${_cmake_prefix_path_backup})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules/)

#
# Filter out spurious "FALSE" in the library lists:
#
if(DEFINED BLAS_LIBRARIES)
  list(REMOVE_ITEM BLAS_LIBRARIES "FALSE")
endif()
if(DEFINED LAPACK_LIBRARIES)
  list(REMOVE_ITEM LAPACK_LIBRARIES "FALSE")
endif()

#
# Work around a bug in CMake 3.11 by simply filtering out
# "PkgConf::PKGC_BLAS". See bug
#   https://gitlab.kitware.com/cmake/cmake/issues/17934
#
if(DEFINED BLAS_LIBRARIES)
  list(REMOVE_ITEM BLAS_LIBRARIES "PkgConfig::PKGC_BLAS")
endif()

#
# Well, in case of static archives we have to manually pick up the
# complete link interface. *sigh*
#
# If CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES is not available, do it
# unconditionally for the most common case (gfortran).
#
if(NOT BUILD_SHARED_LIBS)
  set(_fortran_libs ${CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES})
  #
  # Since CMake 3.9 the gcc runtime libraries libgcc.a and libgcc_s.so.1
  # have been added to the CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES variable.
  # We thus have to remove the shared low-level runtime library
  # libgcc_s.so.1 from the link interface; otherwise completely static
  # linkage is broken.
  #
  list(REMOVE_ITEM _fortran_libs gcc_s)
  set_if_empty(_fortran_libs gfortran quadmath m)

  foreach(_lib ${_fortran_libs})
    find_system_library(${_lib}_LIBRARY NAMES ${_lib})
    list(APPEND _additional_libraries ${_lib}_LIBRARY)
  endforeach()
endif()


deal_ii_package_handle(LAPACK
  LIBRARIES
    REQUIRED LAPACK_LIBRARIES
    OPTIONAL BLAS_LIBRARIES ${_additional_libraries}
  LINKER_FLAGS OPTIONAL LAPACK_LINKER_FLAGS BLAS_LINKER_FLAGS
  INCLUDE_DIRS OPTIONAL LAPACK_INCLUDE_DIRS
  USER_INCLUDE_DIRS OPTIONAL LAPACK_INCLUDE_DIRS
  CLEAR
    atlas_LIBRARY atlcblas_LIBRARY atllapack_LIBRARY blas_LIBRARY
    eigen_blas_LIBRARY f77blas_LIBRARY gslcblas_LIBRARY lapack_LIBRARY
    m_LIBRARY ptf77blas_LIBRARY ptlapack_LIBRARY refblas_LIBRARY
    reflapack_LIBRARY BLAS_LIBRARIES ${_additional_libraries}
    LAPACK_SYMBOL_CHECK # clean up check in configure_1_lapack.cmake
  )
