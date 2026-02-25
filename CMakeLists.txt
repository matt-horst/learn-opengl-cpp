cmake_minimum_required(VERSION 4.2)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863
)

FetchContent_MakeAvailable(glm)

find_package(glfw3 REQUIRED)

add_executable(LearnOpenGL)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE VENDOR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/vendor/*/*.c")

target_sources(LearnOpenGL PRIVATE ${PROJECT_SOURCES} ${VENDOR_SOURCES})

target_include_directories(LearnOpenGL PRIVATE include)

add_library(assimp SHARED IMPORTED)

set_target_properties(assimp PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libassimp.so"
)

target_link_libraries(LearnOpenGL PRIVATE glfw glm::glm assimp)
