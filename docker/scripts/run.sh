#!/bin/bash
set -e

DOCKER_IMAGE="ros-noetic:latest"
DOCKER_CONTAINER="ros-noetic-dev"
ROS_WORKSPACE="$(pwd)/../../../"

echo "Running Docker container for Linux..."
echo "ROS workspace: ${ROS_WORKSPACE}"

docker run -it \
    --net=host \
    --privileged \
    --device=/dev/video0:/dev/video0 \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v ${ROS_WORKSPACE}:/home/ros/workspace \
    --name "${DOCKER_CONTAINER}" \
    $DOCKER_IMAGE