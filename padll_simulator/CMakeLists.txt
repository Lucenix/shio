cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0148)
    cmake_policy(SET CMP0148 NEW)
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(padll_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CSVPARSER_VERSION "v5.5.0" CACHE STRING "csv-parser version/tag or branch")
set(CSV_BUILD_TESTS OFF CACHE BOOL "Build csv-parser tests" FORCE)
set(CSV_BUILD_PROGRAMS OFF CACHE BOOL "Build csv-parser programs" FORCE)
set(CSV_BUILD_SINGLE_INCLUDE_TEST OFF CACHE BOOL "Build csv-parser single-header smoke test" FORCE)

include(FetchContent)

FetchContent_Declare(
    csv
    GIT_REPOSITORY https://github.com/vincentlaucsb/csv-parser.git
    GIT_SHALLOW TRUE
    GIT_TAG ${CSVPARSER_VERSION}
)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_SHALLOW TRUE
    GIT_TAG yaml-cpp-0.9.0
)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build yaml-cpp as static lib" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable yaml-cpp test targets" FORCE)

FetchContent_MakeAvailable(csv yaml-cpp)

add_executable(
    padll_simulator
    src/simulator.cpp
    src/control_application/control_application.cpp
    src/control_application/control_application_config_parser.cpp
    src/control_application/static_control_application.cpp
    src/control_application/dynamic_max_min_control_application.cpp
    src/control_application/psfa_control_application.cpp
    src/control_application/promotion_control_application.cpp
    src/data/bandwidths.cpp
    src/data/node_stats.cpp
    src/data/workload_replayer.cpp
    src/data/node_replayer.cpp
)

target_include_directories(padll_simulator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_include_directories(padll_simulator SYSTEM PRIVATE
    "${csv_SOURCE_DIR}/include"
    "${yaml-cpp_SOURCE_DIR}/include"
)

if (UNIX AND NOT APPLE)
    message(STATUS "Detecting UNIX (!Apple)")
    target_compile_options(padll_simulator PRIVATE
        -g
        -pthread
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wdisabled-optimization
        -Wformat=2
        -Winit-self
        -Wlogical-op
        -Wmissing-declarations
        -Wmissing-include-dirs
        -Wnoexcept
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wshadow
        -Wsign-conversion
        -Wsign-promo
        -Wstrict-null-sentinel
        -Wstrict-overflow=5
        -Wswitch-default
        -Wundef
        -Wno-unused
        -fPIC
        -fpermissive
    )
    target_link_libraries(padll_simulator PRIVATE pthread dl)
endif()

option(ENABLE_ASAN "Build with AddressSanitizer + UndefinedBehaviorSanitizer instrumentation" OFF)
if (ENABLE_ASAN)
    message(STATUS "AddressSanitizer/UBSan enabled")
    target_compile_options(padll_simulator PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
    target_link_options(padll_simulator PRIVATE -fsanitize=address,undefined)
endif ()

target_link_libraries(padll_simulator PRIVATE csv yaml-cpp)