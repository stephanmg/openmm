#---------------------------------------------------
# OpenMM 
#
# Creates OpenMM library, base name=OpenMM.
# Default libraries are shared & optimized. Variants
# are created for static (_static) and debug (_d).
#
# Windows:
#   OpenMM[_d].dll
#   OpenMM[_d].lib
#   OpenMM_static[_d].lib
# Unix:
#   libOpenMM[_d].so
#   libOpenMM_static[_d].a
#----------------------------------------------------

# Don't create a new project name if this is part of a mega-build from the
# parent directory
IF( NOT PROJECT_NAME )
    PROJECT (OpenMM)
ENDIF( NOT PROJECT_NAME )

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#SET(CMAKE_VERBOSE_MAKEFILE 1)

# CMake 2.4 on dart server only needs to know about Dart
INCLUDE(Dart)
MARK_AS_ADVANCED(DART_ROOT)

# Avoid dart server problem with cmake 2.4 --cmb
set(cmv "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")
IF(NOT cmv EQUAL "2.4") # This whole file...

# We have custom cmake modules for FindOpenMM and running python tests
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

# Older cmake versions do not have cmake_policy command
# Cmake 2.4.5, the default cmake on red hat linux, has the
# cmake_policy command, but it does not work
# "if(POLICY ..." does not work with cmake 2.4.[56] on red hat
# (cmake 2.4.7 is OK)
if(COMMAND cmake_policy)
    if(CMAKE_MAJOR_VERSION GREATER 2 OR CMAKE_MINOR_VERSION GREATER 5)
        cmake_policy(SET CMP0003 NEW) 
        cmake_policy(SET CMP0005 NEW) 
    endif(CMAKE_MAJOR_VERSION GREATER 2 OR CMAKE_MINOR_VERSION GREATER 5)
    if(CMAKE_MAJOR_VERSION GREATER 2 OR CMAKE_MINOR_VERSION GREATER 6 OR CMAKE_PATCH_VERSION GREATER 2)
        cmake_policy(SET CMP0011 NEW) 
    endif(CMAKE_MAJOR_VERSION GREATER 2 OR CMAKE_MINOR_VERSION GREATER 6 OR CMAKE_PATCH_VERSION GREATER 2)
endif(COMMAND cmake_policy)

# Where to install
IF(${CMAKE_C_COMPILER} MATCHES "gcc")
    IF(NOT OPENMM_INSTALL_PREFIX)
        SET(OPENMM_INSTALL_PREFIX "/usr/local/openmm")
    ENDIF(NOT OPENMM_INSTALL_PREFIX)
ELSE(${CMAKE_C_COMPILER} MATCHES "gcc")
    IF(NOT OPENMM_INSTALL_PREFIX)
        SET(OPENMM_INSTALL_PREFIX "$ENV{ProgramFiles}/OpenMM")
    ENDIF(NOT OPENMM_INSTALL_PREFIX)
ENDIF(${CMAKE_C_COMPILER} MATCHES "gcc")

# It seems that on linux and mac, everything is trying to be installed in /usr/local/openmm
# But if every install target is prefixed with /openmm/, on Windows the install files
# end up in C:/Program Files/OpenMM/openmm/ which is ugly.
# Better to set CMAKE_INSTALL_PREFIX to /usr/local/openmm and leave /openmm/ off the 
# install target names.  Plus, the user now has the opportunity to install into /usr/local/
# if she so chooses.  --cmb
IF(${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})
  SET(CMAKE_INSTALL_PREFIX ${OPENMM_INSTALL_PREFIX} CACHE PATH "Where to install OpenMM" FORCE)
ENDIF(${CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT})

