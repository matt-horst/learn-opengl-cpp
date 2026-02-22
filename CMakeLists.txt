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

target_sources(LearnOpenGL PRIVATE src/main.cpp vendor/glad/glad.c)

target_include_directories(LearnOpenGL PRIVATE include)
target_link_libraries(LearnOpenGL PRIVATE glfw)
target_link_libraries(LearnOpenGL PRIVATE glm::glm)
