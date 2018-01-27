cmake_minimum_required (VERSION 3.5)
project (Lisp++)
set (Lisp++_VERSION_MAJOR 0)
set (Lisp++_VERSION_MINOR 1)
set (CMAKE_CXX_STANDARD 11)

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES Debug Release DebugMX31 ReleaseMX31)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
    "Reset the configurations to what we need"
    FORCE)
endif()

SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

include_directories (${PROJECT_SOURCE_DIR}/Catch2/single_include)
include_directories (${PROJECT_SOURCE_DIR})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
    add_library(Catch INTERFACE)
    target_include_directories(Catch INTERFACE test)

    add_executable(runtest
      runtests.cpp
      test_core/test_object.cpp
      test_core/test_cons_factory.cpp
      test_core/test_cons.cpp
      )
    target_link_libraries(runtest Catch Core Simul)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

add_executable(sim_cons_factory simul/lisp_sim_cons_factory.cpp)
target_link_libraries(sim_cons_factory Core Simul)

add_subdirectory (core)
add_subdirectory (simul)