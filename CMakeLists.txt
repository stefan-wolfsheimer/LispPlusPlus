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

include_directories (${PROJECT_SOURCE_DIR}/Catch2/single_include)
include_directories (${PROJECT_SOURCE_DIR}/CliArgs/include)
include_directories (${PROJECT_SOURCE_DIR}/CSVPlusPlus/)
include_directories (${PROJECT_SOURCE_DIR})

add_subdirectory (lpp/core)
add_subdirectory (lpp/simul)


IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
    add_library(Catch INTERFACE)
    target_include_directories(Catch INTERFACE test)

    add_executable(runtest
      runtests.cpp
      test_core/test_object.cpp
      test_core/test_integer.cpp
      test_core/test_string.cpp
      test_core/test_cons_pages.cpp
      test_core/test_collectible_container.cpp
      test_core/test_cons.cpp
      test_core/test_array.cpp
      test_core/test_garbage_collector.cpp
      test_core/test_symbol_container.cpp
      test_core/test_type_container.cpp
      test_core/test_function.cpp
      test_core/test_env.cpp
      test_core/test_util.cpp
      test_core/test_vm.cpp
      test_core/compiler/scope.cpp
      test_core/forms/value.cpp
      test_core/forms/define.cpp
      test_core/forms/lambda.cpp
      test_core/test_builtin_function.cpp
      test_core/test_compile_eval.cpp
      test_simul/test_gc_sim.cpp
      )
    target_link_libraries(runtest Catch Core Simul)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# add_library(Simul INTERFACE)
# add_library(Simul)
target_include_directories(Catch INTERFACE test)

add_executable(gc_sim gc_sim.cpp)
target_link_libraries(gc_sim Simul Core)

