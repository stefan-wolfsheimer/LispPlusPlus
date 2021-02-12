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

SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
# @todo: determine -DHAS_FMEMOPEN automatically
# @todo: create config.h file
SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g -DDEBUG -DWITH_COLOR -DHAS_FMEMOPEN -Werror -Wall")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG -DWITH_COLOR")
SET(CMAKE_C_FLAGS_COVERAGE "-O0 -g -DDEBUG -DWITH_COLOR -Werror -Wall -fprofile-arcs -ftest-coverage")

include_directories (${PROJECT_SOURCE_DIR}/Catch2/single_include)
include_directories (${PROJECT_SOURCE_DIR}/CliArgs/include)
include_directories (${PROJECT_SOURCE_DIR}/CSVPlusPlus/)
include_directories (${PROJECT_SOURCE_DIR})

IF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Release)
add_subdirectory (lpp/core)
add_subdirectory (lpp/scheme)
add_subdirectory (lpp/simul)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Release)

IF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES Coverage)
add_subdirectory (lisp/util)
add_subdirectory (lisp/core)
add_subdirectory (lisp/simul)
add_subdirectory (unit_tests/util)
add_subdirectory (unit_tests/simul)
add_subdirectory (unit_tests/core)

ENDIF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES Coverage)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("debug mode")
  add_library(Catch INTERFACE)
  target_include_directories(Catch INTERFACE test)

  add_executable(
    runtest
    runtests.cpp
    #      test_core/numeric.cpp
    test_core/test_object.cpp
    test_core/test_integer.cpp
    test_core/test_cons_pages.cpp
    test_core/test_collectible_container.cpp
    test_core/test_cons.cpp
    test_core/types/traits.cpp
    test_core/types/string.cpp
    test_core/types/array.cpp
    test_core/types/polymorphic_object.cpp
    test_core/types/polymorphic_container.cpp
    test_core/types/form.cpp
    test_core/types/function.cpp
    test_core/types/continuation.cpp
    test_core/memory/allocator.cpp
    test_core/test_env.cpp
    test_core/test_util.cpp
    test_core/test_vm.cpp
    test_core/test_builtin_function.cpp
    test_scheme/language.cpp
    test_simul/test_gc_sim.cpp
    )
  target_link_libraries(runtest Catch Scheme Core Simul)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

IF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Coverage)
  add_executable(
    run_unit_tests
    unit_tests/unit_tests.c
    )
  target_link_libraries(run_unit_tests
    TestUtil
    TestCore
    TestSimul
    LispCore
    LispSimul
    Util
    )
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Coverage)

IF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Release)
# add_library(Simul INTERFACE)
# add_library(Simul)
target_include_directories(Catch INTERFACE test)

add_executable(gc_sim gc_sim.cpp)
target_link_libraries(gc_sim Simul Scheme Core)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES Release)
