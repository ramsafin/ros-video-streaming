cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(ros_video_streaming VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  cv_bridge
  sensor_msgs
  image_transport
  camera_info_manager
)

find_package(OpenCV 4 REQUIRED)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS
    roscpp
    cv_bridge
    sensor_msgs
    image_transport
    camera_info_manager
  DEPENDS
    v4l-utils
)

add_library(catkin_deps INTERFACE)
target_link_libraries(catkin_deps INTERFACE ${catkin_LIBRARIES})
target_include_directories(catkin_deps INTERFACE include ${catkin_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC src/vide_capture.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC catkin_deps)
