cmake_minimum_required(VERSION 3.8)
project(PaperKeyboard)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
find_package(HandDetector REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(libserial REQUIRED libserial)
pkg_check_modules(zbar REQUIRED zbar)
pkg_check_modules(libqrencode REQUIRED libqrencode)

set(SOURCE_FILES main.cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-sign-compare")

add_executable(PaperKeyboard main.cpp PaperKeyboard.cpp PaperKeyboard.h Key.cpp Key.h QR.cpp QR.h utils.cpp utils.h cvui.h)

target_link_libraries(PaperKeyboard ${OpenCV_LIBS})
target_link_libraries(PaperKeyboard ${libserial_LIBRARIES})
target_link_libraries(PaperKeyboard ${zbar_LIBRARIES})
target_link_libraries(PaperKeyboard ${libqrencode_LIBRARIES})
target_link_libraries(PaperKeyboard pthread)
target_link_libraries(PaperKeyboard ${HandDetector_LIBRARY})
include_directories(${HandDetector_INCLUDE_DIR})