#!/bin/bash
set -e

DOCKER_IMAGE="ros-noetic:latest"

echo "Building ROS Noetic Docker image..."

docker build -t $DOCKER_IMAGE -f Dockerfile .

echo "Build completed: $DOCKER_IMAGE"