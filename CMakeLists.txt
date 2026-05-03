cmake_minimum_required(VERSION 3.16)

project(Tucil3_13524059 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(tucil3_core
    src/Board.cpp
    src/graph.cpp
    src/pathFinding.cpp
)

target_include_directories(tucil3_core PUBLIC include)

add_executable(tucil3_gui src/mainGUI.cpp)
target_link_libraries(tucil3_gui PRIVATE tucil3_core)
set_target_properties(tucil3_gui PROPERTIES OUTPUT_NAME "tucil3-gui")

add_custom_target(run
    COMMAND $<TARGET_FILE:tucil3_gui>
    DEPENDS tucil3_gui
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

find_package(raylib QUIET)

if (raylib_FOUND)
    target_link_libraries(tucil3_gui PRIVATE raylib)
else()
    include(FetchContent)

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )

    FetchContent_MakeAvailable(raylib)
    target_link_libraries(tucil3_gui PRIVATE raylib)
endif()
