cmake_minimum_required(VERSION 3.10)

# set the project name
project(GEOS_PERFORMANCE VERSION 1.0)

################################################################################

add_library(zlib INTERFACE)
set_target_properties(zlib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "/usr/include"
  INTERFACE_LINK_DIRECTORIES "/usr/lib"
  INTERFACE_LINK_LIBRARIES "z"
)

################################################################################

# read in GEOS paths from geos-config
find_program(GEOS_CONFIG geos-config)
if (NOT GEOS_CONFIG)
  message(FATAL_ERROR, "Unable to find geos-config on PATH")
endif()

exec_program(${GEOS_CONFIG}
  ARGS --clibs
  OUTPUT_VARIABLE GEOS_LIBS)

exec_program(${GEOS_CONFIG}
  ARGS --cflags
  OUTPUT_VARIABLE GEOS_INCLUDES)

# message("-- GEOS_LIBS ${GEOS_LIBS}")
# message("-- GEOS_INCLUDES ${GEOS_INCLUDES}")

string(REGEX MATCH "[-][L]([^ ;]+)"
            _nothing
            "${GEOS_LIBS}")
set(GEOS_LIB_DIR ${CMAKE_MATCH_1})

string(REGEX MATCH "[-][l]([^ ;]+)"
            _nothing
            "${GEOS_LIBS}")
set(GEOS_LIB_NAME ${CMAKE_MATCH_1})

string(REGEX MATCH "[-][I]([^ ;]+)"
            _nothing
            "${GEOS_INCLUDES}")
set(GEOS_INC_DIR ${CMAKE_MATCH_1})

message("-- GEOS_LIB_DIR = ${GEOS_LIB_DIR}")
message("-- GEOS_LIB_NAME = ${GEOS_LIB_NAME}")
message("-- GEOS_INCLUDE_DIR = ${GEOS_INC_DIR}")

# set up geos link interface
add_library(libgeos_c INTERFACE) # or STATIC instead of SHARED
set_target_properties(libgeos_c PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${GEOS_INC_DIR}
  INTERFACE_LINK_DIRECTORIES ${GEOS_LIB_DIR}
  INTERFACE_LINK_LIBRARIES ${GEOS_LIB_NAME}
)

################################################################################

add_library(geos_perf_cxx_flags INTERFACE)
if (APPLE)
  target_compile_options(geos_perf_cxx_flags INTERFACE "-DAPPLE")
endif()

################################################################################

# add the executable and link geos/zlib
file(GLOB_RECURSE _sources ${CMAKE_SOURCE_DIR}/geos_perf*.c CONFIGURE_DEPEND)
add_executable(geos_perf ${_sources})
unset(_sources)
target_link_libraries(geos_perf libgeos_c)
target_link_libraries(geos_perf zlib)
target_link_libraries(geos_perf geos_perf_cxx_flags)
target_include_directories(geos_perf
  PRIVATE $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  )

################################################################################

# set up the build-specific include information
# in particular the DATA_DAR
file(RELATIVE_PATH DATA_DIR ${CMAKE_BINARY_DIR} "${CMAKE_SOURCE_DIR}/data")
message("-- DATA_DIR = ${DATA_DIR}")
configure_file(${CMAKE_SOURCE_DIR}/geos_perf_config.h.in
  ${CMAKE_BINARY_DIR}/geos_perf_config.h
  )





