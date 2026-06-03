cmake_minimum_required(VERSION 3.15)
project(IziLang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(IZI_ENABLE_READLINE "Enable GNU readline support in the REPL" ON)
option(IZI_ENABLE_RAYLIB "Enable raylib support" OFF)
option(IZI_BUILD_TESTS "Build IziLang tests" ON)

set(RAYLIB_DIR "" CACHE PATH "Path to raylib installation (include/ and lib/ subdirs)")

set(IZI_HAS_READLINE OFF)
set(IZI_HAS_RAYLIB OFF)

if(IZI_ENABLE_READLINE)
    find_path(READLINE_INCLUDE_DIR readline/readline.h)
    find_library(READLINE_LIBRARY readline)
    if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
        set(IZI_HAS_READLINE ON)
        message(STATUS "GNU readline enabled")
    else()
        message(WARNING "IZI_ENABLE_READLINE=ON but readline was not found. Building without readline support.")
    endif()
endif()

if(IZI_ENABLE_RAYLIB)
    if(RAYLIB_DIR)
        find_path(RAYLIB_INCLUDE_DIR raylib.h PATHS "${RAYLIB_DIR}/include" NO_DEFAULT_PATH)
        find_library(RAYLIB_LIBRARY raylib PATHS "${RAYLIB_DIR}/lib" NO_DEFAULT_PATH)
    else()
        find_path(RAYLIB_INCLUDE_DIR raylib.h)
        find_library(RAYLIB_LIBRARY raylib)
    endif()

    if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
        set(IZI_HAS_RAYLIB ON)
        message(STATUS "raylib enabled")
    else()
        message(FATAL_ERROR "IZI_ENABLE_RAYLIB=ON but raylib was not found. Set RAYLIB_DIR or disable IZI_ENABLE_RAYLIB.")
    endif()
endif()

file(GLOB_RECURSE IZI_ALL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

set(IZI_MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM IZI_ALL_SOURCES "${IZI_MAIN_SOURCE}")

add_library(izilang_core STATIC ${IZI_ALL_SOURCES})
target_compile_features(izilang_core PUBLIC cxx_std_20)
target_include_directories(izilang_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(IZI_HAS_READLINE)
    target_compile_definitions(izilang_core PUBLIC HAVE_READLINE)
    target_include_directories(izilang_core PUBLIC "${READLINE_INCLUDE_DIR}")
    target_link_libraries(izilang_core PUBLIC "${READLINE_LIBRARY}")
endif()

if(IZI_HAS_RAYLIB)
    target_compile_definitions(izilang_core PUBLIC HAVE_RAYLIB)
    target_include_directories(izilang_core PUBLIC "${RAYLIB_INCLUDE_DIR}")
    target_link_libraries(izilang_core PUBLIC "${RAYLIB_LIBRARY}")
    if(UNIX AND NOT APPLE)
        target_link_libraries(izilang_core PUBLIC GL X11)
    endif()
endif()

if(UNIX)
    target_link_libraries(izilang_core PUBLIC m dl pthread)
endif()

add_executable(izi "${IZI_MAIN_SOURCE}")
target_link_libraries(izi PRIVATE izilang_core)

if(IZI_BUILD_TESTS)
    file(GLOB_RECURSE IZI_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    add_executable(izilang_tests ${IZI_TEST_SOURCES})
    target_link_libraries(izilang_tests PRIVATE izilang_core)
    target_include_directories(izilang_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")

    enable_testing()
    add_test(NAME cpp_tests COMMAND $<TARGET_FILE:izilang_tests>)
    add_test(
        NAME izilang_script_tests
        COMMAND $<TARGET_FILE:izi> test
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
