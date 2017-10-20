cmake_minimum_required(VERSION 3.8)
project(ogl_practice)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_FILES main.cpp)

add_executable(ogl_practice ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(SFML 2 REQUIRED system window graphics network audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${SFML_LIBRARIES})
    target_link_libraries(ogl_practice ${SFML_DEPENDENCIES})
endif()

set(YAML_CPP_STATIC_LIBRARY TRUE)
find_package(YamlCpp REQUIRED)
if (YAMLCPP_FOUND)
    include_directories(${YAMLCPP_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${YAMLCPP_LIBRARY})
endif()

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${OPENGL_LIBRARIES})
endif()

find_package(GLM REQUIRED)
if (GLM_FOUND)
    include_directories(${GLM_INCLUDE_DIRS})
    target_link_libraries(ogl_practice ${GLM_LIBRARIES})
endif()

find_package(ASSIMP REQUIRED)
if (ASSIMP_FOUND)
    include_directories(${ASSIMP_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${ASSIMP_LIBRARIES})
endif()

find_package(Boost COMPONENTS system thread filesystem REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    ADD_DEFINITIONS(-DHAS_BOOST)
    target_link_libraries(ogl_practice ${Boost_LIBRARIES})
endif()

set(CRISPSYNTH_STATIC_LIBRARY ON)
find_package(CrispSynth REQUIRED)
if (CRISPSYNTH_FOUND)
    include_directories(${CRISPSYNTH_INCLUDE_DIR})
    target_link_libraries(ogl_practice ${CRISPSYNTH_LIBRARY} ${CMAKE_DL_LIBS})
endif()
