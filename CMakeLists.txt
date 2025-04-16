cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(ros_video_streaming VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  cv_bridge
  sensor_msgs
  image_transport
  camera_info_manager
)

find_package(OpenCV 4 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(V4L2 REQUIRED libv4l2)

catkin_package(
  INCLUDE_DIRS
    include
    ${V4L2_INCLUDE_DIRS}
  LIBRARIES
    ${PROJECT_NAME}
  CATKIN_DEPENDS
    roscpp
    cv_bridge
    sensor_msgs
    image_transport
    camera_info_manager
  DEPENDS
    OpenCV
    libv4l2
)

add_library(catkin_deps INTERFACE)

target_link_libraries(catkin_deps INTERFACE
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
)

target_include_directories(catkin_deps INTERFACE
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_library(v4l2_deps INTERFACE)
target_link_libraries(v4l2_deps INTERFACE ${V4L2_LIBRARIES})
target_include_directories(v4l2_deps INTERFACE include ${V4L2_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC src/capture.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC v4l2_deps plog::plog)

add_executable(playground playground.cpp)
target_link_libraries(playground PRIVATE v4l2_deps plog::plog)
