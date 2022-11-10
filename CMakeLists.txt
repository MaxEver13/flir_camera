cmake_minimum_required(VERSION 2.8.3)

#工程名字
project(dual_cam_recorder)

#编译选项
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "-std=c++11 -g -o3 -Wall")

#依赖的第三方库
#官方不提供.cmake文件，需要自己提供
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(Spinnaker REQUIRED)
if(NOT Spinnaker_FOUND)
  message(STATUS "Spinnaker not found in system path")
endif()

message(STATUS "Spinnaker library dir: ${Spinnaker_LIBRARIES}")
message(STATUS "Spinnaker include dir: ${Spinnaker_INCLUDE_DIRS}")

find_package(OpenCV REQUIRED)
message(STATUS ${OPENCV_VERSION})

#头文件
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${Spinnaker_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIRS}
)

#编译可执行程序
add_executable(recorder recorder.cpp)
add_executable(trigger trigger.cpp)
add_executable(recorder_trigger recorder_trigger.cpp)

#库
target_link_libraries(recorder
    ${Spinnaker_LIBRARIES}
    ${OPENCV_LIBS}
    -pthread #多线程
)

target_link_libraries(trigger
    ${Spinnaker_LIBRARIES}
    ${OPENCV_LIBS}
    -pthread #多线程
)

target_link_libraries(recorder_trigger
    ${Spinnaker_LIBRARIES}
    ${OPENCV_LIBS}
    -pthread #多线程
)
