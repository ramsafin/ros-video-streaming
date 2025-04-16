#!/bin/bash
set -e

# Unset the noninteractive frontend to enable interactive package installation
unset DEBIAN_FRONTEND

# Source the ROS setup file if it exists
if [ -f "/opt/ros/noetic/setup.bash" ]; then
    source /opt/ros/noetic/setup.bash
fi

# If no arguments are provided, start an interactive bash shell
if [ $# -eq 0 ]; then
    exec bash
else
    # or execute the passed command
    exec "$@"
fi