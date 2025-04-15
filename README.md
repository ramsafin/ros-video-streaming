# ROS video streaming

Video streaming ROS package based on [Video4Linux API (ver. 2)](https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/v4l2.html) which is originally developed for the [Servosila Engineer](https://www.servosila.com/en/index.shtml) mobile robot.

It is written in C++17 and provides clear API.

## Prerequisites

- Ubuntu 16.04 (Xenial)
- ROS Kinetic
- OpenCV 3 (comes with ROS)
- GNU GCC (with C++17 support) 
- Video4Linux2 API

## Installation

### Video4Linux2
Install v4l-utils apt package:
```shell
sudo apt-get install v4l-utils
```
### ROS Kinetic
[Manually](http://wiki.ros.org/kinetic/Installation/Ubuntu) or using [ansible playbook](https://github.com/ramsafin/ansible-ros-kinetic-playbook).

## Getting Started

To download the latest available release, clone the repository into your _catkin workspace_ (e.g. `~/catkin_ws`):
```shell
cd ~/catkin_ws/src && git clone https://github.com/ramsafin/ros-video-streaming.git
```

Build ROS package:
```shell
cd ~/catkin_ws && catkin_make --pkg ros_video_streaming
```

Run tests (`Note!` Change parameters in [launch file](launch/camera.launch)):
```shell
rostest ros_video_streaming cameraHz.test
```

## Example (launch file)

The following ROS launch file will start ROS _master node_ along with _video_streamer node_.

```xml
<launch>
  
  <!-- start video streaming node -->
  <include file="$(find ros_video_streaming)/launch/camera.launch">
    
    <!-- ROS node name -->
    <arg name="camera_name" value="camera"/>
    
    <arg name="frame_id" value="$(arg camera_name)"/>

    <!-- video device -->
    <arg name="device_name" value="/dev/video0"/>
    
    <!-- frame resolution -->
    <arg name="width" value="640"/>
    <arg name="height" value="480"/>

    <!-- frame rate -->
    <arg name="fps" value="30"/>
    
    <!-- ROS image format (as defined in sensor_msgs/image_encodings.h -->
    <arg name="image_format" value="yuv422"/>

    <!-- camera info -->
    <arg name="camera_info_url" value="file:///$(find ros_video_streaming)/calibration/camera.yaml"/>

    <!-- whether to start image_view node (visualization) -->
    <arg name="image_view_enabled" value="false"/>
    
    <!-- image_view topic, e.g. 'rosrun image_view image_view image:=/camera/image_raw' -->
    <arg name="image_view_topic" value="image_raw"/>

    <!-- image_proc node -->
    <arg name="image_proc_enabled" default="false"/>
  
  </include>
  
</launch>
```

## Limitations and Issues
- **YUV422** image format in ROS Kinetic represents **UYVY** (other formats does not supported, e.g. **YUYV**).
In this case frames are converted into **grayscale** format, as it is computationally less demanded compared to the conversion into an **RGB**.

- No synchronization between multiple cameras (e.g. in case of stereo systems).

- Published frames timestamp is represented by ROS Time (not native driver's timestamp).

- Captured frames are not queued and potentially can be lost during streaming.

## Paper

[R. Safin, and R. Lavrenov "Implementation of ROS Package for Simultaneous Video Streaming from Several Different Cameras"](https://www.researchgate.net/publication/325903109_Implementation_of_ROS_package_for_simultaneous_video_streaming_from_several_different_cameras?origin=mail&uploadChannel=re390&reqAcc=Jenny_Midwinter&useStoredCopy=0)

_The 2018 International Conference on Artificial Life and Robotics (ICAROB 2018)At: B-Con Plaza, Beppu, Oita, Japan._
