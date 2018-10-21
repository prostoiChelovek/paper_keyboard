cmake_minimum_required(VERSION 3.8)
project(PaperKeyboard)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
find_package(HandDetector REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(libserial REQUIRED libserial)

set(SOURCE_FILES main.cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -lpthread")

add_executable(PaperKeyboard main.cpp PaperKeyboard.cpp PaperKeyboard.h PKBKey.cpp PKBKey.h)

target_link_libraries(PaperKeyboard ${OpenCV_LIBS})
target_link_libraries(PaperKeyboard ${libserial_LIBRARIES})

if (HandDetector_FOUND)
    include_directories(${HandDetector_INCLUDE_DIR})
    target_link_libraries(PaperKeyboard ${HANDDETECTOR_LIBRARY})
else (HandDetector_FOUND)
    message(FATAL_ERROR "HandDetector library not found")
endif (HandDetector_FOUND)