# The source is organized into subdirectories, but we handle them all from
# this CMakeLists file rather than letting CMake visit them as SUBDIRS.
SET(OPENMM_SOURCE_SUBDIRS . openmmapi olla libraries/jama libraries/quern libraries/lepton libraries/sfmt libraries/lbfgs libraries/hilbert libraries/csha1 platforms/reference serialization libraries/validate)
IF(WIN32)
    SET(OPENMM_SOURCE_SUBDIRS ${OPENMM_SOURCE_SUBDIRS} libraries/pthreads)
    ADD_CUSTOM_TARGET(PthreadsLibraries ALL)
    FILE(GLOB PTHREADS_LIBRARIES libraries/pthreads/lib/*.dll libraries/pthreads/lib/*.lib)
	ADD_CUSTOM_COMMAND(TARGET PthreadsLibraries COMMAND  ${CMAKE_COMMAND} ARGS -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")
    FOREACH(lib ${PTHREADS_LIBRARIES})
        ADD_CUSTOM_COMMAND(TARGET PthreadsLibraries COMMAND ${CMAKE_COMMAND} ARGS -E copy ${lib} "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")
        INSTALL(FILES ${lib} DESTINATION "lib/")
    ENDFOREACH(lib)
    LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
    SET(PTHREADS_LIB pthreadVC2)
ELSE(WIN32)
    SET(PTHREADS_LIB pthread)
ENDIF(WIN32)

# The build system will set ARCH64 for 64 bit builds, which require
# use of the lib64/ library directories rather than lib/.
#SET( ARCH64 OFF CACHE BOOL "ON for 64bit builds, OFF for 32bit builds")
#MARK_AS_ADVANCED( ARCH64  )
#IF (ARCH64)
#    SET(LIB64 64)
#ELSE (ARCH64)
#    SET(LIB64) # nothing
#ENDIF (ARCH64)

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  SET( LIB64 64 )
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  SET( LIB64  )
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8 )

IF (APPLE)
    # Build universal binaries compatible with OS X 10.7
    IF (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        SET (CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "The minimum version of OS X to support" FORCE)
    ENDIF (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    IF (NOT CMAKE_OSX_ARCHITECTURES)
        SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "The processor architectures to build for" FORCE)
    ENDIF (NOT CMAKE_OSX_ARCHITECTURES)

    # Improve the linking behavior of Mac libraries
    SET (CMAKE_INSTALL_NAME_DIR "@rpath")
    SET(EXTRA_COMPILE_FLAGS "-msse2 -stdlib=libc++")
ELSE (APPLE)
    IF (WIN32)
        SET(EXTRA_COMPILE_FLAGS)
    ELSE (WIN32)
        SET(EXTRA_COMPILE_FLAGS "-msse2")
    ENDIF (WIN32)
ENDIF (APPLE)

IF(UNIX AND NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Debug or Release build" FORCE)
ENDIF (UNIX AND NOT CMAKE_BUILD_TYPE)

IF (NOT CMAKE_CXX_FLAGS_DEBUG)
    SET(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "To use when CMAKE_BUILD_TYPE=Debug" FORCE)
ENDIF (NOT CMAKE_CXX_FLAGS_DEBUG)

IF (NOT CMAKE_CXX_FLAGS_RELEASE)
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING 
          "To use when CMAKE_BUILD_TYPE=Release" FORCE)
ENDIF (NOT CMAKE_CXX_FLAGS_RELEASE)



# Collect up information about the version of the OpenMM library we're building
# and make it available to the code so it can be built into the binaries.

SET(OPENMM_LIBRARY_NAME OpenMM)
SET(OPENMM_MAJOR_VERSION 5)
SET(OPENMM_MINOR_VERSION 2)
SET(OPENMM_BUILD_VERSION 0)

SET(OPENMM_COPYRIGHT_YEARS "2008")

# underbar separated list of dotted authors, no spaces or commas
SET(OPENMM_AUTHORS "Peter.Eastman")

ADD_DEFINITIONS(-DOPENMM_LIBRARY_NAME=${OPENMM_LIBRARY_NAME}
                -DOPENMM_MAJOR_VERSION=${OPENMM_MAJOR_VERSION}
                -DOPENMM_MINOR_VERSION=${OPENMM_MINOR_VERSION}
                -DOPENMM_BUILD_VERSION=${OPENMM_BUILD_VERSION})

# CMake quotes automatically when building Visual Studio projects but we need
# to add them ourselves for Linux or Cygwin. Two cases to avoid duplicate quotes
# in Visual Studio which end up in the binary.

IF (${CMAKE_GENERATOR} MATCHES "Visual Studio")
   SET(NEED_QUOTES FALSE)
ELSE (${CMAKE_GENERATOR} MATCHES "Visual Studio")
   SET(NEED_QUOTES TRUE)
ENDIF (${CMAKE_GENERATOR} MATCHES "Visual Studio")

##TODO: doesn't work without quotes in nightly build
SET(NEED_QUOTES TRUE)

IF(NEED_QUOTES)
   ADD_DEFINITIONS(-DOPENMM_COPYRIGHT_YEARS="${OPENMM_COPYRIGHT_YEARS}"
                   -DOPENMM_AUTHORS="${OPENMM_AUTHORS}")
ELSE(NEED_QUOTES)
   ADD_DEFINITIONS(-DOPENMM_COPYRIGHT_YEARS=${OPENMM_COPYRIGHT_YEARS}
                   -DOPENMM_AUTHORS=${OPENMM_AUTHORS})
ENDIF(NEED_QUOTES)

# -DOPENMM_TYPE has to be defined in the target subdirectories.
# -Dsimbody_EXPORTS defined automatically when Windows DLL build is being done.

# Report the version number to the CMake UI
SET(OPENMM_VERSION
    "${OPENMM_MAJOR_VERSION}.${OPENMM_MINOR_VERSION}.${OPENMM_BUILD_VERSION}"
     CACHE STRING "This is the version of OpenMM which will be built." FORCE)
# We don't actually use this version for anything yet
MARK_AS_ADVANCED(OPENMM_VERSION)

SET(SHARED_TARGET ${OPENMM_LIBRARY_NAME})
SET(STATIC_TARGET ${OPENMM_LIBRARY_NAME}_static)


## If no one says otherwise, change the executable path to drop into the same binary
## location as the DLLs so that the test cases will use the just-build DLLs.
IF(NOT EXECUTABLE_OUTPUT_PATH)
  SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR} 
      CACHE INTERNAL "Single output directory for building all executables.")
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)
IF(NOT LIBRARY_OUTPUT_PATH)
  SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR} 
      CACHE INTERNAL "Single output directory for building all libraries.")
ENDIF(NOT LIBRARY_OUTPUT_PATH)
SET(${PROJECT_NAME}_EXECUTABLE_DIR ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
SET(${PROJECT_NAME}_LIBRARY_DIR    ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})

# Ensure that debug libraries have "_d" appended to their names.
# CMake gets this right on Windows automatically with this definition.
IF (${CMAKE_GENERATOR} MATCHES "Visual Studio")
    SET(CMAKE_DEBUG_POSTFIX "_d" CACHE INTERNAL "" FORCE)
ENDIF (${CMAKE_GENERATOR} MATCHES "Visual Studio")

# But on Unix or Cygwin we have to add the suffix manually
IF (UNIX AND CMAKE_BUILD_TYPE MATCHES Debug)
    SET(SHARED_TARGET ${SHARED_TARGET}_d)
    SET(STATIC_TARGET ${STATIC_TARGET}_d)
ENDIF (UNIX AND CMAKE_BUILD_TYPE MATCHES Debug)

# used by plugin
SET(OPENMM_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# These are all the places to search for header files which are
# to be part of the API.
SET(API_INCLUDE_DIRS) # start empty
FOREACH(subdir ${OPENMM_SOURCE_SUBDIRS})
    # append
    SET(API_INCLUDE_DIRS ${API_INCLUDE_DIRS}
                         ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include 
                         ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/openmm
                         ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/openmm/internal)
ENDFOREACH(subdir)

# We'll need both *relative* path names, starting with their API_INCLUDE_DIRS,
# and absolute pathnames.
SET(API_REL_INCLUDE_FILES)   # start these out empty
SET(API_ABS_INCLUDE_FILES)

FOREACH(dir ${API_INCLUDE_DIRS})
    FILE(GLOB fullpaths ${dir}/*.h) # returns full pathnames
    SET(API_ABS_INCLUDE_FILES ${API_ABS_INCLUDE_FILES} ${fullpaths})

    FOREACH(pathname ${fullpaths})
        GET_FILENAME_COMPONENT(filename ${pathname} NAME)
        SET(API_REL_INCLUDE_FILES ${API_REL_INCLUDE_FILES} ${dir}/${filename})
    ENDFOREACH(pathname)
ENDFOREACH(dir)

# collect up source files
SET(SOURCE_FILES) # empty
SET(SOURCE_INCLUDE_FILES)

FOREACH(subdir ${OPENMM_SOURCE_SUBDIRS})
    FILE(GLOB src_files  ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/src/*/*.cpp)
    FILE(GLOB incl_files ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/src/*.h)
    SET(SOURCE_FILES         ${SOURCE_FILES}         ${src_files})   #append
    SET(SOURCE_INCLUDE_FILES ${SOURCE_INCLUDE_FILES} ${incl_files})

    ## Make sure we find these locally before looking in OpenMM/include if
    ## OpenMM was previously installed there.
    INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include)
ENDFOREACH(subdir)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/libraries/sfmt/src/SFMT.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_SSE2=1")

# If API wrappers are being generated, and add them to the build.
SET(OPENMM_BUILD_C_AND_FORTRAN_WRAPPERS ON CACHE BOOL "Build wrappers for C and Fortran")
IF(OPENMM_BUILD_C_AND_FORTRAN_WRAPPERS)
    ADD_SUBDIRECTORY(wrappers)
    SET(SOURCE_FILES ${SOURCE_FILES} wrappers/OpenMMCWrapper.cpp  wrappers/OpenMMFortranWrapper.cpp)
    SET_SOURCE_FILES_PROPERTIES(wrappers/OpenMMCWrapper.cpp wrappers/OpenMMFortranWrapper.cpp PROPERTIES GENERATED TRUE)
ENDIF(OPENMM_BUILD_C_AND_FORTRAN_WRAPPERS)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)

ADD_LIBRARY(${SHARED_TARGET} SHARED ${SOURCE_FILES} ${SOURCE_INCLUDE_FILES} ${API_ABS_INCLUDE_FILES})
SET_TARGET_PROPERTIES(${SHARED_TARGET} PROPERTIES LINK_FLAGS "${EXTRA_COMPILE_FLAGS}" COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -DOPENMM_BUILDING_SHARED_LIBRARY -DLEPTON_BUILDING_SHARED_LIBRARY -DOPENMM_VALIDATE_BUILDING_SHARED_LIBRARY")
IF(WIN32)
    ADD_DEPENDENCIES(${SHARED_TARGET} PthreadsLibraries)
ENDIF(WIN32)

SET(OPENMM_BUILD_STATIC_LIB OFF CACHE BOOL "Whether to build static OpenMM libraries")
IF(OPENMM_BUILD_STATIC_LIB)
    ADD_LIBRARY(${STATIC_TARGET} STATIC ${SOURCE_FILES} ${SOURCE_INCLUDE_FILES} ${API_ABS_INCLUDE_FILES})
    SET_TARGET_PROPERTIES(${STATIC_TARGET} PROPERTIES LINK_FLAGS "${EXTRA_COMPILE_FLAGS}" COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -DOPENMM_USE_STATIC_LIBRARIES -DOPENMM_BUILDING_STATIC_LIBRARY -DLEPTON_USE_STATIC_LIBRARIES -DLEPTON_BUILDING_STATIC_LIBRARY -DOPENMMM_VALIDATE_BUILDING_STATIC_LIBRARY -DOPENMM_VALIDATE_BUILDING_STATIC_LIBRARY")
ENDIF(OPENMM_BUILD_STATIC_LIB)

IF(OPENMM_BUILD_C_AND_FORTRAN_WRAPPERS)
    ADD_DEPENDENCIES(${SHARED_TARGET} ApiWrappers)
    IF(OPENMM_BUILD_STATIC_LIB)
        ADD_DEPENDENCIES(${STATIC_TARGET} ApiWrappers)
    ENDIF(OPENMM_BUILD_STATIC_LIB)
ENDIF(OPENMM_BUILD_C_AND_FORTRAN_WRAPPERS)

# On Linux need to link to libdl
FIND_LIBRARY(DL_LIBRARY dl)
IF(DL_LIBRARY)
  TARGET_LINK_LIBRARIES(${SHARED_TARGET} ${DL_LIBRARY} ${PTHREADS_LIB})
  IF(OPENMM_BUILD_STATIC_LIB)
    TARGET_LINK_LIBRARIES(${STATIC_TARGET} ${DL_LIBRARY} ${PTHREADS_LIB})
  ENDIF(OPENMM_BUILD_STATIC_LIB)
ENDIF(DL_LIBRARY)
IF(WIN32)
    MARK_AS_ADVANCED(DL_LIBRARY)
ENDIF(WIN32)

ADD_SUBDIRECTORY(platforms/reference/tests)

# Which hardware platforms to build

# A bit of tedium because we are using custom FindCUDA files that happen to work...
SET(FINDCUDA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/FindCUDA")
SET(CUDA_BUILD_CUBIN OFF)
FIND_PACKAGE(CUDA QUIET)
IF(CUDA_FOUND)
    IF(NOT CUDA_NVCC_FLAGS)
        SET(FLAGS "")
        # Note that cmake will insert semicolons between these item automatically...
        SET(FLAGS ${FLAGS} -gencode arch=compute_11,code=sm_11)
        SET(FLAGS ${FLAGS} -gencode arch=compute_12,code=sm_12)
        SET(FLAGS ${FLAGS} -gencode arch=compute_13,code=sm_13)
        SET(FLAGS ${FLAGS} -gencode arch=compute_20,code=sm_20)
        SET(FLAGS ${FLAGS} -use_fast_math)
        IF(MSVC)
            # Unfortunately the variables CUDA_NVCC_FLAGS_RELEASE and CUDA_NVCC_FLAGS_DEBUG
            # appear to be unused, at least in CMake 2.6
            # Release /MD linkage
            set(FLAGS ${FLAGS} "-Xcompiler \"/MD\" -DOPENMMCUDA_BUILDING_SHARED_LIBRARY")
        ENDIF(MSVC)
        SET(CUDA_NVCC_FLAGS  "${FLAGS}"
            CACHE STRING "Semicolon delimit multiple arguments")
    ENDIF(NOT CUDA_NVCC_FLAGS)
    SET(OPENMM_BUILD_CUDA_LIB ON CACHE BOOL "Build OpenMMCuda library for Nvidia GPUs")

    # I wish I was not hardcoding /MD here
    # Avoid strange windows link error with cuda free energy 
    # plugin by specifying /MD to CUDA_NVCC_FLAGS
    MARK_AS_ADVANCED(CLEAR CUDA_NVCC_FLAGS)
    IF(MSVC)
        # Unfortunately the variables CUDA_NVCC_FLAGS_RELEASE and CUDA_NVCC_FLAGS_DEBUG
        # appear to be unused, at least in CMake 2.6
        # Release /MD linkage
        SET(HAS_NVCC_FLAG FALSE)
        IF(CUDA_NVCC_FLAGS MATCHES "-Xcompiler")
            SET(HAS_NVCC_FLAG TRUE)
        ENDIF(CUDA_NVCC_FLAGS MATCHES "-Xcompiler")
        IF(NOT HAS_NVCC_FLAG)
            set(new_flags "-Xcompiler \"/MD\" -DOPENMMCUDA_BUILDING_SHARED_LIBRARY")
            if(CUDA_NVCC_FLAGS)
                set(new_flags "${CUDA_NVCC_FLAGS};${new_flags}")
            endif(CUDA_NVCC_FLAGS)
            SET(CUDA_NVCC_FLAGS "${new_flags}" 
                CACHE STRING "Semicolon delimit multiple arguments" FORCE)
        ENDIF(NOT HAS_NVCC_FLAG)
    ENDIF(MSVC)
ELSE(CUDA_FOUND)
    SET(OPENMM_BUILD_CUDA_LIB OFF CACHE BOOL "Build OpenMMCuda library for Nvidia GPUs")
ENDIF(CUDA_FOUND)
IF(OPENMM_BUILD_CUDA_LIB)
    ADD_SUBDIRECTORY(platforms/cuda)
ENDIF(OPENMM_BUILD_CUDA_LIB)
MARK_AS_ADVANCED(CUDA_VERBOSE_BUILD)
MARK_AS_ADVANCED(CUDA_BUILD_CUBIN)
MARK_AS_ADVANCED(CUDA_BUILD_EMULATION)

FIND_PACKAGE(OpenCL QUIET)
# Plus the tesla/linux is taking too long on the tests
IF(OPENCL_FOUND)
    SET(OPENMM_BUILD_OPENCL_LIB ON CACHE BOOL "Build OpenMMOpenCL library")
ELSE(OPENCL_FOUND)
    SET(OPENMM_BUILD_OPENCL_LIB OFF CACHE BOOL "Build OpenMMOpenCL library")
ENDIF(OPENCL_FOUND)
IF(OPENMM_BUILD_OPENCL_LIB)
    ADD_SUBDIRECTORY(platforms/opencl)
ENDIF(OPENMM_BUILD_OPENCL_LIB)

# Optimized CPU platform

SET(OPENMM_BUILD_CPU_LIB ON CACHE BOOL "Build optimized CPU platform")
IF(OPENMM_BUILD_CPU_LIB)
   ADD_SUBDIRECTORY(platforms/cpu)
ENDIF(OPENMM_BUILD_CPU_LIB)

# Amoeba plugin

SET(OPENMM_BUILD_AMOEBA_PLUGIN ON CACHE BOOL "Build Amoeba plugin")
SET(OPENMM_BUILD_AMOEBA_PATH)
IF(OPENMM_BUILD_AMOEBA_PLUGIN)
   SET(OPENMM_BUILD_AMOEBA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/plugins/amoeba)
   ADD_SUBDIRECTORY(plugins/amoeba)
ENDIF(OPENMM_BUILD_AMOEBA_PLUGIN)

# RPMD plugin

SET(OPENMM_BUILD_RPMD_PLUGIN ON CACHE BOOL "Build RPMD plugin")
SET(OPENMM_BUILD_RPMD_PATH)
IF(OPENMM_BUILD_RPMD_PLUGIN)
   SET(OPENMM_BUILD_RPMD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/plugins/rpmd)
   ADD_SUBDIRECTORY(plugins/rpmd)
ENDIF(OPENMM_BUILD_RPMD_PLUGIN)

# Drude plugin

SET(OPENMM_BUILD_DRUDE_PLUGIN ON CACHE BOOL "Build Drude plugin")
SET(OPENMM_BUILD_DRUDE_PATH)
IF(OPENMM_BUILD_DRUDE_PLUGIN)
   SET(OPENMM_BUILD_DRUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/plugins/drude)
   ADD_SUBDIRECTORY(plugins/drude)
ENDIF(OPENMM_BUILD_DRUDE_PLUGIN)

# CPU PME plugin

FIND_PACKAGE(FFTW QUIET)
IF(FFTW_FOUND)
    SET(OPENMM_BUILD_PME_PLUGIN ON CACHE BOOL "Build CPU PME plugin")
ELSE(FFTW_FOUND)
    SET(OPENMM_BUILD_PME_PLUGIN OFF CACHE BOOL "Build CPU PME plugin")
ENDIF(FFTW_FOUND)
SET(OPENMM_BUILD_PME_PATH)
IF(OPENMM_BUILD_PME_PLUGIN)
   SET(OPENMM_BUILD_PME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/plugins/cpupme)
   ADD_SUBDIRECTORY(plugins/cpupme)
ENDIF(OPENMM_BUILD_PME_PLUGIN)

INSTALL_TARGETS(/lib RUNTIME_DIRECTORY /lib ${SHARED_TARGET})
IF(OPENMM_BUILD_STATIC_LIB)
  INSTALL_TARGETS(/lib RUNTIME_DIRECTORY /lib ${STATIC_TARGET})
ENDIF(OPENMM_BUILD_STATIC_LIB)
FILE(GLOB CORE_HEADERS     include/*.h          */include/*.h)
FILE(GLOB TOP_HEADERS      include/openmm/*.h          */include/openmm/*.h)
FILE(GLOB INTERNAL_HEADERS include/openmm/internal/*.h */include/openmm/internal/*.h )
FILE(GLOB REFERENCE_HEADERS platforms/reference/include/*.h)
INSTALL_FILES(/include                 FILES ${CORE_HEADERS})
INSTALL_FILES(/include/openmm          FILES ${TOP_HEADERS})
INSTALL_FILES(/include/openmm/internal FILES ${INTERNAL_HEADERS})
INSTALL_FILES(/include/openmm/reference FILES ${REFERENCE_HEADERS})

# Serialization support

ADD_SUBDIRECTORY(serialization)
FILE(GLOB serialization_files  ${CMAKE_SOURCE_DIR}/serialization/src/*.cpp)
SET_SOURCE_FILES_PROPERTIES(${serialization_files} PROPERTIES COMPILE_FLAGS "-DOPENMM_BUILDING_SHARED_LIBRARY -DTIXML_USE_STL -DIEEE_8087")

# Python wrappers

SET(OPENMM_BUILD_PYTHON_WRAPPERS ON CACHE BOOL "Build wrappers for Python")
IF(OPENMM_BUILD_PYTHON_WRAPPERS)
    IF(NOT OPENMM_BUILD_AMOEBA_PLUGIN)
        MESSAGE(SEND_ERROR "The Python wrappers require that the AMOEBA plugin be built.")
    ENDIF(NOT OPENMM_BUILD_AMOEBA_PLUGIN)
    IF(NOT OPENMM_BUILD_RPMD_PLUGIN)
        MESSAGE(SEND_ERROR "The Python wrappers require that the RPMD plugin be built.")
    ENDIF(NOT OPENMM_BUILD_RPMD_PLUGIN)
    IF(NOT OPENMM_BUILD_DRUDE_PLUGIN)
        MESSAGE(SEND_ERROR "The Python wrappers require that the Drude plugin be built.")
    ENDIF(NOT OPENMM_BUILD_DRUDE_PLUGIN)
    ADD_SUBDIRECTORY(wrappers/python)
ENDIF(OPENMM_BUILD_PYTHON_WRAPPERS)


#
# Allow automated build and dashboard.
#
# redundant INCLUDE(Dart) statement
# INCLUDE (Dart)

#IF (UNIX AND NOT CYGWIN AND NOT APPLE)
#  IF (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES Debug)
#    ADD_DEFINITIONS(-fprofile-arcs -ftest-coverage)
#    LINK_LIBRARIES(gcov)
#  ENDIF (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES Debug)
#ENDIF (UNIX AND NOT CYGWIN AND NOT APPLE)


#
# Testing
#

ENABLE_TESTING()

IF (EXECUTABLE_OUTPUT_PATH)
  SET (TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
  SET (TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

# Build and install documentation

find_package(Doxygen QUIET)
mark_as_advanced(CLEAR DOXYGEN_EXECUTABLE)
IF(DOXYGEN_EXECUTABLE)
    # Generate C++ API documentation

    SET(DOXY_CONFIG_C++ "${CMAKE_CURRENT_BINARY_DIR}/DoxyfileC++")
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/docs/DoxyfileC++.in 
          ${DOXY_CONFIG_C++}
          @ONLY )
    FILE(GLOB_RECURSE OPENMM_INCLUDES "openmm/include/*.h")
    FILE(GLOB_RECURSE OLLA_INCLUDES "olla/include/*.h")
    ADD_CUSTOM_COMMAND(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/api-c++/index.html"
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG_C++}
        DEPENDS ${OPENMM_INCLUDES} ${OLLA_INCLUDES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating C++ API documentation using Doxygen")
    ADD_CUSTOM_TARGET(C++ApiDocs 
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/api-c++/index.html"
        COMMENT "Generating C++ API documentation using Doxygen"
        SOURCES 
            "${CMAKE_CURRENT_SOURCE_DIR}/docs/DoxyfileC++.in"
            ${OPENMM_INCLUDES}
            ${OLLA_INCLUDES}
    ) 
    FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/api-c++/")
    INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/api-c++/"
            DESTINATION "docs/api-c++/")
    INSTALL(FILES "docs/C++ API Reference.html"
            DESTINATION "docs/")
    ADD_CUSTOM_TARGET(DoxygenApiDocs
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/api-c++/index.html"
        COMMENT "Generating C++ API documentation using Doxygen"
        SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/DoxyfileC++.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/DoxyfilePython.in"
        ${OPENMM_INCLUDES}
        ${OLLA_INCLUDES}
    )
    set(OPENMM_GENERATE_API_DOCS OFF CACHE BOOL "Whether to create API documentation using Doxygen")
    IF (OPENMM_GENERATE_API_DOCS)
        SET_TARGET_PROPERTIES(DoxygenApiDocs PROPERTIES EXCLUDE_FROM_ALL FALSE)
    ENDIF (OPENMM_GENERATE_API_DOCS)

    # Generate Python API documentation

    IF (OPENMM_BUILD_PYTHON_WRAPPERS)
        SET(DOXY_CONFIG_PYTHON "${CMAKE_CURRENT_BINARY_DIR}/DoxyfilePython")
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/docs/DoxyfilePython.in 
            ${DOXY_CONFIG_PYTHON}
            @ONLY )
        ADD_CUSTOM_COMMAND(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/api-python/index.html"
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG_PYTHON}
            DEPENDS RunSwig
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Generating Python API documentation using Doxygen")
        ADD_CUSTOM_TARGET(PythonApiDocs 
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/api-python/index.html"
            COMMENT "Generating Python API documentation using Doxygen"
            SOURCES 
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/DoxyfilePython.in"
                ${OPENMM_INCLUDES}
                ${OLLA_INCLUDES}
        ) 
        FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/api-python/")
        INSTALL(DIRECTORY "${PROJECT_BINARY_DIR}/api-python/"
                DESTINATION "docs/api-python/")
        INSTALL(FILES "docs/Python API Reference.html"
                DESTINATION "docs/")
        ADD_DEPENDENCIES(DoxygenApiDocs PythonApiDocs)
    ENDIF (OPENMM_BUILD_PYTHON_WRAPPERS)
ENDIF(DOXYGEN_EXECUTABLE)

install(FILES docs/OpenMMUsersGuide.pdf docs/OpenMMDeveloperGuide.pdf
    DESTINATION docs/)

FILE(GLOB LICENSE_FILES "docs/licenses/*.txt")
install(FILES ${LICENSE_FILES}
    DESTINATION licenses/)

ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples)

ENDIF(NOT cmv EQUAL "2.4") # This whole file...
