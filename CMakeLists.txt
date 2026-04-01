cmake_minimum_required(VERSION 3.15)
project(IziLang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# Options (mirror Premake5 custom options)
# ------------------------------------------------------------
option(HAVE_RAYLIB "Enable raylib support (set RAYLIB_DIR to the raylib install prefix)" OFF)
option(HAVE_READLINE "Enable GNU readline support in the REPL" ON)

# When the user passes -DHAVE_RAYLIB=ON they can also set RAYLIB_DIR
set(RAYLIB_DIR "" CACHE PATH "Path to raylib installation (include/ and lib/ subdirs)")

# ------------------------------------------------------------
# Collect all source files under src/ (mirrors premake "src/**.cpp")
# ------------------------------------------------------------
file(GLOB_RECURSE IZI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Separate out main.cpp so the tests target can reuse the rest
set(IZI_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM IZI_SOURCES "${IZI_MAIN}")
# IZI_SOURCES now contains everything except main.cpp
set(IZI_LIB_SOURCES ${IZI_SOURCES})

# ------------------------------------------------------------
# Helper: apply common settings to a target
# ------------------------------------------------------------
function(izi_configure_target TARGET)
    target_include_directories(${TARGET} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

    target_compile_definitions(${TARGET} PRIVATE
        $<$<BOOL:${HAVE_RAYLIB}>:HAVE_RAYLIB>
        $<$<BOOL:${HAVE_READLINE}>:HAVE_READLINE>
    )

    if(HAVE_RAYLIB AND RAYLIB_DIR)
        target_include_directories(${TARGET} PRIVATE "${RAYLIB_DIR}/include")
        target_link_directories(${TARGET} PRIVATE "${RAYLIB_DIR}/lib")
        target_link_libraries(${TARGET} PRIVATE raylib)
        if(UNIX AND NOT APPLE)
            target_link_libraries(${TARGET} PRIVATE GL X11)
        endif()
    endif()

    if(UNIX)
        target_link_libraries(${TARGET} PRIVATE m dl pthread)
        if(HAVE_READLINE)
            target_link_libraries(${TARGET} PRIVATE readline)
        endif()
    endif()
endfunction()

# ------------------------------------------------------------
# Main executable: izi
# ------------------------------------------------------------
add_executable(izi
    ${IZI_MAIN}
    ${IZI_LIB_SOURCES}
)
izi_configure_target(izi)

# ------------------------------------------------------------
# Tests executable (uses Catch2 bundled in tests/catch.hpp)
# ------------------------------------------------------------
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

add_executable(tests
    ${TEST_SOURCES}
    ${IZI_LIB_SOURCES}
)
izi_configure_target(tests)
# Also expose the tests/ directory so Catch2's catch.hpp can be found
target_include_directories(tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)
