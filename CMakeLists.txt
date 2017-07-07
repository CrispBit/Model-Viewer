cmake_minimum_required(VERSION 3.8)
project(ogl_practice)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp Mesh.h Mesh.cpp Texture.h Texture.cpp)

add_executable(ogl_practice ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(SFML 2 REQUIRED system window graphics network audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${SFML_LIBRARIES})
endif()

find_package(OpenGL)
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${OPENGL_LIBRARIES})
endif()

find_package(GLEW)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(ogl_practice ${GLEW_LIBRARIES})
endif()

find_package(ASSIMP)
if (ASSIMP_FOUND)
    include_directories(${ASSIMP_INCLUDE_DIRS})
    target_link_libraries(ogl_practice ${ASSIMP_LIBRARIES})
endif()
